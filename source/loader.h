/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_LOADER_H_
#define _CAM_LOADER_H_

#include <cam.h>

namespace akaFrame { namespace cam {

struct Loader {
                                Loader                 (void
                                                       ) { }
                                Loader                 (const Loader           &other
                                                       ) = delete;
Loader&                         operator=              (const Loader           &other
                                                       ) = delete;

virtual                        ~Loader                 (void
                                                       ) { }

virtual
cam_error_t                     load_chunk             (const void             *buff
                                                      , int                     buff_size
                                                       ) = 0;

virtual
cam_provider_t*                 provider               (void
                                                       ) = 0;
};

}} // namespace akaFrame.cam

#endif // !_CAM_LOADER_H_

