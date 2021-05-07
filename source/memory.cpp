/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include "memory.h"

#if SX_PLATFORM_WINDOWS
#include <Windows.h>
#include <malloc.h>
#define aligned_alloc _aligned_malloc
#define aligned_free  _aligned_free
#else
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#define aligned_free free
#endif

#include <algorithm>

namespace akaFrame { namespace cam { namespace mem {

#if SX_PLATFORM_WINDOWS

struct PageAllocator : public Allocator {

i64 divisor_size(void)
{
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        return (i64)si.dwPageSize;
}

void* allocate(i64 size, i64 align)
{
        align = align < 0 ? divisor_size() : align;
        size = std::max(size, divisor_size()); 
        CAM_ASSERT(divisor_size() % align == 0 && "bad alignment");
        CAM_ASSERT(size % divisor_size()  == 0 && "bad size");
        void *m = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        CAM_ASSERT(m && "failed to allocate memory pages");
        return m;
}

void deallocate(void *p)
{
        if (!p) return;
        BOOL r = VirtualFree(p, 0, MEM_RELEASE);
        CAM_ASSERT(r != FALSE && "failed to free memory pages");
}

}; // PageAllocator

#else //  !SX_PLATFORM_WINDOWS

struct PageAllocator : public Allocator {

typedef struct alloc_size_s {
        void *p;
        i64 size;
} alloc_size_t;

i64 _num_alloc_sizes;
i64 _max_alloc_sizes;
alloc_size_t *_alloc_sizes;

PageAllocator()
{
        _num_alloc_sizes = 0;
        _max_alloc_sizes = 1024;
        _alloc_sizes = (alloc_size_t*)malloc(sizeof(alloc_size_t) * _max_alloc_sizes);
        CAM_ASSERT(_alloc_sizes && "failed to allocate memory");
}

virtual ~PageAllocator()
{
        free(_alloc_sizes);
}

i64 divisor_size(void)
{
        return (i64)getpagesize();
}

void* allocate(i64 size, i64 align)
{
        align = align < 0 ? divisor_size() : align;
        size = std::max(size, divisor_size()); 
        CAM_ASSERT(divisor_size() % align == 0 && "bad alignment");
        CAM_ASSERT(size % divisor_size()  == 0 && "bad size");
        void *m = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

        alloc_size_t *const asz = _alloc_sizes + _num_alloc_sizes;
        asz->p = m; asz->size = size;

        if (++_num_alloc_sizes >= _max_alloc_sizes) {
                _max_alloc_sizes *= 2;
                _alloc_sizes = (alloc_size_t*)realloc(
                        _alloc_sizes, sizeof(_max_alloc_sizes) * _max_alloc_sizes);
        }

        CAM_ASSERT(m && "failed to allocate memory");
        return m;
}

void deallocate(void *p)
{
        if (!p) return;

        for (i64 i = 0; i < _num_alloc_sizes; ++i) {
                alloc_size_t *const asz = _alloc_sizes + i;
                if (asz->p != p) continue;
                int r = munmap(p, asz->size);
                CAM_ASSERT(r == 0 && "failed to free memory");
                *asz = _alloc_sizes[--_num_alloc_sizes];
                return;
        }

        CAM_ASSERT(!"bad pointer");
}

}; // PageAllocator

#endif // !SX_PLATFORM_WINDOWS

struct GeneralAllocator : public Allocator {

i64 divisor_size(void)
{
        return 4;
}

void* allocate(i64 size, i64 align)
{
        align = align < 0 ? divisor_size() : align;
        size = std::max(size, divisor_size()); 
        CAM_ASSERT(divisor_size() % align == 0 && "bad alignment");
        CAM_ASSERT(size % divisor_size()  == 0 && "bad size");
        void *m = aligned_alloc(size, align);
        CAM_ASSERT(m && "failed to allocate memory");
        return m;
}

void deallocate(void *p)
{
        if (!p) return;
        aligned_free(p);
}

}; // GeneralAllocator

Allocator& page_allocator(void)
{
        static PageAllocator instance;
        return instance;
}

Allocator& general_allocator(void)
{
        static GeneralAllocator instance;
        return instance;
}

}}} // namespace akaFrame.cam.mem
