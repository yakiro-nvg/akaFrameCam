/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_Z390_SVC_H_
#define _CAM_Z390_SVC_H_

#include <cam.h>
#include "z390_loader.h"

namespace akaFrame { namespace cam { namespace z390 {

struct Z390Machine;

void                            svc                            (Z390Loader             &loader
                                                              , Z390Machine            &m
                                                              , cam_tid_t               tid
                                                              , u8                      svc_id
                                                              , bool                   &stop_dispatch
                                                               );

}}} // namespace akaFrame.cam.z390

#endif // !_CAM_Z390_SVC_H_
