/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include "fiber.h"
#include "common.h"

using namespace akaFrame::cam::mem;
using namespace akaFrame::cam::array;
using namespace akaFrame::cam::id_table;

namespace akaFrame { namespace cam {

struct stack_frame_t {
        int           previous;
        cam_address_t pid;
};

#define NO_FRAME (-1)

static void providers_entry(struct cam_s *cam, cam_address_t fid)
{
        for (int i = 0; i < cam->_providers_n; ++i) {
                auto p = cam->_providers[i];
                p->fiber_entry(p, fid);
        }
}

static void providers_leave(struct cam_s *cam, cam_address_t fid)
{
        for (int i = 0; i < cam->_providers_n; ++i) {
                auto p = cam->_providers[i];
                p->fiber_leave(p, fid);
        }
}

inline void frame_push(Fiber &fiber, cam_address_t pid)
{
        int offset = (int)size(fiber._stack);
        auto f = (stack_frame_t*)push_many(fiber._stack, sizeof(stack_frame_t));
        f->previous  = fiber._frame; f->pid = pid;
        fiber._frame = offset;
}

inline stack_frame_t* frame_top(Fiber &fiber)
{
        CAM_ASSERT(fiber._frame != NO_FRAME);
        return (stack_frame_t*)&fiber._stack[fiber._frame];
}

inline void frame_pop(Fiber &fiber)
{
        CAM_ASSERT(fiber._frame != NO_FRAME);
        auto f = frame_top(fiber);
        resize(fiber._stack, fiber._frame);
        fiber._frame = f->previous;
}

Fiber::Fiber(
        struct cam_s *cam, cam_address_t fid, void *userdata,
        cam_address_t entry_pid, cam_address_t *args, int arity, cam_k_t k, void *ktx)
        : _cam(cam)
        , _fid(fid)
        , _stack(page_allocator())
        , _frame(NO_FRAME)
        , _tlpvs((void**)(this + 1))
        , _yielded(true)
        , _resuming(false)
        , _userdata(userdata)
{
        providers_entry(cam, _fid);

        fiber::push(*this, Continuation { k, ktx });
        frame_push(*this, entry_pid);
        auto p = resolve<cam_program_t>(cam->_id_table, entry_pid);
        p->prepare(cam, fid, entry_pid, args, arity);
        fiber::push(*this, Continuation { cam_nop_k, nullptr });
}

Fiber::~Fiber()
{
        providers_leave(_cam, _fid);
        drop(_cam->_id_table, _fid);
}

namespace fiber {

void call(Fiber &fiber, cam_address_t pid, cam_address_t *args, int arity, cam_k_t k, void *ktx)
{
        push(fiber, Continuation { k, ktx });
        frame_push(fiber, pid);
        auto p = resolve<cam_program_t>(fiber._cam->_id_table, pid);
        p->prepare(fiber._cam, fiber._fid, pid, args, arity);
        if (!fiber._yielded) {
                p->execute(fiber._cam, fiber._fid, pid);
        }
}

void go_back(Fiber &fiber)
{
        frame_pop(fiber);
}

cam_address_t top_program(Fiber &fiber)
{
        return frame_top(fiber)->pid;
}

void yield(Fiber &fiber, cam_k_t k, void *ktx)
{
        push(fiber, Continuation { k, ktx });
        fiber._yielded = true;
}

void resume(Fiber &fiber)
{
        CAM_ASSERT(!fiber._resuming);
        fiber._resuming = true;

        fiber._yielded = false;
        do {
                auto ctn = pop<Continuation>(fiber);
                ctn.k(fiber._cam, fiber._fid, ctn.ktx);
                if (fiber._yielded || fiber._frame == NO_FRAME) {
                        break;
                } else {
                        auto pid = frame_top(fiber)->pid;
                        auto p = resolve<cam_program_t>(fiber._cam->_id_table, pid);
                        p->execute(fiber._cam, fiber._fid, pid);
                }
        } while (!fiber._yielded);

        fiber._resuming = false;
}

}}} // namespace akaFrame.cam.fiber