/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_Z390_LOADER_H_
#define _CAM_Z390_LOADER_H_

#include <cam.h>
#include "loader.h"
#include "array.h"
#include "z390_program.h"

namespace akaFrame { namespace cam { namespace z390 {

struct Z390Loader : public Loader {
                                Z390Loader             (struct cam_s           *cam
                                                       );
virtual                        ~Z390Loader             (void
                                                       );

virtual
cam_error_t                     load_chunk             (const void             *buff
                                                      , int                     buff_size
                                                       );

virtual
cam_provider_t*                 provider               (void
                                                       );

        struct cam_s        *_cam;
        cam_provider_t       _provider;
        Array<Z390Program*>  _programs;
};

}}} // namespace akaFrame.cam.z390

#endif // !_CAM_Z390_LOADER_H_

