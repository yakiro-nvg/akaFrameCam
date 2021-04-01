/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
// Various of utilities for memory management.
#ifndef _CAM_MEMORY_H_
#define _CAM_MEMORY_H_

#include "prereq.h"

namespace akaFrame { namespace cam { namespace mem {

struct Allocator {

static const i64 DIVISOR_ALIGN = -1;

virtual                ~Allocator              (void) { }

virtual i64             divisor_size           (void
                                               ) = 0;

virtual void*           allocate               (i64                     size
                                              , i64                     align = DIVISOR_ALIGN
                                               ) = 0;

virtual void            deallocate             (void                   *p
                                               ) = 0;

}; // class Allocator

CAM_API
/// Paged memory allocator (with MMU).
Allocator&              page_allocator         (void
                                               );

CAM_API
/// General-purpose allocator.
Allocator&              general_allocator      (void
                                               );

}}} // namespace akaFrame.cam.mem

#endif // !_CAM_MEMORY_H_
