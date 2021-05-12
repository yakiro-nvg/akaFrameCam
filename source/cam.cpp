/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include <cam.h>

#include <new>
#include "common.h"

using namespace akaFrame::cam;
using namespace akaFrame::cam::mem;
using namespace akaFrame::cam::array;
using namespace akaFrame::cam::fiber;
using namespace akaFrame::cam::id_table;
using namespace akaFrame::cam::program_table;

#ifdef CAM_Z390
using namespace akaFrame::cam::z390;
#endif

static void add_loaders(cam_s *cam)
{
#ifdef CAM_Z390
        void *z390p = general_allocator().allocate(sizeof(Z390Loader));
        cam->_z390 = new (z390p) Z390Loader(cam);
        push(cam->_loaders, (Loader*)z390p);
        cam_add_provider(cam, cam->_z390->provider());
#endif
}

cam_s::cam_s()
        :  _loaders(general_allocator())
        , _providers_n(0)
        , _global_buffer_n(0)
        , _program_table(nullptr)
        , _on_unresolved(nullptr)
#ifdef CAM_Z390
        , _z390(nullptr)
#endif
{
        void *ptp = general_allocator().allocate(sizeof(ProgramTable));
        _program_table = new (ptp) ProgramTable(this);
        cam_add_provider(this, provider(*_program_table));

        add_loaders(this);
}

cam_s::~cam_s()
{
        _program_table->~ProgramTable();
        general_allocator().deallocate(_program_table);

#ifdef CAM_Z390
        _z390->~Z390Loader();
        general_allocator().deallocate(_z390);
#endif
}

struct cam_s* cam_new(cam_error_t *out_ec)
{
        *out_ec = CEC_SUCCESS;
        void *p = general_allocator().allocate(sizeof(cam_s));
        auto cam = new (p) cam_s();
        return cam;
}

void cam_delete(struct cam_s *cam)
{
        cam->~cam_s();
        general_allocator().deallocate(cam);
}

void cam_add_provider(struct cam_s *cam, cam_provider_t *provider)
{
        provider->index = cam->_providers_n++;
        CAM_ASSERT(cam->_providers_n <= CAM_MAX_PROVIDERS);
        cam->_providers[provider->index] = provider;
}

void cam_add_program(struct cam_s *cam, const char *name, cam_program_t *program)
{
        set(*cam->_program_table, name, program);
}

cam_error_t cam_load_chunk(struct cam_s *cam, const void *buff, int buff_size)
{
        for (int i = 0; i < size(cam->_loaders); ++i) {
                auto ec = cam->_loaders[i]->load_chunk(buff, buff_size);
                if (ec == CEC_NOT_SUPPORTED) {
                        continue;  // next loader
                } else {
                        return ec; // handled
                }
        }

        return CEC_NOT_SUPPORTED; // not recognized
}

cam_error_t cam_address_make(struct cam_s *cam, void *buff, cam_address_t *out_address)
{
        *out_address = make(cam->_id_table, buff);
        return CEC_SUCCESS;
}

void cam_address_drop(struct cam_s *cam, cam_address_t address)
{
        CAM_ASSERT(address._v._space == CAS_ID);
        drop(cam->_id_table, address);
}

void* cam_address_buffer(struct cam_s *cam, cam_address_t address, cam_address_t fid)
{
        switch (address._v._space) {
        case CAS_GLOBAL:
                return &cam->_global_buffer[address._v._offset];
        case CAS_LOCAL_STACK: {
                CAM_ASSERT(fid._u && "bad fiber");
                auto f = resolve<Fiber>(cam->_id_table, fid);
                return at(*f, address._v._offset); }
        case CAS_ID:
                return resolve<void>(cam->_id_table, address);
        default:
                CAM_ASSERT(!"bad space");
                return nullptr;
        }
}

