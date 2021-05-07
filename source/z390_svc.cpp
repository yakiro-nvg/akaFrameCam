/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include "z390_svc.h"

#include "common.h"
#include "z390_machine.h"

using namespace akaFrame::cam::id_table;

namespace akaFrame { namespace cam { namespace z390 {

static void wto_console_write_end(struct cam_s *cam, cam_tid_t tid)
{
        cam_thread_pop(cam, tid, sizeof(const char*) + sizeof(u32) + sizeof(bool));
        thread_pop<cam_k_t>(cam, tid)(cam, tid);
}

static void wto_console_write(
        struct cam_s *cam, cam_tid_t tid, const char *msg, int len, bool end_line, cam_k_t k)
{
        thread_push(cam, tid, k);
        cam_address_t params[] = {
                        thread_push(cam, tid, msg),
                        thread_push(cam, tid, len),
                        thread_push(cam, tid, end_line)
                };
        auto f = cam_resolve(cam, "CONSOLE-WRITE");
        cam_call(cam, tid, f, params, 3, wto_console_write_end);
}

static void wto_write_msg(struct cam_s *cam, cam_tid_t tid)
{
        auto params = thread_pop<u8*>(cam, tid);
        if ((params[3] & 0x10) != 0 && params[1] == 8) {
                auto adr = u32_address(read_uint4b(params + 4));
                auto buf = (u8*)cam_address_buffer(cam, adr, tid);
                int len = buf[1]; auto msg = (const char*)(buf + 2);
                wto_console_write(cam, tid, msg, len, true, cam_nop_k);
        } else {
                if (params[1] > 4) {
                        int len = params[1] - 4;
                        auto msg = (const char*)(params + 4);
                        wto_console_write(cam, tid, msg, len, true, cam_nop_k);
                } else {
                        CAM_ASSERT(!"not supported");
                }
        }
}

static void wto(
        Z390Loader &loader, Z390Machine &m, cam_tid_t tid, u8 svc_id)
{
        auto params = (u8*)cam_address_buffer(loader._cam, u32_address(m.R[1]), tid);
        if (params[0] == 0) {
                thread_push(loader._cam, tid, params);
                if ((read_uint2b(params + 2) & 0x80) == 0) {
                        // TODO: write timestamp
                        wto_console_write(loader._cam, tid, "", false, 0, wto_write_msg);
                } else {
                        wto_write_msg(loader._cam, tid);
                }
        } else {
                CAM_ASSERT(!"not supported");
        }

        m.R[15] = 0;
}

void svc(
        Z390Loader &loader, Z390Machine &m, cam_tid_t tid, u8 svc_id)
{
        switch (svc_id) {
        case 35: wto(loader, m, tid, svc_id); break;
        default:
                CAM_ASSERT(!"not implemented");
                break;
        }
}

}}} // namespace akaFrame.cam.z390