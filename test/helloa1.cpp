/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include <catch.hpp>
#include <cam.h>

#include "helloa1.cam.h"

static void helloa1_end(struct cam_s *cam, cam_tid_t tid, void *ktx)
{
        printf("<HELLOA1 - END>\n");
        auto task = *(cam_tid_t*)ktx;
        cam_task_delete(cam, task);
}

TEST_CASE("helloa1")
{
        cam_error_t ec;
        auto cam = cam_new(&ec);
        REQUIRE(cam != nullptr);
        REQUIRE(ec == CEC_SUCCESS);

        ec = cam_load_chunk(cam, HELLOA1_CAM, sizeof(HELLOA1_CAM));
        REQUIRE(ec == CEC_SUCCESS);
        auto helloa1 = cam_resolve(cam, "HELLOA1");
        REQUIRE(cam_is_alive_program(cam, helloa1));

        cam_tid_t task;
        task = cam_task_new(cam, &ec, helloa1, nullptr, 0, helloa1_end, &task);
        REQUIRE(ec == CEC_SUCCESS);
        REQUIRE(cam_is_alive_task(cam, task));
        cam_resume(cam, task);

        cam_delete(cam);
}