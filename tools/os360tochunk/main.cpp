/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include <map>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <algorithm>
#include <mapbox/variant.hpp>

#include <cam/platform.h>
#include <cam/version.h>
#include <source/common.h>
#include <source/z390_chunk.h>

enum { ALIGNMENT = 4 };

using namespace std;
using namespace akaFrame::cam;
using namespace akaFrame::cam::z390;
using namespace mapbox::util;

enum class EsdType
{
        SD = 0x0,
        LD = 0x1,
        ER = 0x2,
        WX = 0xA
};

struct Text
{
        const u8 *code;
        u32 address, size;
};

struct Relocation
{
        u32 xesd_id;
        u32  esd_id;
        u32 address;
};

struct External
{
        string name;
        u32 address;
};

struct SdProgram
{
        string name;
        u32 entry, size;
        vector<Text> texts;
        vector<External> externals;
};

struct ErProgram
{
        string name;
};

typedef variant<SdProgram, ErProgram> Program;

static string filename_wo_ext(const char *filename)
{
        string result = filename;

        const size_t last_slash_idx = result.find_last_of("\\/");
        if (string::npos != last_slash_idx)
        {
                result.erase(0, last_slash_idx + 1);
        }

        const size_t period_idx = result.rfind('.');
        if (::string::npos != period_idx)
        {
                result.erase(period_idx);
        }

        return result;
}

static string& rtrim(string &str, const string& chars = "\t\n\v\f\r ")
{
        str.erase(str.find_last_not_of(chars) + 1);
        return str;
}

inline int align_forward(int bytes)
{
        const int mod = bytes%ALIGNMENT;
        if (mod == 0) {
                return bytes;
        } else {
                return ((bytes/ALIGNMENT) + 1)*ALIGNMENT;
        }
}

static const char EBCDIC_TO_ASCII[] = {
        0x00,0x01,0x02,0x03,0x00,0x09,0x00,0x7F,0x00,0x00,0x00,0x0B,0x0C,0x0D,0x0E,0x0F,//00 ................
        0x10,0x11,0x12,0x00,0x00,0x00,0x08,0x00,0x18,0x19,0x00,0x00,0x00,0x00,0x00,0x00,//10 ................
        0x00,0x00,0x1C,0x00,0x00,0x0A,0x17,0x1B,0x00,0x00,0x00,0x00,0x00,0x05,0x06,0x07,//20 ................
        0x00,0x00,0x16,0x00,0x00,0x1E,0x00,0x04,0x00,0x00,0x00,0x00,0x14,0x15,0x00,0x1A,//30 ................
        0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2E,0x3C,0x28,0x2B,0x7C,//40  ...........<(+|
        0x26,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x21,0x24,0x2A,0x29,0x3B,0x5E,//50 &.........!$*);^
        0x2D,0x2F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2C,0x25,0x5F,0x3E,0x3F,//60 -/.........,%_>?
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x3A,0x23,0x40,0x27,0x3D,0x22,//70 .........`:#@'="
        0x00,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x00,0x7B,0x00,0x00,0x00,0x00,//80 .abcdefghi.{....
        0x00,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,0x70,0x71,0x72,0x00,0x7D,0x00,0x00,0x00,0x00,//90 .jklmnopqr.}....
        0x00,0x7E,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x00,0x00,0x00,0x5B,0x00,0x00,//A0 .~stuvwxyz...[..
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x5D,0x00,0x00,//B0 .............]..
        0x00,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x00,0x00,0x00,0x00,0x00,0x00,//C0 .ABCDEFGHI......
        0x00,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,0x50,0x51,0x52,0x00,0x00,0x00,0x00,0x00,0x00,//D0 .JKLMNOPQR......
        0x5C,0x00,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x00,0x00,0x00,0x00,0x00,0x00,//E0 \.STUVWXYZ......
        0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x00,0x00,0x00,0x00,0x00,0x00,//F0 0123456789......
};

static bool write_chunk_header(FILE *outf, int num_programs)
{
        Chunk c;
        c.signature[0] = 'C'; c.signature[1] = 'A';
        c.signature[2] = 'M'; c.signature[3] = '@';
        c.type[0] = 'Z'; c.type[1] = '3'; c.type[2] = '9'; c.type[3] = '0';
        c.ver_major = CAM_VER_MAJOR; c.ver_minor = CAM_VER_MINOR; c.ver_patch = CAM_VER_PATCH;
#if SZ_CPU_ENDIAN_BIG
        c.is_big_endian = 1;
#else
        c.is_big_endian = 0;
#endif
        c.num_programs = num_programs;

        size_t r = fwrite(&c, sizeof(c), 1, outf);
        if (r != 1) {
                printf("failed to write chunk header\n");
                return false;
        }

        return true;
}

