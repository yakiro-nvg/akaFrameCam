/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include "z390_program.h"

#include "common.h"
#include "z390_machine.h"
#include "z390_dispatcher.h"

#define SAVE_AREA_BYTES 72

using namespace akaFrame::cam::mem;
using namespace akaFrame::cam::id_table;

namespace akaFrame { namespace cam { namespace z390 {

inline Z390Program& from_pid(struct cam_s *cam, cam_pid_t pid)
{
        auto p = resolve<u8>(cam->_id_table, u32_id(pid._u));
        return *(Z390Program*)(p - offsetof(Z390Program, _p));
}

inline Z390Loader& loader(Z390Program &p)
{
        return *(Z390Loader*)((u8*)p._p.provider - offsetof(Z390Loader, _provider));
}

inline Z390Machine& machine(Z390Program &p, cam_tid_t tid)
{
        return *(Z390Machine*)*cam_thread_tlpvs(loader(p)._cam, tid, p._p.provider->index);
}

static void load(struct cam_s *cam, cam_pid_t pid)
{
        auto &p = from_pid(cam, pid);
        if (p._code._u != 0) {
                return;
        } else {
                u8 *ws = cam_global_grow(cam, p._chunk->size, &p._code);
                for (int i = 0; i < (int)p._chunk->num_texts; ++i) {
                        auto txt = (const ChunkProgramText*)((const u8*)p._chunk + p._text_offsets[i]);
                        memcpy(ws + txt->address, txt + 1, txt->size);
                }
        }
}

static void prepare(
        struct cam_s *cam, cam_tid_t tid, cam_pid_t pid,
        cam_address_t *params, int arity, cam_k_t k)
{
        load(cam, pid);

        auto &p = from_pid(cam, pid);
        auto &m = machine(p, tid);

        m.PC = p._code._u;

        if (arity > 0) {
                cam_address_t pa;
                auto ps = (cam_address_t*)cam_thread_push(cam, tid, sizeof(cam_address_t)*arity, &pa);
                memcpy(ps, params, sizeof(cam_address_t)*arity);
                ps[arity - 1]._u |= 0x80000000; // set last param high bit
                m.R[1] = pa._u;
        } else {
                m.R[1] = 0;
        }

        thread_push(cam, tid, arity);

        cam_address_t sa;
        u8 *sap = cam_thread_push(cam, tid, SAVE_AREA_BYTES + 2, &sa);
        sap[0] = 0xff; // returned pseudo-opcode
        m.R[13] = sa._u + 2;
        m.R[14] = sa._u;
        m.R[15] = m.PC;

        thread_push(cam, tid, k);
}

static void execute(struct cam_s *cam, cam_tid_t tid, cam_pid_t pid)
{
        dispatch(loader(from_pid(cam, pid)), tid);
}

Z390Program::Z390Program(cam_pid_t pid, cam_provider_t *provider)
        : _pid(pid)
        , _chunk(nullptr)
        , _name(nullptr)
        , _text_offsets(nullptr)
        , _code({ 0 })
{
        _p.provider = provider;
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
        int bytes_taken = sizeof(ChunkProgram) + chunk->name_size + sizeof(u32)*chunk->num_texts;

        program._chunk        = chunk;
        program._name         = (const char*)(chunk + 1);
        program._text_offsets = (u32*)(program._name + chunk->name_size);

        for (int i = 0; i < (int)chunk->num_texts; ++i) {
                auto txt = (const ChunkProgramText*)((const u8*)chunk + program._text_offsets[i]);
                bytes_taken += sizeof(ChunkProgramText) + txt->size;
        }

        return bytes_taken;
}

}}}} // namespace akaFrame.cam.z390.program
