/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_COMMON_H_
#define _CAM_COMMON_H_

#include <cam.h>
#include "array.h"
#include "loader.h"
#include "thread.h"
#include "memory.h"
#include "id_table.h"
#include "program_table.h"

#ifdef CAM_Z390
#include "z390_loader.h"
#endif

#define CAM_MAX_PROVIDERS 8
#define CAM_MAX_GLOBAL    16*1024*1024
#define CAM_MAX_ARITY     16

struct cam_s {
                                cam_s                  (void
                                                       );
                                cam_s                  (const cam_s            &other
                                                       ) = delete;
cam_s&                          operator=              (const cam_s            &other
                                                       ) = delete;
                               ~cam_s                  (void
                                                       );

        akaFrame::cam::IdTable                        _id_table;
        akaFrame::cam::Array<akaFrame::cam::Loader*>  _loaders;
        cam_provider_t*                               _providers[CAM_MAX_PROVIDERS];
        int                                           _providers_n;
        u8                                            _global_buffer[CAM_MAX_GLOBAL];
        int                                           _global_buffer_n;
        akaFrame::cam::id_t                           _borrows[256];
        akaFrame::cam::ProgramTable                  *_program_table;
#ifdef CAM_Z390
        akaFrame::cam::z390::Z390Loader              *_z390;
#endif
};

namespace akaFrame { namespace cam {

inline
cam_address_t                   u32_address            (u32                     u
                                                       );

inline
u32                             read_uint2b            (const u8               *start
                                                       );

inline
u32                             read_uint3b            (const u8               *start
                                                       );

inline
u32                             read_uint4b            (const u8               *start
                                                       );

template <typename T>
cam_address_t                   thread_push            (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                      , T                       value
                                                       );

template <typename T>
T                               thread_pop             (struct cam_s           *cam
                                                      , cam_tid_t               tid
                                                       );

}} // namespace akaFrame.cam

#include "common.inl"

#endif // !_CAM_COMMON_H_
