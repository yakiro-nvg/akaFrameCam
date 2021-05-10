/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include "program_table.h"

#include "common.h"
#include "id_table.h"
#include "console.h"

using namespace akaFrame::cam::id_table;

namespace akaFrame { namespace cam {

static struct StdLibProgram { const char *name; cam_program_t program; } STD_LIB_PROGRAMS[] = {
        { "CONSOLE-WRITE", nullptr, console::write_load, console::write_prepare, console::write_execute }
};

inline ProgramTable& from_provider(struct cam_provider_s *provider)
{
        return *(ProgramTable*)((u8*)provider - offsetof(ProgramTable, _provider));
}

static void t_entry(struct cam_provider_s *provider, cam_tid_t tid)
{
        // nop
}

static void t_leave(struct cam_provider_s *provider, cam_tid_t tid)
{
        // nop
}

static cam_pid_t resolve(struct cam_provider_s *provider, const char *name)
{
        auto &pt = from_provider(provider);
        auto itr = pt._programs.find(name);
        if (itr != pt._programs.end()) {
                return itr->second;
        } else {
                return { 0 };
        }
}

static void add_standard_libs(ProgramTable &pt)
{
        for (auto &p : STD_LIB_PROGRAMS) {
                program_table::set(pt, p.name, &p.program);
        }
}

ProgramTable::ProgramTable(struct cam_s *cam)
        : _cam(cam)
{
        _provider.name[0] = 'P'; _provider.name[1] = 'T';
        _provider.name[2] = 'B'; _provider.name[3] = 'L';
        _provider.t_entry = t_entry;
        _provider.t_leave = t_leave;
        _provider.resolve = resolve;

        add_standard_libs(*this);
}

ProgramTable::~ProgramTable()
{
        for (auto p : _programs) {
                drop(_cam->_id_table, u32_address(p.second._u));
        }
}

namespace program_table {

void set(ProgramTable &pt, const char *name, cam_program_t *program)
{
        auto itr = pt._programs.find(name);
        if (itr != pt._programs.end()) {
                relocate(pt._cam->_id_table, u32_address(itr->second._u), program);
        } else {
                pt._programs[name] = { make(pt._cam->_id_table, program)._u };
        }
}

}}} // namespace akaFrame.cam.program_table
