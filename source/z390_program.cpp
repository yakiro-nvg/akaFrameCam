/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include "z390_program.h"

#include "z390_common.h"
#include "z390_dispatcher.h"

#define SAVE_AREA_BYTES 72

using namespace akaFrame::cam::mem;
using namespace akaFrame::cam::id_table;

namespace akaFrame { namespace cam { namespace z390 {

static void load(struct cam_s *cam, cam_address_t pid)
{
        auto &p = get_program(cam, pid);
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

static void prepare(
        struct cam_s *cam, cam_address_t fid, cam_address_t pid, cam_address_t *args, int arity)
{
        load(cam, pid);

        auto &p = get_program(cam, pid);
        auto &m = get_machine(cam, fid, p._p.provider->index);

        m.PC = p._code._u + p._chunk->entry;

        if (arity > 0) {
                cam_address_t pa;
                auto ps = (cam_address_t*)cam_push(cam, fid, sizeof(cam_address_t)*arity, &pa);
                memcpy(ps, args, sizeof(cam_address_t)*arity);
                ps[arity - 1]._u |= 0x80000000; // set last param high bit
                m.R[1] = pa._u;
        } else {
                m.R[1] = 0;
        }

        push(cam, fid, arity);

        cam_address_t sa;
        u8 *sap = cam_push(cam, fid, SAVE_AREA_BYTES + 2, &sa);
        sap[0] = 0xff; // returned pseudo-opcode
        m.R[13] = sa._u + 2;
        m.R[14] = sa._u;
        m.R[15] = m.PC;
}

static void execute(struct cam_s *cam, cam_address_t fid, cam_address_t pid)
{
        dispatch(get_loader(get_program(cam, pid)._p.provider), fid);
}

Z390Program::Z390Program(cam_address_t pid, cam_provider_t *provider)
        : _pid(pid)
        , _chunk(nullptr)
        , _name(nullptr)
        , _code({ 0 })
{
        _p.provider = provider;
        _p.userdata = this;
        _p.load     = load;
        _p.prepare  = prepare;
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

}}}} // namespace akaFrame.cam.z390.program
