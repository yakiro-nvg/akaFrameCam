/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_Z390_THREAD_H_
#define _CAM_Z390_THREAD_H_

#include <cam.h>

namespace akaFrame { namespace cam {

struct Thread {
        struct StackFrame {
        };

                                Thread                 (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , int                     stack_size
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

        struct cam_s *_cam;
        cam_tid_t     _tid;
        int           _stack_size;
        u8           *_local_storage;
        int           _local_storage_n;
        cam_pid_t     _top_pid;
        bool          _yielded;
};

namespace thread {

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

#endif // !_CAM_Z390_THREAD_H_
