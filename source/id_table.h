/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#ifndef _CAM_ID_TABLE_H_
#define _CAM_ID_TABLE_H_

#include <cam.h>
#include "memory.h"
#include "array.h"

namespace akaFrame { namespace cam {

/** A table used to identify object indirectly.
\brief Provides indirect reference `id_t` that could be used instead of
direct reference (raw pointer). There are some nice properties with `id_t`.
It's a weak-reference so we know whether it's pointed to a dead object or not.
It also could be updated to point to a new address, hence relocatable, which
is needed by some module to provides fast and low-overhead serialization. */
struct CAM_API IdTable {
                                IdTable                (void
                                                       );
                                IdTable                (const IdTable          &other
                                                       );
IdTable&                        operator=              (const IdTable          &other
                                                       );
                               ~IdTable                (void
                                                       );

        struct record_t {
                bool _in_use;
                u8 _generation;
                union {
                        struct { // in-use
                                void *_p;
                        } _iu;
                        struct { // free indices
                                u32 _next;
                                u32 _prev;
                        } _fi;
                } _v;
        };

        Array<record_t> _records;
        int _num_free_indices;
};

namespace id_table {

template <typename T>
/// Returns `NULL` if id has been dropped.
T*                              resolve                (IdTable                &t
                                                      , cam_address_t           id
                                                       );

inline
void*                           relocate               (IdTable                &t
                                                      , cam_address_t           id
                                                      , void                   *new_address
                                                       );

CAM_API
cam_address_t                   make                   (IdTable                &t
                                                      , void                   *address
                                                       );

CAM_API
void                            drop                   (IdTable                &t
                                                      , cam_address_t           id
                                                       );

}}} // namespace akaFrame.cam.id_table

#include "id_table.inl"

#endif // !_CAM_ID_TABLE_H_
