/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
namespace akaFrame { namespace cam { namespace id_table {

id_t u32_id(u32 u)
{
        id_t id; id._u = u;
        return id;
}

u32 id_u32(id_t id)
{
        return id._u;
}

template <typename T> T* resolve(IdTable &t, id_t id)
{
        const auto &r = t._records[id._v._index];
        if (r._in_use && r._generation == id._v._generation) {
                return (T*)r._v._iu._p;
        } else {
                return nullptr;
        }
}

void* relocate(IdTable &t, id_t id, void *new_address)
{
        CAM_ASSERT(new_address);
        auto &r = t._records[id._v._index];
        CAM_ASSERT(r._in_use && r._generation == id._v._generation && "stale id");
        void *const current = r._v._iu._p;
        r._v._iu._p = new_address;
        return current;
}

}}} // namespace akaFrame.cam.id_table
