/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include "z390_dispatcher.h"

#include "z390_common.h"
#include "z390_svc.h"

#if SX_CPU_ENDIAN_BIG
#error "not implemented"
#endif

using namespace akaFrame::cam::id_table;

namespace akaFrame { namespace cam { namespace z390 {

struct RSa
{
        inline static RSa from(const u8 *code, const Z390Machine &m)
        {
                RSa rsa;
                rsa.r1 = code[1] >> 4;
                rsa.r3 = code[1] & 0xf;
                rsa.b2 = code[2] >> 4;
                rsa.d2 = code[2] & 0xf;
                rsa.d2 = (rsa.d2 << 8) | code[3];
                rsa.a._u = m.R[rsa.b2] + rsa.d2;
                return rsa;
        }

        cam_address_t a;
        u8 r1, r3, b2;
        u32 d2;
};

struct RXa
{
        inline static RXa from(const u8 *code, const Z390Machine &m)
        {
                RXa rxa;
                rxa.r1 = code[1] >> 4;
                rxa.x2 = code[1] & 0xf;
                rxa.b2 = code[2] >> 4;
                rxa.d2 = code[2] & 0xf;
                rxa.d2 = (rxa.d2 << 8) | code[3];
                rxa.a._u = m.R[rxa.b2] + m.R[rxa.x2] + rxa.d2;
                return rxa;
        }

        cam_address_t a;
        u8 r1, x2, b2;
        u32 d2;
};

struct RXb
{
        inline static RXb from(const u8 *code, const Z390Machine &m)
        {
                RXb rxb;
                rxb.m1 = code[1] >> 4;
                rxb.x2 = code[1] & 0xf;
                rxb.b2 = code[2] >> 4;
                rxb.d2 = code[2] & 0xf;
                rxb.d2 = (rxb.d2 << 8) | code[3];
                rxb.a._u = m.R[rxb.b2] + m.R[rxb.x2] + rxb.d2;
                return rxb;
        }

