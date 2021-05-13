/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include "fiber.h"
#include "common.h"

using namespace akaFrame::cam::mem;
using namespace akaFrame::cam::array;
using namespace akaFrame::cam::id_table;

namespace akaFrame { namespace cam {

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

Fiber::Fiber(
        struct cam_s *cam, cam_address_t fid, void *userdata,
        cam_address_t entry_pid, u32 *args, int arity, cam_k_t k, void *ktx)
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
        fiber::frame_push(*this, entry_pid, args, arity);
        fiber::push(*this, Continuation { cam_nop_k, nullptr });
}

Fiber::~Fiber()
{
        providers_leave(_cam, _fid);
        drop(_cam->_id_table, _fid);
}

namespace fiber {

void call(Fiber &fiber, cam_address_t pid, u32 *args, int arity, cam_k_t k, void *ktx)
{
        push(fiber, Continuation { k, ktx });
        frame_push(fiber, pid, args, arity);
        auto p = resolve<cam_program_t>(fiber._cam->_id_table, pid);
        p->execute(fiber._cam, fiber._fid, pid);
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