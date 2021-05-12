/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
namespace akaFrame { namespace cam { namespace fiber {

void* at(Fiber &fiber, int offset)
{
        return &fiber._stack[offset];
}

u8* push(Fiber &fiber, int bytes, cam_address_t *out_address)
{
        out_address->_u = 0;
        out_address->_v._space  = CAS_LOCAL_STACK;
        out_address->_v._offset = (u32)array::size(fiber._stack);
        return array::push_many(fiber._stack, bytes);
}

template <typename T>
cam_address_t push(Fiber &fiber, T value)
{
        cam_address_t address;
        *((T*)push(fiber, sizeof(T), &address)) = value;
        return address;
}

u8* pop(Fiber &fiber, int bytes)
{
        i64 old_size = array::size(fiber._stack);
        array::resize(fiber._stack, old_size - bytes);
        return &fiber._stack[array::size(fiber._stack)];
}

template <typename T>
T pop(Fiber &fiber)
{
        return *(T*)pop(fiber, sizeof(T));
}

void* userdata(Fiber &fiber)
{
        return fiber._userdata;
}

}}} // namespace akaFrame.cam.fiber
