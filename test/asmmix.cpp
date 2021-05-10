/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include <catch.hpp>
#include <cam.h>

#include "ASMMIXA1.cam.h"
#include "ASMMIXA6.cam.h"

TEST_CASE("asmmix")
{
        cam_error_t ec;
        auto cam = cam_new(&ec);
        REQUIRE(cam != nullptr);
        REQUIRE(ec == CEC_SUCCESS);

        ec = cam_load_chunk(cam, ASMMIXA1_cam, sizeof(ASMMIXA1_cam));
        REQUIRE(ec == CEC_SUCCESS);
        auto asmmixa1 = cam_resolve(cam, "ASMMIXA1");

        ec = cam_load_chunk(cam, ASMMIXA6_cam, sizeof(ASMMIXA6_cam));
        REQUIRE(ec == CEC_SUCCESS);
        auto asmmixa6 = cam_resolve(cam, "ASMMIXA6");

        cam_tid_t task;
        task = cam_task_new(cam, &ec, asmmixa1, nullptr, 0, cam_nop_k, nullptr);
        REQUIRE(ec == CEC_SUCCESS);
        cam_resume(cam, task);

        cam_task_delete(cam, task);
        cam_delete(cam);
}