/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_Z390_MACHINE_H_
#define _CAM_Z390_MACHINE_H_

#include <cam/prereq.h>

namespace akaFrame { namespace cam { namespace z390 {

struct Z390Machine {
        u32 PC;
        u32 R[16];
};

}}} // namespace akaFrame.cam.z390

#endif // !_CAM_Z390_MACHINE_H_