static bool write_chunk_program_text(FILE *outf, const Text &t)
{
        size_t r;

        ChunkProgramText ct;
        ct.address = t.address;
        ct.size    = t.size;
        r = fwrite(&ct, sizeof(ct), 1, outf);
        if (r != 1) {
                printf("failed to write chunk program text\n");
                return false;
        }

        r = fwrite(t.code, 1, t.size, outf);
        if (r != t.size) {
                printf("failed to write chunk program text code\n");
                return false;
        }

        return true;
}

static bool write_chunk_program_external(FILE *outf, const External &e)
{
        size_t r;

        ChunkProgramExternal ce;
        ce.name_size = align_forward((int)e.name.length() + 1);
        ce.address   = e.address;
        r = fwrite(&ce, sizeof(ce), 1, outf);
        if (r != 1) {
                printf("failed to write chunk program external\n");
                return false;
        }

        r = fwrite(e.name.c_str(), 1, e.name.length(), outf);
        if (r != e.name.length()) {
                printf("failed to write chunk program external name");
                return false;
        }

        static char ZERO[ALIGNMENT] = { 0 };
        r = fwrite(ZERO, ce.name_size - e.name.length(), 1, outf);
        if (r != 1) {
                printf("failed to write chunk program external name padding");
                return false;
        }

        return true;
}

static bool write_chunk_program(FILE *outf, const SdProgram &p)
{
        size_t r;

        ChunkProgram cp;
        cp.entry         = p.entry;
        cp.size          = p.size;
        cp.name_size     = align_forward((int)p.name.length() + 1);
        cp.num_texts     = (int)p.texts.size();
        cp.num_externals = (int)p.externals.size();
        r = fwrite(&cp, sizeof(cp), 1, outf);
        if (r != 1) {
                printf("failed to write chunk program");
                return false;
        }

        r = fwrite(p.name.c_str(), 1, p.name.length(), outf);
        if (r != p.name.length()) {
                printf("failed to write chunk program name");
                return false;
        }

        static char ZERO[ALIGNMENT] = { 0 };
        r = fwrite(ZERO, cp.name_size - p.name.length(), 1, outf);
        if (r != 1) {
                printf("failed to write chunk program name padding");
                return false;
        }

        for (int i = 0; i < (int)p.texts.size(); ++i) {
                auto &t = p.texts[i];
                if (!write_chunk_program_text(outf, t)) {
                        return false;
                }
        }

        for (int i = 0; i < (int)p.externals.size(); ++i) {
                auto &e = p.externals[i];
                if (!write_chunk_program_external(outf, e)) {
                        return false;
                }
        }

        return true;
}

