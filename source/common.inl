/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
namespace akaFrame { namespace cam {

cam_address_t u32_address(u32 u)
{
        cam_address_t a;
        a._u = u;
        return a;
}

u32 read_uint2b(const u8 *start)
{
#if SX_CPU_ENDIAN_BIG
#error "not implemented"
#else
        return (start[0] << 8) | start[1];
#endif
}

u32 read_uint3b(const u8 *start)
{
#if SX_CPU_ENDIAN_BIG
#error "not implemented"
#else
        return (start[0] << 16) | (start[1] << 8) | start[2];
#endif
}

u32 read_uint4b(const u8 *start)
{
#if SX_CPU_ENDIAN_BIG
#error "not implemented"
#else
        return (start[0] << 16) | (start[1] << 8) | start[2];
#endif
}

template <typename T>
cam_address_t push(struct cam_s *cam, cam_tid_t tid, T value)
{
        cam_address_t address;
        *((T*)cam_push(cam, tid, sizeof(T), &address)) = value;
        return address;
}

template <typename T>
T pop(struct cam_s *cam, cam_tid_t tid)
{
        return *(T*)cam_pop(cam, tid, sizeof(T));
}

template <typename T>
T& value(struct cam_s *cam, cam_tid_t tid, cam_address_t address)
{
        return *(T*)cam_address_buffer(cam, address, tid);
}

}} // namespace akaFrame.cam