/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include "thread.h"
#include "common.h"

using namespace akaFrame::cam::mem;
using namespace akaFrame::cam::array;
using namespace akaFrame::cam::id_table;

namespace akaFrame { namespace cam {

static void providers_entry(struct cam_s *cam, cam_tid_t tid)
{
        for (int i = 0; i < cam->_providers_n; ++i) {
                auto p = cam->_providers[i];
                p->t_entry(p, tid);
        }
}

static void providers_leave(struct cam_s *cam, cam_tid_t tid)
{
        for (int i = 0; i < cam->_providers_n; ++i) {
                auto p = cam->_providers[i];
                p->t_leave(p, tid);
        }
}

Thread::Thread(
        struct cam_s *cam, cam_tid_t tid, cam_pid_t entry, cam_address_t *params, int arity)
        : _cam(cam)
        , _tid(tid)
        , _stack(page_allocator())
        , _tlpvs((void**)(this + 1))
{
        providers_entry(cam, _tid);

        auto p = resolve<cam_program_t>(cam->_id_table, u32_id(entry._u));
        p->prepare(cam, tid, entry, params, arity, cam_nop_k);
        thread::yield(*this, cam_nop_k);
}

Thread::~Thread()
{
        providers_leave(_cam, _tid);
        drop(_cam->_id_table, u32_id(_tid._u));
}

namespace thread {

void call(Thread &thread, cam_pid_t pid, cam_address_t *params, int arity, cam_k_t k)
{
        auto p = resolve<cam_program_t>(thread._cam->_id_table, u32_id(pid._u));
        p->prepare(thread._cam, thread._tid, pid, params, arity, k);
        p->execute(thread._cam, thread._tid, pid);
}

void yield(Thread &thread, cam_k_t k)
{
#if 0
        thread_push(thread._cam, thread._tid, k);
        thread_push(thread._cam, thread._tid, thread._top_pid);
        thread._yielded = true;
#endif
}

void resume(Thread &thread)
{
#if 0
        thread._yielded = false;
        auto pid = thread_pop<cam_pid_t>(thread._cam, thread._tid);
        auto p = resolve<cam_program_t>(thread._cam->_id_table, u32_id(pid._u));
        thread_pop<cam_k_t>(thread._cam, thread._tid)(thread._cam, thread._tid);
        p->execute(thread._cam, thread._tid, pid);
#endif
}

}}} // namespace akaFrame.cam.thread