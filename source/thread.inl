/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
namespace akaFrame { namespace cam { namespace thread {

void* at(Thread &thread, int offset)
{
        return &thread._stack[offset];
}

u8* push(Thread &thread, int bytes, cam_address_t *out_address)
{
        out_address->_v._space  = CAS_THREAD_STACK;
        out_address->_v._offset = (u32)array::size(thread._stack);
        return array::push_many(thread._stack, bytes);
}

u8* pop(Thread &thread, int bytes)
{
        i64 old_size = array::size(thread._stack);
        array::resize(thread._stack, old_size - bytes);
        return &thread._stack[array::size(thread._stack)];
}

}}} // namespace akaFrame.cam.thread
