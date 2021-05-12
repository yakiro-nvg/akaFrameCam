/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_Z390_PROGRAM_H_
#define _CAM_Z390_PROGRAM_H_

#include <cam.h>
#include "z390_chunk.h"

namespace akaFrame { namespace cam { namespace z390 {

struct Z390Program {
                                Z390Program            (cam_address_t           pid
                                                      , cam_provider_t         *provider
                                                       );
                                Z390Program            (const Z390Program      &other
                                                       ) = delete;
Z390Program&                    operator=              (const Z390Program      &other
                                                       ) = delete;
                               ~Z390Program            (void
                                                       );

        cam_program_t       _p;
        cam_address_t       _pid;
        const ChunkProgram *_chunk;
        const char         *_name;
        cam_address_t       _code;
};

namespace program {

/// Returns number of bytes taken.
int                             load                   (Z390Program            &program
                                                      , const ChunkProgram     *chunk
                                                       );

}}}} // namespace akaFrame.cam.z390.program

#endif // !_CAM_Z390_PROGRAM_H_