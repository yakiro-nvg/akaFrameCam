/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include "z390_program.h"

#include "z390_common.h"
#include "z390_dispatcher.h"

#define SAVE_AREA_BYTES 72

using namespace akaFrame::cam::mem;
using namespace akaFrame::cam::id_table;

namespace akaFrame { namespace cam { namespace z390 {

static void execute(struct cam_s *cam, cam_address_t fid, cam_address_t pid)
{
        auto &p = get_program(cam, pid);
        auto &m = get_machine(cam, fid, p._p.provider->index);

        program::prepare(p, cam);

        auto bp = cam_bp(cam, fid);
        if (bp._u == cam_sp(cam, fid)._u) { // first execute
                m.PC = p._code._u + p._chunk->entry;

                auto args = cam_args(cam, fid);
                if (args.arity > 0) {
                        // set last param high bit by convention
                        args.at[args.arity - 1] |= 0x80000000;
                        m.R[1] = bp._u;
                } else {
                        m.R[1] = 0;
                }

                cam_address_t sa;
                u8 *sap = cam_push(cam, fid, SAVE_AREA_BYTES + 2, &sa);
                sap[0] = 0xff; // returned pseudo-opcode
                m.R[13] = sa._u + 2;
                m.R[14] = sa._u;
                m.R[15] = m.PC;
        }

        dispatch(get_loader(p._p.provider), fid);
}

Z390Program::Z390Program(cam_address_t pid, cam_provider_t *provider)
        : _pid(pid)
        , _chunk(nullptr)
        , _name(nullptr)
        , _code(u32a(0))
{
        _p.provider = provider;
        _p.userdata = this;
        _p.execute  = execute;
}

Z390Program::~Z390Program()
{
        // nop
}

namespace program {

int load(Z390Program &program, const ChunkProgram *chunk)
{
        int bytes_taken = sizeof(ChunkProgram) + chunk->name_size;

        program._chunk = chunk;
        program._name  = (const char*)(chunk + 1);

        for (int i = 0; i < (int)chunk->num_texts; ++i) {
                auto txt = (const ChunkProgramText*)((const u8*)chunk + bytes_taken);
                bytes_taken += sizeof(ChunkProgramText) + txt->size;
        }

        for (int i = 0; i < (int)chunk->num_externals; ++i) {
                auto ext = (const ChunkProgramExternal*)((const u8*)chunk + bytes_taken);
                bytes_taken += sizeof(ChunkProgramExternal) + ext->name_size;
        }

        return bytes_taken;
}

void prepare(Z390Program &p, struct cam_s *cam)
{
        if (p._code._u != 0) {
                return;
        } else {
                u8 *ws = cam_global_grow(cam, p._chunk->size, &p._code);
                int bytes_taken = sizeof(ChunkProgram) + p._chunk->name_size;

                for (int i = 0; i < (int)p._chunk->num_texts; ++i) {
                        auto txt = (const ChunkProgramText*)((const u8*)p._chunk + bytes_taken);
                        memcpy(ws + txt->address, txt + 1, txt->size);
                        bytes_taken += sizeof(ChunkProgramText) + txt->size;
                }

                for (int i = 0; i < (int)p._chunk->num_externals; ++i) {
                        auto ext = (const ChunkProgramExternal*)((const u8*)p._chunk + bytes_taken);
                        bytes_taken += sizeof(ChunkProgramExternal) + ext->name_size;
                        auto address = (cam_address_t*)(ws + ext->address);
                        auto name = (const char*)(ext + 1);
                        auto pid = cam_resolve(cam, name);
                        CAM_ASSERT(pid._u != 0 && "unresolved program");
                        save_uint4b(address, pid._u);
                }
        }
}

}}}} // namespace akaFrame.cam.z390.program
