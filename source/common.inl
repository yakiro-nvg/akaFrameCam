/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
namespace akaFrame { namespace cam {

cam_address_t u32a(u32 u)
{
        cam_address_t a;
        a._u = u;
        return a;
}

bool is_provider(cam_provider_t *provider, const char *name)
{
        return memcmp(provider->name, name, 4) == 0;
}

u32 load_uint2b(const void *start)
{
        auto s = (const u8*)start;
#if SX_CPU_ENDIAN_BIG
#error "not implemented"
#else
        return (s[0] << 8) | s[1];
#endif
}

u32 load_uint3b(const void *start)
{
        auto s = (const u8*)start;
#if SX_CPU_ENDIAN_BIG
#error "not implemented"
#else
        return (s[0] << 16) | (s[1] << 8) | s[2];
#endif
}

u32 load_uint4b(const void *start)
{
        auto s = (const u8*)start;
#if SX_CPU_ENDIAN_BIG
#error "not implemented"
#else
        return (s[0] << 24) | (s[1] << 16) | (s[2] << 8) | s[3];
#endif
}

void save_uint4b(void *start, u32 value)
{
        auto s = (u8*)start;
        s[0] = (value >> 24) & 0xff;
        s[1] = (value >> 16) & 0xff;
        s[2] = (value >>  8) & 0xff;
        s[3] = (value      ) & 0xff;
}

template <typename T>
cam_address_t push(struct cam_s *cam, cam_address_t fid, T value)
{
        cam_address_t address;
        *((T*)cam_push(cam, fid, sizeof(T), &address)) = value;
        return address;
}

template <typename T>
T pop(struct cam_s *cam, cam_address_t fid)
{
        return *(T*)cam_pop(cam, fid, sizeof(T));
}

template <typename T>
T& value(struct cam_s *cam, cam_address_t fid, cam_address_t address)
{
        return *(T*)cam_address_buffer(cam, address, fid);
}

}} // namespace akaFrame.cam