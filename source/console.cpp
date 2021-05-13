/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include "console.h"
#include "common.h"

#include <stdio.h>

namespace akaFrame { namespace cam { namespace console {

void write_execute(struct cam_s *cam, cam_address_t fid, cam_address_t pid)
{
        auto args = cam_args(cam, fid);
        CAM_ASSERT(args.arity == 3);
        auto len      = args.at[0];
        auto msg      = value<const char*>(cam, fid, u32a(args.at[1]));
        auto end_line = args.at[2];
        printf(end_line ? "%.*s\n" : "%.*s", len, msg);
        cam_go_back(cam, fid);
}

}}} // namespace akaFrame.cam.console
