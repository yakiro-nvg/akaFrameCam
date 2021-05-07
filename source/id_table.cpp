/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
#include "id_table.h"

#define MINIMUM_FREE_INDICES 1024
#define MAX_NUM_IDS (1u << 24)

namespace akaFrame { namespace cam {

IdTable::IdTable()
        : _records(mem::page_allocator())
        , _num_free_indices(0)
{
        // root of free-list (0-index)
        auto &r = array::push(_records);
        r._in_use = false; r._v._fi._next = 0; r._v._fi._prev = 0;
}

IdTable::IdTable(const IdTable &other)
        : _records(other._records)
        , _num_free_indices(other._num_free_indices)
{
        // nop
}

IdTable& IdTable::operator=(const IdTable &other)
{
        return *this;
}

IdTable::~IdTable(void)
{
        // nop
}

namespace id_table {

id_t make(IdTable &t, void *address)
{
        CAM_ASSERT(address);

        u32 idx;

        // don't reuse too early
        if (t._num_free_indices > MINIMUM_FREE_INDICES) {
                --t._num_free_indices;
                idx = t._records[0]._v._fi._prev; // erase back
                auto &node = t._records[idx];
                auto &next = t._records[node._v._fi._next];
                auto &prev = t._records[node._v._fi._prev];
                next._v._fi._prev = node._v._fi._prev;
                prev._v._fi._next = node._v._fi._next;
        } else {
                array::push(t._records);
                const i64 back_idx = array::size(t._records) - 1;
                CAM_ASSERT(back_idx < MAX_NUM_IDS);
                idx = (u32)(back_idx);
        }

        auto &r = t._records[idx];
        r._in_use = true; r._v._iu._p = address;

        id_t id;
        id._v._index      = idx;
        id._v._generation = r._generation;
        return id;
}

void drop(IdTable &t, id_t id)
{
        if (id._v._index > 0) {
                auto &node      = t._records[id._v._index];
                auto &root      = t._records[0];
                auto &root_next = t._records[root._v._fi._next];
                CAM_ASSERT(node._generation == id._v._generation && "double drop");

                node._in_use = false;
                ++node._generation;

                // push free index to head
                ++t._num_free_indices;
                node._v._fi._next      = root._v._fi._next;
                node._v._fi._prev      = 0;
                root_next._v._fi._prev = id._v._index;
                root._v._fi._next      = id._v._index;
        }
}

}}} // namespace akaFrame.cam.id_table
