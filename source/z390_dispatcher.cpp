/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include "z390_dispatcher.h"

#include "z390_common.h"
#include "z390_svc.h"

using namespace akaFrame::cam::id_table;

namespace akaFrame { namespace cam { namespace z390 {

static void stm(Z390Machine &m, u8 from, u8 to, u32 *save_area)
{
#define STM_CASE(c, n) \
CASE_##c: \
        case c: \
                *save_area = m.R[c]; \
                save_area += 1; \
                if (to == c) break; \
                else goto CASE_##n

        switch (from) {
        STM_CASE( 0,  1);
        STM_CASE( 1,  2);
        STM_CASE( 2,  3);
        STM_CASE( 3,  4);
        STM_CASE( 4,  5);
        STM_CASE( 5,  6);
        STM_CASE( 6,  7);
        STM_CASE( 7,  8);
        STM_CASE( 8,  9);
        STM_CASE( 9, 10);
        STM_CASE(10, 11);
        STM_CASE(11, 12);
        STM_CASE(12, 13);
        STM_CASE(13, 14);
        STM_CASE(14, 15);
        STM_CASE(15,  0);
        }
}

static u32 ins_size(u8 opcode)
{
        u8 t = opcode >> 6;
        return (t == 0) ? 2 : ((t == 3) ? 6 : 4);
}

static u32 ins_a7xx(
        Z390Loader &loader, cam_tid_t tid, Z390Machine &m, u8 *code, bool &stop_dispatch)
{
        switch (code[1] & 0xf) {
        case 0x5: { // BRAS
                u8 r1 = code[1] >> 4;
                i32 i2 = read_uint2b(code + 2);
                m.R[r1] = m.PC + 4;
                return m.PC + i2*2; }
        default:
                CAM_ASSERT(!"not implemented");
        }

        return m.PC + ins_size(code[0]);
}

static u32 ins_lt_0x40(
        Z390Loader &loader, cam_tid_t tid, Z390Machine &m, u8 *code, bool &stop_dispatch)
{
        switch (code[0]) {
        case 0x05: { // BALR
                u8 r1 = code[1] >> 4;
                u8 r2 = code[1] & 0xf;
                m.R[r1] = m.PC + 2;
                if (r2 != 0) {
                        return m.R[r2];
                }

                break; }
        case 0x0a: { // SVC
                svc(loader, m, tid, code[1], stop_dispatch);
                break; }
        default:
                CAM_ASSERT(!"not implemented");
                break;
        }

        return m.PC + ins_size(code[0]);
}

static u32 ins_lt_0x80(
        Z390Loader &loader, cam_tid_t tid, Z390Machine &m, u8 *code, bool &stop_dispatch)
{
        CAM_ASSERT(!"not implemented");
        return m.PC + ins_size(code[0]);
}

static u32 ins_lt_0xc0(
        Z390Loader &loader, cam_tid_t tid, Z390Machine &m, u8 *code, bool &stop_dispatch)
{
        switch (code[0]) {
        case 0x90: { // STM
                u8 r1 = code[1] >> 4;
                u8 r3 = code[1] & 0xf;
                u8 b2 = code[2] >> 4;
                u32 d2 = code[2] & 0xf;
#if SX_CPU_ENDIAN_BIG
#error "not implemented"
#else
                d2 = (d2 << 8) | code[3];
#endif
                auto save_area_addr = u32_address(m.R[b2] + d2);
                auto save_area = cam_address_buffer(loader._cam, save_area_addr, tid);
                stm(m, r1, r3, (u32*)save_area);
                break; }
        case 0xa7:
                return ins_a7xx(loader, tid, m, code, stop_dispatch);
        default:
                CAM_ASSERT(!"not implemented");
                break;
        }

        return m.PC + ins_size(code[0]);
}

static u32 ins_lt_0xff(
        Z390Loader &loader, cam_tid_t tid, Z390Machine &m, u8 *code, bool &stop_dispatch)
{
        CAM_ASSERT(!"not implemented");
        return m.PC + ins_size(code[0]);
}

static void call_end(struct cam_s *cam, cam_tid_t tid, void *ktx)
{
        auto &m = *(Z390Machine*)ktx;
        m.PC = m.R[14]; // continue at R14
}

void dispatch(Z390Loader &loader, cam_tid_t tid)
{
        auto &m = get_machine(loader._cam, tid, loader._provider.index);

        bool stop_dispatch = false;
        do {
                auto pca = u32_address(m.PC);
                if (pca._v._space == CAS_PROGRAM_ID) {
                        int arity = 0;
                        cam_address_t dps[CAM_MAX_ARITY];
                        if (m.R[1] != 0) {
                                auto sps = (cam_address_t*)cam_address_buffer(
                                        loader._cam, u32_address(m.R[1]), tid);
                                do {
                                        dps[arity]._u = sps[arity]._u & 0x7FFFFFFF;
                                        if ((sps[arity]._u & 0x80000000) != 0) {
                                                break; // last param
                                        } else {
                                                ++arity;
                                        }
                                } while (true);
                        }

                        cam_call(
                                loader._cam, tid,
                                { pca._u }, dps, arity,
                                call_end, &m);
                        stop_dispatch = true;
                } else {
                        u8 *code = (u8*)cam_address_buffer(loader._cam, u32_address(m.PC), tid);
                        if (code[0] == 0xff) {
                                cam_go_back(loader._cam, tid);
                                stop_dispatch = true;
                        } else {
                                if (code[0] < 0x80) {
                                        if (code[0] < 0x40) {
                                                m.PC = ins_lt_0x40(loader, tid, m, code, stop_dispatch);
                                        } else {
                                                m.PC = ins_lt_0x80(loader, tid, m, code, stop_dispatch);
                                        }
                                } else {
                                        if (code[0] < 0xc0) {
                                                m.PC = ins_lt_0xc0(loader, tid, m, code, stop_dispatch);
                                        } else {
                                                m.PC = ins_lt_0xff(loader, tid, m, code, stop_dispatch);
                                        }
                                }
                        }
                }
        } while (!stop_dispatch);
}

}}} // namespace akaFrame.cam.z390