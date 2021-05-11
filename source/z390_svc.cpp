/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include "z390_svc.h"

#include "common.h"
#include "z390_machine.h"

using namespace akaFrame::cam::id_table;

namespace akaFrame { namespace cam { namespace z390 {

static void load(
        Z390Loader &loader, Z390Machine &m, cam_fid_t fid)
{
        char name[9];
        auto nbuf = cam_address_buffer(
                loader._cam, u32_address(m.R[0]), fid);
        memcpy(name, nbuf, 8);
        name[8] = '\0';

        auto pid = cam_resolve(loader._cam, name);
        if (pid._u != 0) {
                m.R[ 0] = pid._u;
                m.R[ 1] = 0;
                m.R[15] = 0;
        } else {
                m.R[ 0] = 0;
                m.R[ 1] = 0;
                m.R[15] = 8;
        }
}

static void wto_console_write_end(struct cam_s *cam, cam_fid_t fid, void *)
{
        cam_pop(cam, fid, sizeof(const char*) + sizeof(u32) + sizeof(bool));
        pop<cam_k_t>(cam, fid)(cam, fid, nullptr);
}

static void wto_console_write(
        struct cam_s *cam, cam_fid_t fid, const char *msg, int len, bool end_line, cam_k_t k)
{
        push(cam, fid, k);
        cam_address_t params[] = {
                        push(cam, fid, msg),
                        push(cam, fid, len),
                        push(cam, fid, end_line)
                };
        auto f = cam_resolve(cam, "CONSOLE-WRITE");
        cam_call(cam, fid, f, params, 3, wto_console_write_end, nullptr);
}

static void wto_write_msg(struct cam_s *cam, cam_fid_t fid, void *)
{
        auto params = pop<u8*>(cam, fid);
        if ((params[3] & 0x10) != 0 && params[1] == 8) {
                auto adr = u32_address(load_uint4b(params + 4));
                auto buf = (u8*)cam_address_buffer(cam, adr, fid);
                int len = buf[1]; auto msg = (const char*)(buf + 2);
                wto_console_write(cam, fid, msg, len, true, cam_nop_k);
        } else {
                if (params[1] > 4) {
                        int len = params[1] - 4;
                        auto msg = (const char*)(params + 4);
                        wto_console_write(cam, fid, msg, len, true, cam_nop_k);
                } else {
                        CAM_ASSERT(!"not supported");
                }
        }
}

static void wto(
        Z390Loader &loader, Z390Machine &m, cam_fid_t fid)
{
        m.R[15] = 0;
        auto params = (u8*)cam_address_buffer(loader._cam, u32_address(m.R[1]), fid);
        if (params[0] == 0) {
                push(loader._cam, fid, params);
                if ((load_uint2b(params + 2) & 0x80) == 0) {
                        // TODO: write timestamp
                        wto_console_write(loader._cam, fid, "", false, 0, wto_write_msg);
                } else {
                        wto_write_msg(loader._cam, fid, nullptr);
                }
        } else {
                CAM_ASSERT(!"not supported");
        }
}

void svc(
        Z390Loader &loader, Z390Machine &m, cam_fid_t fid, u8 svc_id, bool &stop_dispatch)
{
        switch (svc_id) {
        case 8: {
                load(loader, m, fid);
                break; }

        case 35: {
                wto (loader, m, fid);
                stop_dispatch = true;
                break; }

        default:
                CAM_ASSERT(!"not implemented");
                break;
        }
}

}}} // namespace akaFrame.cam.z390