        cam_address_t a;
        u8 m1, x2, b2;
        u32 d2;
};

static void stm(Z390Machine &m, u8 from, u8 to, u32 *buff)
{
#define STM_CASE(c, n) \
CASE_##c: \
        case c: \
                save_uint4b(buff, m.R[c]); \
                buff += 1; \
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

static void lm(Z390Machine &m, u8 from, u8 to, u32 *buff)
{
#define LM_CASE(c, n) \
CASE_##c: \
        case c: \
                m.R[c] = load_uint4b(buff); \
                buff += 1; \
                if (to == c) break; \
                else goto CASE_##n

        switch (from) {
        LM_CASE( 0,  1);
        LM_CASE( 1,  2);
        LM_CASE( 2,  3);
        LM_CASE( 3,  4);
        LM_CASE( 4,  5);
        LM_CASE( 5,  6);
        LM_CASE( 6,  7);
        LM_CASE( 7,  8);
        LM_CASE( 8,  9);
        LM_CASE( 9, 10);
        LM_CASE(10, 11);
        LM_CASE(11, 12);
        LM_CASE(12, 13);
        LM_CASE(13, 14);
        LM_CASE(14, 15);
        LM_CASE(15,  0);
        }
}

static u32 compare(u32 i1, u32 i2)
{
        if (i1 == i2) {
                return 0x8;
        } else if (i1 > i2) {
                return 0x2;
        } else {
                return 0x4;
        }
}

static u32 ins_size(u8 opcode)
{
        u8 t = opcode >> 6;
        return (t == 0) ? 2 : ((t == 3) ? 6 : 4);
}

static u32 ins_a7xx(
        Z390Loader &loader, cam_address_t fid, Z390Machine &m, u8 *code, bool &stop_dispatch)
{
        switch (code[1] & 0xf) {
        case 0x5: { // BRAS
                u8 r1 = code[1] >> 4;
                i32 i2 = load_uint2b(code + 2);
                m.R[r1] = m.PC + 4;
                return m.PC + i2*2; }

        default:
                CAM_ASSERT(!"not implemented");
        }

        return m.PC + ins_size(code[0]);
}

static u32 ins_lt_0x40(
        Z390Loader &loader, cam_address_t fid, Z390Machine &m, u8 *code, bool &stop_dispatch)
{
        switch (code[0]) {
        case 0x05: { // BALR
                u8 r1 = code[1] >> 4;
                u8 r2 = code[1] & 0xf;
                m.R[r1] = m.PC + 2;
                m.R[r1] |= 0x80000000;

                if (r2 != 0) {
                        return m.R[r2];
                }

                break; }

        case 0x07: { // BCR
                u8 m1 = code[1] >> 4;
                u8 r2 = code[1] & 0xf;
                if (m.CC == 0 && (m1 & 0x8) != 0 ||
                    m.CC == 1 && (m1 & 0x4) != 0 ||
                    m.CC == 2 && (m1 & 0x2) != 0 ||
                    m.CC == 3 && (m1 & 0x1) != 0) {
                        return m.R[r2];
                }

                break; }

        case 0x0a: { // SVC
                svc(loader, m, fid, code[1], stop_dispatch);
                break; }

        case 0x0d: { // BASR
                u8 r1 = code[1] >> 4;
                u8 r2 = code[1] & 0xf;
                m.R[r1] = m.PC + 2;

                if (r2 != 0) {
                        return m.R[r2];
                }

                break; }

        case 0x12: { // LTR
                u8 r1 = code[1] >> 4;
                u8 r2 = code[1] & 0xf;
                m.R[r1] = m.R[r2];
                m.CC    = compare(m.R[r2], 0);
                break; }

        case 0x1b: { // SR
                u8 r1 = code[1] >> 4;
                u8 r2 = code[1] & 0xf;
                m.R[r1] = (i32)m.R[r1] - (i32)m.R[r2];
                break; }

        default:
                CAM_ASSERT(!"not implemented");
                break;
        }

        return m.PC + ins_size(code[0]);
}

static u32 ins_lt_0x80(
        Z390Loader &loader, cam_address_t fid, Z390Machine &m, u8 *code, bool &stop_dispatch)
{
        switch (code[0]) {
        case 0x41: { // LA
                auto rxa = RXa::from(code, m);
                m.R[rxa.r1] = rxa.a._u;
                break; }

        case 0x47: { // BC
                auto rxb = RXb::from(code, m);
                if (m.CC == 0 && (rxb.m1 & 0x8) != 0 ||
                    m.CC == 1 && (rxb.m1 & 0x4) != 0 ||
                    m.CC == 2 && (rxb.m1 & 0x2) != 0 ||
                    m.CC == 3 && (rxb.m1 & 0x1) != 0) {
                        return rxb.a._u;
                }

                break; }

        case 0x50: { // ST
                auto rxa = RXa::from(code, m);
                auto buff = cam_address_buffer(loader._cam, rxa.a, fid);
                save_uint4b(buff, m.R[rxa.r1]);
                break; }

        case 0x58: { // L
                auto rxa = RXa::from(code, m);
                auto buff = cam_address_buffer(loader._cam, rxa.a, fid);
                m.R[rxa.r1] = load_uint4b(buff);
                break; }

        default:
                CAM_ASSERT(!"not implemented");
                break;
        }

        return m.PC + ins_size(code[0]);
}

static u32 ins_lt_0xc0(
        Z390Loader &loader, cam_address_t fid, Z390Machine &m, u8 *code, bool &stop_dispatch)
{
        switch (code[0]) {
        case 0x90: { // STM
                auto rsa = RSa::from(code, m);
                auto buff = cam_address_buffer(loader._cam, rsa.a, fid);
                stm(m, rsa.r1, rsa.r3, (u32*)buff);
                break; }

        case 0x98: { // LM
                auto rsa = RSa::from(code, m);
                auto buff = cam_address_buffer(loader._cam, rsa.a, fid);
                lm (m, rsa.r1, rsa.r3, (u32*)buff);
                break; }

        case 0xa7:
                return ins_a7xx(loader, fid, m, code, stop_dispatch);

        default:
                CAM_ASSERT(!"not implemented");
                break;
        }

        return m.PC + ins_size(code[0]);
}

static u32 ins_lt_0xff(
        Z390Loader &loader, cam_address_t fid, Z390Machine &m, u8 *code, bool &stop_dispatch)
{
        CAM_ASSERT(!"not implemented");
        return m.PC + ins_size(code[0]);
}

static void call_end(struct cam_s *cam, cam_address_t fid, void *ktx)
{
        auto &m = *(Z390Machine*)ktx;
        m.PC = m.R[14]; // continue at R14
        auto pid = cam_top_program(cam, fid);
        auto program = resolve<cam_program_t>(cam->_id_table, pid);
        auto &loader = get_loader(program->provider);
        dispatch(loader, fid);
}

static void call_program(struct cam_s *cam, Z390Machine &m, cam_address_t fid, cam_address_t pid)
{
        if (m.R[1] != 0) {
                auto args = (u32*)cam_address_buffer(cam, u32a(m.R[1]), fid);

                int i = 0;
                do {
                        args[i] &= 0x7FFFFFFF;
                        if ((args[i] & 0x80000000) != 0) {
                                ++i;
                                break; // last param
                        } else {
                                ++i;
                        }
                } while (true);

                cam_call(cam, fid, pid, args, i, call_end, &m);
        } else {
                cam_call(cam, fid, pid, nullptr, 0, call_end, &m);
        }
}

void dispatch(Z390Loader &loader, cam_address_t fid)
{
        auto &m = get_machine(loader._cam, fid, loader._provider.index);

        bool stop_dispatch = false;
        do {
                auto pca = u32a(m.PC);
                if (pca._v._space == CAS_ID) {
                        auto target = resolve<cam_program_t>(loader._cam->_id_table, pca);
                        if (is_provider(target->provider, "Z390")) {
                                program::prepare(get_program(target), loader._cam);
                                m.PC = get_program(target)._code._u;
                        } else {
                                call_program(loader._cam, m, fid, pca);
                                stop_dispatch = true;
                        }
                } else {
                        u8 *code = (u8*)cam_address_buffer(loader._cam, pca, fid);
                        if (code[0] == 0xff) {
                                cam_go_back(loader._cam, fid);
                                stop_dispatch = true;
                        } else {
                                if (code[0] < 0x80) {
                                        if (code[0] < 0x40) {
                                                m.PC = ins_lt_0x40(loader, fid, m, code, stop_dispatch);
                                        } else {
                                                m.PC = ins_lt_0x80(loader, fid, m, code, stop_dispatch);
                                        }
                                } else {
                                        if (code[0] < 0xc0) {
                                                m.PC = ins_lt_0xc0(loader, fid, m, code, stop_dispatch);
                                        } else {
                                                m.PC = ins_lt_0xff(loader, fid, m, code, stop_dispatch);
                                        }
                                }
                        }
                }
        } while (!stop_dispatch);
}

}}} // namespace akaFrame.cam.z390