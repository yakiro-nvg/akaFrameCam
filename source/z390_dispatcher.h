/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_Z390_DISPATCHER_H_
#define _CAM_Z390_DISPATCHER_H_

#include <cam.h>
#include "z390_loader.h"

namespace akaFrame { namespace cam { namespace z390 {

void                            dispatch                       (Z390Loader             &loader
                                                              , cam_fid_t               fid
                                                               );

}}} // namespace akaFrame.cam.z390

#endif // !_CAM_Z390_DISPATCHER_H_