static bool to_chunk(const uint8_t *obj, int obj_size, FILE *outf)
{
        if (obj_size % 80 != 0) {
                printf("input file size must be multiple of 80\n");
                return false;
        }

        map<u32, Program> programs;
        vector<Relocation> relocations;

        // parse records
        for (int i = 0; i < obj_size; i += 80) {
                const auto record_num = i / 80 + 1;
                auto line = obj + i;
                if (line[0] != 0x02) {
                        printf("record #%08d: expected 0x02 at beginning, found 0x%02x\n", record_num, line[0]);
                        return false;
                }

                char c1 = EBCDIC_TO_ASCII[line[1]];
                char c2 = EBCDIC_TO_ASCII[line[2]];
                char c3 = EBCDIC_TO_ASCII[line[3]];

                if (c1 == 'E' && c2 == 'S' && c3 == 'D') {
                        u32 esd_id = load_uint2b(line + 14);

                        char esd_name_buff[9];
                        esd_name_buff[8] = '\0';
                        for (int i = 0; i < 8; ++i) {
                                esd_name_buff[i] = EBCDIC_TO_ASCII[line[16 + i]];
                        }
                        string esd_name = esd_name_buff;
                        rtrim(esd_name);

                        auto type = (EsdType)line[24];
                        switch (type) {
                        case EsdType::SD: {
                                SdProgram p;
                                p.name  = esd_name;
                                p.entry = load_uint3b(line + 25);
                                p.size  = load_uint3b(line + 29);
                                programs[esd_id] = p;
                                break; }
                        case EsdType::LD:
                                break;
                        case EsdType::ER: {
                                ErProgram p;
                                p.name  = esd_name;
                                programs[esd_id] = p;
                                break; }
                        case EsdType::WX:
                                break;
                        default:
                                printf("record #%0x8d: invalid ESD type 0x%02x\n", record_num, (u32)type);
                                return false;
                        }
                } else if (c1 == 'T' && c2 == 'X' && c3 == 'T') {
                        u32 esd_id = load_uint2b(line + 14);
                        auto pitr = programs.find(esd_id);
                        if (pitr == programs.end()) {
                                printf("record #%0x8d: undefined ESD 0x%02x\n", record_num, esd_id);
                        }
                        auto &p = pitr->second.get<SdProgram>();

                        Text txt;
                        txt.code    = line + 16;
                        txt.address = load_uint3b(line + 5);
                        txt.size    = load_uint2b(line + 10);
                        p.texts.push_back(txt);
                } else if (c1 == 'E' && c2 == 'N' && c3 == 'D') {
                        // nop
                } else if (c1 == 'R' && c2 == 'L' && c3 == 'D') {
                        u32 xesd_id = load_uint2b(line + 16);
                        u32  esd_id = load_uint2b(line + 18);
                        u8  sign    = ((line[20] >> 1) & 0x1);
                        u8  rld_len = ((line[20] >> 2) & 0x3) + 1;
                        u8  adcon   = ((line[20] >> 4) & 0x3);
                        u32 address = load_uint3b(line + 21);
                        if (sign != 0 || rld_len != 4 || adcon != 0) {
                                printf("record #%0x8d: only 4-bytes A type address constant supported\n", record_num);
                                return false;
                        }

                        relocations.push_back({ xesd_id, esd_id, address });
                } else {
                        printf("unknown record type '%c%c%c'\n", c1, c2, c3);
                        return false;
                }
        }

        // resolve externals
        for (auto &rld : relocations) {
                auto xesd_itr = programs.find(rld.xesd_id);
                auto  esd_itr = programs.find(rld. esd_id);
                if (xesd_itr == programs.end() || esd_itr == programs.end()) {
                        printf("bad relocation, esd not found\n");
                        return false;
                }

                auto &erp = xesd_itr->second.get<ErProgram>();
                auto &sdp =  esd_itr->second.get<SdProgram>();
                sdp.externals.push_back({ erp.name, rld.address });
        }

        if (!write_chunk_header(
                outf, (int)count_if(programs.begin(), programs.end(),
                                    [](const pair<u32, Program> &itr) {
                                        return itr.second.is<SdProgram>(); }))) {
                return false;
        }

        for (auto &itr : programs) {
                if (itr.second.is<SdProgram>()) {
                        if (!write_chunk_program(outf, itr.second.get<SdProgram>())) {
                                return false;
                        }
                }
        }

        return true;
}

int main(int argc, char *argv[])
{
        int ec;
        FILE *f;

        if (argc != 2) {
                printf("invalid number of arguments\n");
                return -1;
        }

        const char *obj_file_path = argv[1];
        f = fopen(obj_file_path, "rb");
        if (!f) {
                printf("failed to open '%s'\n", obj_file_path);
                return -1;
        }

        ec = fseek(f, 0, SEEK_END);
        if (ec != 0) {
                printf("failed to seek\n");
                return -1;
        }

        int len = (int)ftell(f);
        if (len <= 0) {
                printf("failed to get file length\n");
                return -1;
        }

        ec = fseek(f, 0, SEEK_SET);
        if (ec != 0) {
                printf("failed to seek\n");
                return -1;
        }

        void *buf = malloc(len);
        if (!buf) {
                printf("failed to allocate read buffer");
                return -1;
        }

        size_t r = fread(buf, 1, len, f);
        if (r != len) {
                printf("failed to read\n");
                return -1;
        }

        fclose(f);

        auto out_file_path = filename_wo_ext(obj_file_path) + ".cam";
        f = fopen(out_file_path.c_str(), "wb");
        if (!f) {
                printf("failed to write chunk '%s'\n", out_file_path.c_str());
                return -1;
        }

        if (!to_chunk((const uint8_t*)buf, len, f)) {
                return -1;
        }

        fclose(f);
        free(buf);

        return 0;
}