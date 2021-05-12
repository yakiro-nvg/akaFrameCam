/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_PROGRAM_TABLE_H_
#define _CAM_PROGRAM_TABLE_H_

#include <cam.h>
#include <map>
#include <string>
#include <tuple>
#include "id_table.h"

namespace akaFrame { namespace cam {

struct ProgramTable {
                                ProgramTable           (struct cam_s           *cam
                                                       );
                                ProgramTable           (const ProgramTable     &other
                                                       ) = delete;
ProgramTable&                   operator=              (const ProgramTable     &other
                                                       ) = delete;
                               ~ProgramTable           (void
                                                       );

        struct cam_s                         *_cam;
        cam_provider_t                        _provider;
        std::map<std::string, cam_address_t>  _programs;
};

namespace program_table {

inline
cam_provider_t*                 provider               (ProgramTable           &program_table
                                                       );

void                            set                    (ProgramTable           &program_table
                                                      , const char             *name
                                                      , cam_program_t          *program
                                                       );

}}} // namespace akaFrame.cam.program_table

#include "program_table.inl"

#endif // !_CAM_PROGRAM_TABLE_H_

