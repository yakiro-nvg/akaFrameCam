/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include <cam.h>

#include <new>
#include "common.h"

using namespace akaFrame::cam;
using namespace akaFrame::cam::mem;
using namespace akaFrame::cam::array;
using namespace akaFrame::cam::thread;
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

int cam_address_borrow(struct cam_s *cam, void *buff, cam_address_t *out_address)
{
        int num_borrows = sizeof(cam->_borrows) / sizeof(cam->_borrows[0]);

        for (auto &id : cam->_borrows) {
                if (id._u == 0) {
                        id = make(cam->_id_table, buff);
                        out_address->_v._space  = CAS_BORROWED;
                        out_address->_b._index  = &id - cam->_borrows;
                        out_address->_b._offset = 0;
                        return CEC_SUCCESS;
                }
        }

        return CEC_NO_MEMORY;
}

void cam_address_relocate(struct cam_s *cam, cam_address_t address, void *buff)
{
        CAM_ASSERT(address._v._space  == CAS_BORROWED);
        CAM_ASSERT(address._b._offset == 0);
        id_t id = cam->_borrows[address._b._index];
        relocate(cam->_id_table, cam->_borrows[address._b._index], buff);
}

void cam_address_drop(struct cam_s *cam, cam_address_t address)
{
        CAM_ASSERT(address._v._space  == CAS_BORROWED);
        CAM_ASSERT(address._b._offset == 0);
        drop(cam->_id_table, cam->_borrows[address._b._index]);
}

void* cam_address_buffer(struct cam_s *cam, cam_address_t address, cam_tid_t tid)
{
        switch (address._v._space) {
        case CAS_GLOBAL:
                return &cam->_global_buffer[address._v._offset];
        case CAS_THREAD_LOCAL: {
                auto t = resolve<Thread>(cam->_id_table, u32_id(tid._u));
                return t->_local_storage + address._v._offset; }
        case CAS_BORROWED:
                return resolve<void>(cam->_id_table, cam->_borrows[address._b._index]);
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

void cam_nop_k(struct cam_s *cam, cam_tid_t tid)
{
        // nop
}

void cam_call(
        struct cam_s *cam, cam_tid_t tid, cam_pid_t pid,
        cam_address_t *params, int arity, cam_k_t k)
{
        id_t id = u32_id(tid._u);
        auto t = resolve<Thread>(cam->_id_table, id);
        call(*t, pid, params, arity, k);
}

cam_tid_t cam_thread_new(
        struct cam_s *cam, int *out_ec, int stack_size,
        cam_pid_t entry, cam_address_t *params, int arity)
{
        int thread_sz = sizeof(Thread) + sizeof(void*)*CAM_MAX_PROVIDERS + stack_size;
        auto ntp = general_allocator().allocate(thread_sz);
        cam_tid_t tid = { id_u32(make(cam->_id_table, ntp)) };
        new (ntp) Thread(cam, tid, stack_size, entry, params, arity);
        return tid;
}

void cam_thread_delete(struct cam_s *cam, cam_tid_t tid)
{
        auto t = resolve<Thread>(cam->_id_table, u32_id(tid._u));
        t->~Thread(); general_allocator().deallocate(t);
}

void cam_thread_yield(struct cam_s *cam, cam_tid_t tid, cam_k_t k)
{
        id_t id = u32_id(tid._u);
        auto t = resolve<Thread>(cam->_id_table, id);
        yield(*t, k);
}

void cam_thread_resume(struct cam_s *cam, cam_tid_t tid)
{
        id_t id = u32_id(tid._u);
        auto t = resolve<Thread>(cam->_id_table, id);
        resume(*t);
}

void** cam_thread_tlpvs(struct cam_s *cam, cam_tid_t tid, int index)
{
        CAM_ASSERT(index < CAM_MAX_PROVIDERS);
        auto t = resolve<Thread>(cam->_id_table, u32_id(tid._u));
        return (void**)(t + 1) + index;
}

u8* cam_thread_push(struct cam_s *cam, cam_tid_t tid, int bytes, cam_address_t *out_address)
{
        auto t = resolve<Thread>(cam->_id_table, u32_id(tid._u));
        int old_size = t->_local_storage_n;
        t->_local_storage_n += bytes;
        CAM_ASSERT(t->_local_storage_n <= t->_stack_size);
        out_address->_v._space  = CAS_THREAD_LOCAL;
        out_address->_v._offset = old_size;
        return t->_local_storage + old_size;
}

u8* cam_thread_pop(struct cam_s *cam, cam_tid_t tid, int bytes)
{
        auto t = resolve<Thread>(cam->_id_table, u32_id(tid._u));
        t->_local_storage_n -= bytes;
        CAM_ASSERT(t->_local_storage_n >= 0);
        return t->_local_storage + t->_local_storage_n;
}

u8* cam_global_grow(struct cam_s *cam, int bytes, cam_address_t *out_address)
{
        int old_size = cam->_global_buffer_n;
        cam->_global_buffer_n += bytes;
        CAM_ASSERT(cam->_global_buffer_n <= CAM_MAX_GLOBAL);
        out_address->_v._space  = CAS_GLOBAL;
        out_address->_v._offset = old_size;
        return cam->_global_buffer + old_size;
}

bool cam_is_alive_program(struct cam_s *cam, cam_pid_t pid)
{
        return resolve<void>(cam->_id_table, u32_id(pid._u)) != nullptr;
}

bool cam_is_alive_thread(struct cam_s *cam, cam_tid_t tid)
{
        return resolve<void>(cam->_id_table, u32_id(tid._u)) != nullptr;
}
