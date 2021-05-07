/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
namespace akaFrame { namespace cam { namespace task {

void* at(Task &task, int offset)
{
        return &task._stack[offset];
}

u8* push(Task &task, int bytes, cam_address_t *out_address)
{
        out_address->_v._space  = CAS_LOCAL_STACK;
        out_address->_v._offset = (u32)array::size(task._stack);
        return array::push_many(task._stack, bytes);
}

template <typename T>
cam_address_t push(Task &task, T value)
{
        cam_address_t address;
        *((T*)push(task, sizeof(T), &address)) = value;
        return address;
}

u8* pop(Task &task, int bytes)
{
        i64 old_size = array::size(task._stack);
        array::resize(task._stack, old_size - bytes);
        return &task._stack[array::size(task._stack)];
}

template <typename T>
T pop(Task &task)
{
        return *(T*)pop(task, sizeof(T));
}

}}} // namespace akaFrame.cam.task