cam_address_t cam_resolve(struct cam_s *cam, const char *name)
{
        for (int i = 0; i < cam->_providers_n; ++i) {
                auto p = cam->_providers[i];
                auto pid = p->resolve(p, name);
                if (pid._u == 0) {
                        continue;   // next provider
                } else {
                        return pid; // found
                }
        }

        // try with unresolved handler
        if (cam->_on_unresolved) {
                // to prevent recursion
                auto ou = cam->_on_unresolved;
                cam->_on_unresolved = nullptr;
                auto pid = ou(cam, name);
                cam->_on_unresolved = ou;
                return pid;
        } else {
                return { 0 }; // not found
        }
}

void cam_on_unresolved(struct cam_s *cam, cam_on_unresolved_t callback)
{
        cam->_on_unresolved = callback;
}

void cam_nop_k(struct cam_s *, cam_address_t, void *)
{
        // nop
}

void cam_call(
        struct cam_s *cam, cam_address_t fid, cam_address_t pid,
        cam_address_t *args, int arity, cam_k_t k, void *ktx)
{
        auto f = resolve<Fiber>(cam->_id_table, fid);
        call(*f, pid, args, arity, k, ktx);
}

void cam_go_back(struct cam_s *cam, cam_address_t fid)
{
        auto f = resolve<Fiber>(cam->_id_table, fid);
        go_back(*f);
}

cam_address_t cam_fiber_new(
        struct cam_s *cam, cam_error_t *out_ec, void *userdata,
        cam_address_t entry_pid, cam_address_t *args, int arity, cam_k_t k, void *ktx)
{
        *out_ec = CEC_SUCCESS;
        int fiber_size = sizeof(Fiber) + sizeof(void*)*CAM_MAX_PROVIDERS;
        auto ntp = general_allocator().allocate(fiber_size);
        auto fid = make(cam->_id_table, ntp);
        new (ntp) Fiber(cam, fid, userdata, entry_pid, args, arity, k, ktx);
        return fid;
}

void cam_fiber_delete(struct cam_s *cam, cam_address_t fid)
{
        auto f = resolve<Fiber>(cam->_id_table, fid);
        f->~Fiber(); general_allocator().deallocate(f);
}

void* cam_fiber_userdata(struct cam_s *cam, cam_address_t fid)
{
        auto f = resolve<Fiber>(cam->_id_table, fid);
        return userdata(*f);
}

cam_address_t cam_top_program(struct cam_s *cam, cam_address_t fid)
{
        auto f = resolve<Fiber>(cam->_id_table, fid);
        return top_program(*f);
}

void cam_yield(struct cam_s *cam, cam_address_t fid, cam_k_t k, void *ktx)
{
        auto f = resolve<Fiber>(cam->_id_table, fid);
        yield(*f, k, ktx);
}

void cam_resume(struct cam_s *cam, cam_address_t fid)
{
        auto f = resolve<Fiber>(cam->_id_table, fid);
        resume(*f);
}

void* cam_get_tlpvs(struct cam_s *cam, cam_address_t fid, int index)
{
        auto f = resolve<Fiber>(cam->_id_table, fid);
        return f->_tlpvs[index];
}

void cam_set_tlpvs(struct cam_s *cam, cam_address_t fid, int index, void *state)
{
        auto f = resolve<Fiber>(cam->_id_table, fid);
        f->_tlpvs[index] = state;
}

u8* cam_push(struct cam_s *cam, cam_address_t fid, int bytes, cam_address_t *out_address)
{
        auto f = resolve<Fiber>(cam->_id_table, fid);
        return push(*f, bytes, out_address);
}

u8* cam_pop(struct cam_s *cam, cam_address_t fid, int bytes)
{
        auto f = resolve<Fiber>(cam->_id_table, fid);
        return pop(*f, bytes);
}

u8* cam_global_grow(struct cam_s *cam, int bytes, cam_address_t *out_address)
{
        int old_size = cam->_global_buffer_n;
        cam->_global_buffer_n += bytes;
        CAM_ASSERT(cam->_global_buffer_n <= CAM_MAX_GLOBAL);
        out_address->_u = 0;
        out_address->_v._space  = CAS_GLOBAL;
        out_address->_v._offset = old_size;
        return cam->_global_buffer + old_size;
}