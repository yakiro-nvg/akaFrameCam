/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include "console.h"
#include "common.h"

#include <stdio.h>

namespace akaFrame { namespace cam { namespace console {

void write_load(struct cam_s *cam, cam_address_t pid)
{
        // nop
}

void write_prepare(
        struct cam_s *cam, cam_address_t fid, cam_address_t pid, cam_address_t *args, int arity)
{
        CAM_ASSERT(arity == 3);
        push(cam, fid, args);
}

void write_execute(struct cam_s *cam, cam_address_t fid, cam_address_t pid)
{
        auto args = pop<cam_address_t*>(cam, fid);
        auto msg      = value<const char*>(cam, fid, args[0]);
        auto len      = value<i32        >(cam, fid, args[1]);
        auto end_line = value<u8         >(cam, fid, args[2]);
        printf(end_line ? "%.*s\n" : "%.*s", len, msg);
        cam_go_back(cam, fid);
}

}}} // namespace akaFrame.cam.console
