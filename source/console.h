/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_CONSOLE_H_
#define _CAM_CONSOLE_H_

#include <cam.h>

namespace akaFrame { namespace cam { namespace console {

void                            write_load             (struct cam_s           *cam
                                                      , cam_pid_t               pid
                                                       );

void                            write_prepare          (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , cam_pid_t               pid
                                                      , cam_address_t          *params
                                                      , int                     arity
                                                      , cam_k_t                 k
                                                       );

void                            write_execute          (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , cam_pid_t               pid
                                                       );

}}} // namespace akaFrame.cam.console

#endif // !_CAM_CONSOLE_H_

