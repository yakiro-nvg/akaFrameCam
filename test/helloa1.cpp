/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include <catch.hpp>
#include <cam.h>

#include "helloa1.cam.h"

TEST_CASE("helloa1")
{
        int ec;
        auto cam = cam_new(&ec);
        REQUIRE(cam != nullptr);
        REQUIRE(ec == CEC_SUCCESS);

        ec = cam_load_chunk(cam, HELLOA1_CAM, sizeof(HELLOA1_CAM));
        REQUIRE(ec == CEC_SUCCESS);
        auto helloa1 = cam_resolve(cam, "HELLOA1");
        REQUIRE(cam_is_alive_program(cam, helloa1));

        auto thread = cam_thread_new(cam, &ec, helloa1, nullptr, 0);
        REQUIRE(ec == CEC_SUCCESS);
        REQUIRE(cam_is_alive_thread(cam, thread));
        cam_thread_resume(cam, thread);

        cam_thread_delete(cam, thread);
        cam_delete(cam);
}