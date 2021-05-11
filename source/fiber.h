/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_FIBER_H_
#define _CAM_FIBER_H_

#include <cam.h>
#include "array.h"

namespace akaFrame { namespace cam {

struct Fiber {
                                Fiber                  (struct cam_s           *cam
                                                      , cam_fid_t               fid
                                                      , cam_pid_t               entry
                                                      , cam_address_t          *params
                                                      , int                     arity
                                                      , cam_k_t                 k
                                                      , void                   *ktx
                                                       );
                                Fiber                  (const Fiber            &other
                                                       ) = delete;
Fiber&                          operator=              (const Fiber            &other
                                                       ) = delete;
                               ~Fiber                  (void
                                                       );

        struct cam_s  *_cam;
        cam_fid_t      _fid;
        Array<u8>      _stack;
        int            _frame;
        void         **_tlpvs;
        bool           _yielded;
};

namespace fiber {

inline
void*                           at                     (Fiber                  &fiber
                                                      , int                     offset
                                                       );

inline
u8*                             push                   (Fiber                  &fiber
                                                      , int                     bytes
                                                      , cam_address_t          *out_address
                                                       );

template <typename T>
cam_address_t                   push                   (Fiber                  &fiber
                                                      , T                       value
                                                       );

inline
u8*                             pop                    (Fiber                  &fiber
                                                      , int                     bytes
                                                       );

template <typename T>
T                               pop                    (Fiber                  &fiber
                                                       );

void                            call                   (Fiber                  &fiber
                                                      , cam_pid_t               pid
                                                      , cam_address_t          *params
                                                      , int                     arity
                                                      , cam_k_t                 k
                                                      , void                   *ktx
                                                       );

void                            go_back                (Fiber                  &fiber
                                                       );

cam_pid_t                       top_program            (Fiber                  &fiber
                                                       );

void                            yield                  (Fiber                  &fiber
                                                      , cam_k_t                 k
                                                      , void                   *ktx
                                                       );

void                            resume                 (Fiber                  &fiber
                                                       );

}}} // namespace akaFrame.cam.fiber

#include "fiber.inl"

#endif // !_CAM_FIBER_H_
