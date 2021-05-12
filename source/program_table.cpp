/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include "program_table.h"

#include "common.h"
#include "id_table.h"
#include "console.h"

using namespace akaFrame::cam::id_table;

namespace akaFrame { namespace cam {

static cam_provider_t stdlib = { 0, 'S', 'T', 'D', 'L', nullptr, nullptr, nullptr, nullptr };

static struct StdLibProgram { const char *name; cam_program_t program; } STD_LIB_PROGRAMS[] = {
        { "CONSOLE-WRITE", &stdlib, nullptr, nullptr, console::write_prepare, console::write_execute }
};

inline ProgramTable& from_provider(struct cam_provider_s *provider)
{
        return *(ProgramTable*)provider->userdata;
}

static void fiber_entry(struct cam_provider_s *provider, cam_address_t fid)
{
        auto &pt = from_provider(provider);
        for (auto &pair : pt._programs) {
                auto pid = pair.second;
                auto p = id_table::resolve<cam_program_t>(pt._cam->_id_table, pid);
                if (p->provider->fiber_entry) {
                        p->provider->fiber_entry(p->provider, fid);
                }
        }
}

static void fiber_leave(struct cam_provider_s *provider, cam_address_t fid)
{
        auto &pt = from_provider(provider);
        for (auto &pair : pt._programs) {
                auto pid = pair.second;
                auto p = id_table::resolve<cam_program_t>(pt._cam->_id_table, pid);
                if (p->provider->fiber_leave) {
                        p->provider->fiber_leave(p->provider, fid);
                }
        }
}

static cam_address_t resolve(struct cam_provider_s *provider, const char *name)
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
        _provider.userdata    = this;
        _provider.fiber_entry = fiber_entry;
        _provider.fiber_leave = fiber_leave;
        _provider.resolve     = resolve;

        add_standard_libs(*this);
}

ProgramTable::~ProgramTable()
{
        for (auto p : _programs) {
                drop(_cam->_id_table, p.second);
        }
}

namespace program_table {

void set(ProgramTable &pt, const char *name, cam_program_t *program)
{
        auto itr = pt._programs.find(name);
        if (itr != pt._programs.end()) {
                relocate(pt._cam->_id_table, itr->second, program);
        } else {
                pt._programs[name] = make(pt._cam->_id_table, program);
        }
}

}}} // namespace akaFrame.cam.program_table
