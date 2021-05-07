/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include "task.h"
#include "common.h"

using namespace akaFrame::cam::mem;
using namespace akaFrame::cam::array;
using namespace akaFrame::cam::id_table;

namespace akaFrame { namespace cam {

struct stack_frame_t {
        int       previous;
        cam_pid_t pid;
};

#define NO_FRAME (-1)

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

inline void frame_push(Task &task, cam_pid_t pid)
{
        int offset = (int)size(task._stack);
        auto f = (stack_frame_t*)push_many(task._stack, sizeof(stack_frame_t));
        f->previous = task._frame; f->pid = pid;
        task._frame = offset;
}

inline stack_frame_t* frame_top(Task &task)
{
        CAM_ASSERT(task._frame != NO_FRAME);
        return (stack_frame_t*)&task._stack[task._frame];
}

inline void frame_pop(Task &task)
{
        CAM_ASSERT(task._frame != NO_FRAME);
        auto f = frame_top(task);
        resize(task._stack, task._frame);
        task._frame = f->previous;
}

Task::Task(
        struct cam_s *cam, cam_tid_t tid, cam_pid_t entry,
        cam_address_t *params, int arity, cam_k_t k, void *ktx)
        : _cam(cam)
        , _tid(tid)
        , _stack(page_allocator())
        , _frame(NO_FRAME)
        , _tlpvs((void**)(this + 1))
{
        providers_entry(cam, _tid);

        task::push(*this, Continuation { k, ktx });
        frame_push(*this, entry);
        auto p = resolve<cam_program_t>(cam->_id_table, u32_id(entry._u));
        p->prepare(cam, tid, entry, params, arity);
        task::push(*this, Continuation { cam_nop_k, nullptr });
}

Task::~Task()
{
        providers_leave(_cam, _tid);
        drop(_cam->_id_table, u32_id(_tid._u));
}

namespace task {

void call(Task &task, cam_pid_t pid, cam_address_t *params, int arity, cam_k_t k, void *ktx)
{
        push(task, Continuation { k, ktx });
        frame_push(task, pid);
        auto p = resolve<cam_program_t>(task._cam->_id_table, u32_id(pid._u));
        p->prepare(task._cam, task._tid, pid, params, arity);
        p->execute(task._cam, task._tid, pid);
}

void go_back(Task &task)
{
        frame_pop(task);
        auto ctn = pop<Continuation>(task);
        ctn.k(task._cam, task._tid, ctn.ktx);
}

cam_pid_t top_program(Task &task)
{
        return frame_top(task)->pid;
}

void yield(Task &task, cam_k_t k, void *ktx)
{
        push(task, Continuation { k, ktx });
        task._yielded = true;
}

void resume(Task &task)
{
        auto ctn = pop<Continuation>(task);
        ctn.k(task._cam, task._tid, ctn.ktx);
        task._yielded = false;
        do {
                auto pid = frame_top(task)->pid;
                auto p = resolve<cam_program_t>(task._cam->_id_table, u32_id(pid._u));
                p->execute(task._cam, task._tid, pid);
        } while (!task._yielded && task._frame != NO_FRAME);
}

}}} // namespace akaFrame.cam.task