/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_Z390_CHUNK_H_
#define _CAM_Z390_CHUNK_H_

#include <cam/prereq.h>

namespace akaFrame { namespace cam { namespace z390 {

#pragma pack(push, 1)
struct Chunk
{
        u8  signature[4];
        u8  type[4];
        u8  ver_major;
        u8  ver_minor;
        u8  ver_patch;
        u8  is_big_endian;
        u32 num_programs;
};

struct ChunkProgram
{
        u32 entry;
        u32 size;
        u32 name_size;
        u32 num_texts;
        u32 num_vcons;
};

struct ChunkProgramText
{
        u32 address;
        u32 size;
};

struct ChunkProgramVcon
{
        u32 name_size;
        u32 address;
};
#pragma pack(pop)

}}} // namespace akaFrame.cam.z390

#endif // !_CAM_Z390_CHUNK_H_
