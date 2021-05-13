/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
namespace akaFrame { namespace cam { namespace fiber {

void* at(Fiber &fiber, int offset)
{
        return &fiber._stack[offset];
}

template <typename T>
T at(Fiber &fiber, int offset)
{
        return *(T*)at(fiber, offset);
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
        cam_address_t adr;
        *((T*)push(fiber, sizeof(T), &adr)) = value;
        return adr;
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

void frame_push(Fiber &fiber, cam_address_t pid, u32 *args, int arity)
{
        int offset = (int)array::size(fiber._stack);
        auto f = (stack_frame_t*)array::push_many(fiber._stack, sizeof(stack_frame_t));
        f->previous  = fiber._frame; f->pid = pid;
        fiber._frame = offset;
        
        for (int i = 0; i < arity; ++i) {
                push(fiber, args[i]);
        }

        push(fiber, arity);
        f->bp = (int)array::size(fiber._stack);
}

stack_frame_t* frame_top(Fiber &fiber)
{
        CAM_ASSERT(fiber._frame != NO_FRAME);
        return (stack_frame_t*)&fiber._stack[fiber._frame];
}

void frame_pop(Fiber &fiber)
{
        CAM_ASSERT(fiber._frame != NO_FRAME);
        auto f = frame_top(fiber);
        array::resize(fiber._stack, fiber._frame);
        fiber._frame = f->previous;
}

cam_address_t bp(Fiber &fiber)
{
        cam_address_t adr;
        adr._u = 0;
        adr._v._space  = CAS_LOCAL_STACK;
        adr._v._offset = frame_top(fiber)->bp;
        return adr;
}

cam_address_t sp(Fiber &fiber)
{
        cam_address_t adr;
        adr._u = 0;
        adr._v._space  = CAS_LOCAL_STACK;
        adr._v._offset = array::size(fiber._stack);
        return adr;
}

cam_args_t args(Fiber &fiber)
{
        cam_args_t r;
        int bp = frame_top(fiber)->bp;
        int ad = bp - sizeof(int);
        r.arity = at<int>(fiber, ad);
        int bd = sizeof(cam_address_t)*r.arity;
        r.at = (u32*)at(fiber, ad - bd);
        return r;
}

int arity(Fiber &fiber)
{
        int bp = frame_top(fiber)->bp;
        return at<int>(fiber, bp - sizeof(int));
}

void* userdata(Fiber &fiber)
{
        return fiber._userdata;
}

}}} // namespace akaFrame.cam.fiber
