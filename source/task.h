/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_Z390_TASK_H_
#define _CAM_Z390_TASK_H_

#include <cam.h>
#include "array.h"

namespace akaFrame { namespace cam {

struct Task {
                                Task                   (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , cam_pid_t               entry
                                                      , cam_address_t          *params
                                                      , int                     arity
                                                      , cam_k_t                 k
                                                      , void                   *ktx
                                                       );
                                Task                   (const Task             &other
                                                       ) = delete;
Task&                           operator=              (const Task             &other
                                                       ) = delete;
                               ~Task                   (void
                                                       );

        struct cam_s  *_cam;
        cam_tid_t      _tid;
        Array<u8>      _stack;
        int            _frame;
        void         **_tlpvs;
        bool           _yielded;
};

namespace task {

inline
void*                           at                     (Task                   &task
                                                      , int                     offset
                                                       );

inline
u8*                             push                   (Task                   &task
                                                      , int                     bytes
                                                      , cam_address_t          *out_address
                                                       );

template <typename T>
cam_address_t                   push                   (Task                   &task
                                                      , T                       value
                                                       );

inline
u8*                             pop                    (Task                   &task
                                                      , int                     bytes
                                                       );

template <typename T>
T                               pop                    (Task                   &task
                                                       );

void                            call                   (Task                   &task
                                                      , cam_pid_t               pid
                                                      , cam_address_t          *params
                                                      , int                     arity
                                                      , cam_k_t                 k
                                                      , void                   *ktx
                                                       );

void                            go_back                (Task                   &task
                                                       );

cam_pid_t                       top_program            (Task                   &task
                                                       );

void                            yield                  (Task                   &task
                                                      , cam_k_t                 k
                                                      , void                   *ktx
                                                       );

void                            resume                 (Task                   &task
                                                       );

}}} // namespace akaFrame.cam.task

#include "task.inl"

#endif // !_CAM_Z390_TASK_H_
