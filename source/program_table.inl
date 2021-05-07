/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
namespace akaFrame { namespace cam { namespace program_table {

cam_provider_t* provider(ProgramTable &program_table)
{
        return &program_table._provider;
}

}}} // namespace akaFrame.cam.program_table
