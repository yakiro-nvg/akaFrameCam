/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_Z390_COMMON_H_
#define _CAM_Z390_COMMON_H_

#include <cam.h>
#include "common.h"
#include "z390_loader.h"
#include "z390_machine.h"
#include "z390_program.h"

namespace akaFrame { namespace cam { namespace z390 {

inline
Z390Loader&                     get_loader             (cam_provider_t         *provider
                                                       );

inline
Z390Machine&                    get_machine            (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , int                     index
                                                       );

inline
Z390Program&                    get_program            (cam_program_t          *program
                                                       );

inline
Z390Program&                    get_program            (struct cam_s           *cam
                                                      , cam_pid_t               pid
                                                       );
}}} // namespace akaFrame.cam.z390

#include "z390_common.inl"

#endif // !_CAM_Z390_COMMON_H_

