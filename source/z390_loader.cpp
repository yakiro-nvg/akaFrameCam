/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include "z390_loader.h"

#include <new>
#include "memory.h"
#include "id_table.h"
#include "common.h"
#include "z390_machine.h"
#include <cam/version.h>

using namespace akaFrame::cam::mem;
using namespace akaFrame::cam::array;
using namespace akaFrame::cam::id_table;
using namespace akaFrame::cam::z390::program;

namespace akaFrame { namespace cam { namespace z390 {

inline Z390Loader& from_provider(struct cam_provider_s *provider)
{
        return *(Z390Loader*)((u8*)provider - offsetof(Z390Loader, _provider));
}

static void t_entry(struct cam_provider_s *provider, cam_tid_t tid)
{
        auto &loader = from_provider(provider);
        auto m = general_allocator().allocate(sizeof(Z390Machine));
        memset(m, 0, sizeof(Z390Machine));
        cam_thread_set_tlpvs(loader._cam, tid, provider->index, m);
}

static void t_leave(struct cam_provider_s *provider, cam_tid_t tid)
{
        auto &loader = from_provider(provider);
        auto m = cam_thread_get_tlpvs(loader._cam, tid, provider->index);
        general_allocator().deallocate(m);
}

static cam_pid_t resolve(struct cam_provider_s *provider, const char *name)
{
        auto &loader = from_provider(provider);

        for (int i= 0; i < size(loader._programs); ++i) {
                auto &p = loader._programs[i];
                if (strcmp(p->_name, name) == 0) {
                        return p->_pid;
                }
        }

        return { 0 };
}

static bool verify_chunk_header(const Chunk &c)
{
        return c.signature[0] == 'C' && c.signature[1] == '@' &&
               c.signature[2] == 'M' && c.signature[3] == '#' &&
               c.type[0] == 'Z' && c.type[1] == '3' && c.type[2] == '9' && c.type[3] == '0' &&
               c.ver_major == CAM_VER_MAJOR && c.ver_minor == CAM_VER_MINOR && c.ver_patch == CAM_VER_PATCH &&
#if SZ_CPU_ENDIAN_BIG
               c.is_big_endian == true;
#else
               c.is_big_endian == false;
#endif
}

Z390Loader::Z390Loader(struct cam_s *cam)
        : _cam(cam)
        , _programs(page_allocator())
{
        _provider.t_entry = t_entry;
        _provider.t_leave = t_leave;
        _provider.resolve = resolve;
}

Z390Loader::~Z390Loader()
{
        for (int i = 0; i < size(_programs); ++i) {
                auto &p = _programs[i];
                drop(_cam->_id_table, u32_id(p->_pid._u));
                p->~Z390Program(); general_allocator().deallocate(p);
        }
}

int Z390Loader::load_chunk(const void *buff, int buff_size)
{
        auto c = *(const Chunk*)buff;
        if (buff_size < sizeof(Chunk) || !verify_chunk_header(c)) {
                return CEC_BAD_CHUNK;
        }

        int bytes_taken = sizeof(Chunk);
        for (int i = 0; i < (int)c.num_programs; ++i) {
                auto npp = general_allocator().allocate(sizeof(Z390Program));
                cam_pid_t pid = { id_u32(make(_cam->_id_table, npp)) };
                auto np = new (npp) Z390Program(pid, &_provider); push(_programs, np);
                bytes_taken += load(*np, (const ChunkProgram*)((const u8*)buff + bytes_taken));
        }

        return CEC_SUCCESS;
}

cam_provider_t* Z390Loader::provider()
{
        return &_provider;
}

}}} // namespace akaFrame.cam.z390
