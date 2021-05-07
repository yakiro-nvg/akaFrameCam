/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_Z390_THREAD_H_
#define _CAM_Z390_THREAD_H_

#include <cam.h>
#include "array.h"

namespace akaFrame { namespace cam {

struct Thread {
        struct StackFrame {
        };

                                Thread                 (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , cam_pid_t               entry
                                                      , cam_address_t          *params
                                                      , int                     arity
                                                       );
                                Thread                 (const Thread           &other
                                                       ) = delete;
Thread&                         operator=              (const Thread           &other
                                                       ) = delete;
                               ~Thread                 (void
                                                       );

        struct cam_s  *_cam;
        cam_tid_t      _tid;
        Array<u8>      _stack;
        void         **_tlpvs;
};

namespace thread {

inline
void*                           at                     (Thread                 &thread
                                                      , int                     offset
                                                       );

inline
u8*                             push                   (Thread                 &thread
                                                      , int                     bytes
                                                      , cam_address_t          *out_address
                                                       );

inline
u8*                             pop                    (Thread                 &thread
                                                      , int                     bytes
                                                       );

void                            call                   (Thread                 &thread
                                                      , cam_pid_t               pid
                                                      , cam_address_t          *params
                                                      , int                     arity
                                                      , cam_k_t                 k
                                                       );

void                            yield                  (Thread                 &thread
                                                      , cam_k_t                 k
                                                       );

void                            resume                 (Thread                 &thread
                                                       );

}}} // namespace akaFrame.cam.thread

#include "thread.inl"

#endif // !_CAM_Z390_THREAD_H_
