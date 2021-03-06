/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include "z390_loader.h"

#include <new>
#include "z390_common.h"
#include "z390_machine.h"
#include <cam/version.h>

using namespace akaFrame::cam::mem;
using namespace akaFrame::cam::array;
using namespace akaFrame::cam::id_table;
using namespace akaFrame::cam::z390::program;

namespace akaFrame { namespace cam { namespace z390 {

static void fiber_entry(struct cam_provider_s *provider, cam_address_t fid)
{
        auto &loader = get_loader(provider);
        auto m = general_allocator().allocate(sizeof(Z390Machine));
        memset(m, 0, sizeof(Z390Machine));
        cam_set_tlpvs(loader._cam, fid, provider->index, m);
}

static void fiber_leave(struct cam_provider_s *provider, cam_address_t fid)
{
        auto &loader = get_loader(provider);
        auto m = cam_get_tlpvs(loader._cam, fid, provider->index);
        general_allocator().deallocate(m);
}

static cam_address_t resolve(struct cam_provider_s *provider, const char *name)
{
        auto &loader = get_loader(provider);

        for (int i= 0; i < size(loader._programs); ++i) {
                auto &p = loader._programs[i];
                if (strcmp(p->_name, name) == 0) {
                        return p->_pid;
                }
        }

        return u32a(0);
}

static bool verify_chunk_header(const Chunk &c)
{
        return c.signature[0] == 'C' && c.signature[1] == 'A' &&
               c.signature[2] == 'M' && c.signature[3] == '@' &&
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
        _provider.name[0] = 'Z'; _provider.name[1] = '3';
        _provider.name[2] = '9'; _provider.name[3] = '0';
        _provider.userdata    = this;
        _provider.fiber_entry = fiber_entry;
        _provider.fiber_leave = fiber_leave;
        _provider.resolve     = resolve;
}

Z390Loader::~Z390Loader()
{
        for (int i = 0; i < size(_programs); ++i) {
                auto &p = _programs[i];
                drop(_cam->_id_table, p->_pid);
                p->~Z390Program(); general_allocator().deallocate(p);
        }
}

cam_error_t Z390Loader::load_chunk(const void *buff, int buff_size)
{
        auto c = *(const Chunk*)buff;
        if (buff_size < sizeof(Chunk) || !verify_chunk_header(c)) {
                return CEC_BAD_CHUNK;
        }

        int bytes_taken = sizeof(Chunk);
        for (int i = 0; i < (int)c.num_programs; ++i) {
                auto npp = general_allocator().allocate(sizeof(Z390Program));
                auto pid = make(_cam->_id_table, npp);
                auto np = new (npp) Z390Program(pid, &_provider);
                array::push(_programs, np);
                bytes_taken += load(*np, (const ChunkProgram*)((const u8*)buff + bytes_taken));
        }

        return CEC_SUCCESS;
}

cam_provider_t* Z390Loader::provider()
{
        return &_provider;
}

}}} // namespace akaFrame.cam.z390
