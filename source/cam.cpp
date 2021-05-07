/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include <cam.h>

#include <new>
#include "common.h"

using namespace akaFrame::cam;
using namespace akaFrame::cam::mem;
using namespace akaFrame::cam::array;
using namespace akaFrame::cam::task;
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
#ifdef CAM_Z390
        , _z390(nullptr)
#endif
{
        memset(_borrows, 0, sizeof(_borrows));

        void *ptp = general_allocator().allocate(sizeof(ProgramTable));
        _program_table = new (ptp) ProgramTable(this);
        cam_add_provider(this, provider(*_program_table));

        add_loaders(this);
}

cam_s::~cam_s()
{
        for (auto id : _borrows) {
                if (id._u != 0) {
                        drop(_id_table, id);
                }
        }

        _program_table->~ProgramTable();
        general_allocator().deallocate(_program_table);

#ifdef CAM_Z390
        _z390->~Z390Loader();
        general_allocator().deallocate(_z390);
#endif
}

struct cam_s* cam_new(int *out_ec)
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

int cam_load_chunk(struct cam_s *cam, const void *buff, int buff_size)
{
        for (int i = 0; i < size(cam->_loaders); ++i) {
                int ec = cam->_loaders[i]->load_chunk(buff, buff_size);
                if (ec == CEC_NOT_SUPPORTED) {
                        continue;  // next loader
                } else {
                        return ec; // handled
                }
        }

        return CEC_NOT_SUPPORTED; // not recognized
}

int cam_address_make(struct cam_s *cam, void *buff, bool borrow, cam_address_t *out_address)
{
        if (borrow) {
                int num_borrows = sizeof(cam->_borrows) / sizeof(cam->_borrows[0]);

                for (auto &id : cam->_borrows) {
                        if (id._u == 0) {
                                id = make(cam->_id_table, buff);
                                out_address->_u = 0;
                                out_address->_v._space  = CAS_BORROWED;
                                out_address->_b._index  = &id - cam->_borrows;
                                out_address->_b._offset = 0;
                                return CEC_SUCCESS;
                        }
                }

                return CEC_NO_MEMORY;
        } else {
                *out_address = make(cam->_id_table, buff);
                return CEC_SUCCESS;
        }
}

void cam_address_drop(struct cam_s *cam, cam_address_t address)
{
        if (address._v._space == CAS_BORROWED) {
                drop(cam->_id_table, cam->_borrows[address._b._index]);
        } else {
                CAM_ASSERT(address._v._space == CAS_ID);
                drop(cam->_id_table, address);
        }
}

void* cam_address_buffer(struct cam_s *cam, cam_address_t address, cam_tid_t tid)
{
        switch (address._v._space) {
        case CAS_GLOBAL:
                return &cam->_global_buffer[address._v._offset];
        case CAS_LOCAL_STACK: {
                auto t = resolve<Task>(cam->_id_table, u32_address(tid._u));
                return at(*t, address._v._offset); }
        case CAS_BORROWED:
                return resolve<void>(cam->_id_table, cam->_borrows[address._b._index]);
        case CAS_ID:
                return resolve<void>(cam->_id_table, address);
        default:
                CAM_ASSERT(!"bad space");
                return nullptr;
        }
}

cam_pid_t cam_resolve(struct cam_s *cam, const char *name)
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

        return { 0 }; // not found
}

void cam_nop_k(struct cam_s *, cam_tid_t, void *)
{
        // nop
}

void cam_call(
        struct cam_s *cam, cam_tid_t tid, cam_pid_t pid,
        cam_address_t *params, int arity, cam_k_t k, void *ktx)
{
        auto t = resolve<Task>(cam->_id_table, u32_address(tid._u));
        call(*t, pid, params, arity, k, ktx);
}

void cam_go_back(struct cam_s *cam, cam_tid_t tid)
{
        auto t = resolve<Task>(cam->_id_table, u32_address(tid._u));
        go_back(*t);
}

cam_tid_t cam_task_new(
        struct cam_s *cam, int *out_ec, cam_pid_t entry,
        cam_address_t *params, int arity, cam_k_t k, void *ktx)
{
        int task_size = sizeof(Task) + sizeof(void*)*CAM_MAX_PROVIDERS;
        auto ntp = general_allocator().allocate(task_size);
        cam_tid_t tid = { make(cam->_id_table, ntp)._u };
        new (ntp) Task(cam, tid, entry, params, arity, k, ktx);
        return tid;
}

void cam_task_delete(struct cam_s *cam, cam_tid_t tid)
{
        auto t = resolve<Task>(cam->_id_table, u32_address(tid._u));
        t->~Task(); general_allocator().deallocate(t);
}

cam_pid_t cam_top_program(struct cam_s *cam, cam_tid_t tid)
{
        auto t = resolve<Task>(cam->_id_table, u32_address(tid._u));
        return top_program(*t);
}

void cam_yield(struct cam_s *cam, cam_tid_t tid, cam_k_t k, void *ktx)
{
        auto id = u32_address(tid._u);
        auto t = resolve<Task>(cam->_id_table, id);
        yield(*t, k, ktx);
}

void cam_resume(struct cam_s *cam, cam_tid_t tid)
{
        auto id = u32_address(tid._u);
        auto t = resolve<Task>(cam->_id_table, id);
        resume(*t);
}

void* cam_get_tlpvs(struct cam_s *cam, cam_tid_t tid, int index)
{
        auto t = resolve<Task>(cam->_id_table, u32_address(tid._u));
        return t->_tlpvs[index];
}

void cam_set_tlpvs(struct cam_s *cam, cam_tid_t tid, int index, void *state)
{
        auto t = resolve<Task>(cam->_id_table, u32_address(tid._u));
        t->_tlpvs[index] = state;
}

u8* cam_push(struct cam_s *cam, cam_tid_t tid, int bytes, cam_address_t *out_address)
{
        auto t = resolve<Task>(cam->_id_table, u32_address(tid._u));
        return push(*t, bytes, out_address);
}

u8* cam_pop(struct cam_s *cam, cam_tid_t tid, int bytes)
{
        auto t = resolve<Task>(cam->_id_table, u32_address(tid._u));
        return pop(*t, bytes);
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

bool cam_is_alive_program(struct cam_s *cam, cam_pid_t pid)
{
        return resolve<void>(cam->_id_table, u32_address(pid._u)) != nullptr;
}

bool cam_is_alive_task(struct cam_s *cam, cam_tid_t tid)
{
        return resolve<void>(cam->_id_table, u32_address(tid._u)) != nullptr;
}
