/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include <cam.h>

#include "ASMMIXA1.cam.h"
#include "ASMMIXA6.cam.h"

extern "C" int ASMMIXA1()
{
        cam_error_t ec;
        auto cam = cam_new(&ec);
        CAM_ASSERT(cam != nullptr);
        CAM_ASSERT(ec == CEC_SUCCESS);

        ec = cam_load_chunk(cam, ASMMIXA1_cam, sizeof(ASMMIXA1_cam));
        CAM_ASSERT(ec == CEC_SUCCESS);
        auto asmmixa1 = cam_resolve(cam, "ASMMIXA1");

        ec = cam_load_chunk(cam, ASMMIXA6_cam, sizeof(ASMMIXA6_cam));
        CAM_ASSERT(ec == CEC_SUCCESS);

        auto fiber = cam_fiber_new(
                cam, &ec, nullptr, asmmixa1, nullptr, 0, cam_nop_k, nullptr);
        CAM_ASSERT(ec == CEC_SUCCESS);
        cam_resume(cam, fiber);

        cam_fiber_delete(cam, fiber);
        cam_delete(cam);
        return 0;
}