/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_ARRAY_H_
#define _CAM_ARRAY_H_

#include "prereq.h"
#include "memory.h"

namespace akaFrame { namespace cam {

/// Resizable array of POD (no destructor) items.
template <typename T>
struct Array {
                        Array                  (mem::Allocator         &allocator
                                               );
                        Array                  (const Array            &other
                                               );
Array&                  operator=              (const Array            &other
                                               );
                       ~Array                  (void
                                               );

                        operator bool          (void
                                               ) const;
T&                      operator[]             (i64                     i
                                               );
const T&                operator[]             (i64                     i
                                               ) const;

        mem::Allocator &_allocator;
        i64 _size, _capacity;
        T *_items;
};

namespace array {

template <typename T>
i64                     size                   (const Array<T>         &a
                                               );

template <typename T>
void                    reserve                (Array<T>               &a
                                              , i64                     capacity
                                               );

template <typename T>
void                    resize                 (Array<T>               &a
                                              , i64                     new_size
                                               );

template <typename T>
void                    push                   (Array<T>               &a
                                              , const T                &item
                                               );

template <typename T>
T&                      push                   (Array<T>               &a
                                               );

template <typename T>
void                    push_unsafe            (Array<T>               &a
                                              , const T                &item
                                               );

template <typename T>
T&                      push_unsafe            (Array<T>               &a
                                               );

/// Returns pointer to the first added item.
template <typename T>
T*                      push_many              (Array<T>               &a
                                              , i64                     num_items
                                               );

}}} // namespace akaFrame.cam.array

#include "array.inl"

#endif // !_CAM_ARRAY_H_
