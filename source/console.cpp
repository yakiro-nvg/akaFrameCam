/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include "console.h"
#include "common.h"

#include <stdio.h>

namespace akaFrame { namespace cam { namespace console {

void write_load(struct cam_s *cam, cam_pid_t pid)
{
        // nop
}

void write_prepare(
        struct cam_s *cam, cam_fid_t fid, cam_pid_t pid, cam_address_t *params, int arity)
{
        CAM_ASSERT(arity == 3);
        push(cam, fid, params);
}

void write_execute(struct cam_s *cam, cam_fid_t fid, cam_pid_t pid)
{
        auto params = pop<cam_address_t*>(cam, fid);
        auto msg      = value<const char*>(cam, fid, params[0]);
        auto len      = value<int        >(cam, fid, params[1]);
        auto end_line = value<bool       >(cam, fid, params[2]);
        printf(end_line ? "%.*s\n" : "%.*s", len, msg);
        cam_go_back(cam, fid);
}

}}} // namespace akaFrame.cam.console
