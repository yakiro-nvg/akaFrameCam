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

        cam_tid_t task;
        task = cam_task_new(cam, &ec, asmmixa1, nullptr, 0, cam_nop_k, nullptr);
        CAM_ASSERT(ec == CEC_SUCCESS);
        cam_resume(cam, task);

        cam_task_delete(cam, task);
        cam_delete(cam);
        return 0;
}