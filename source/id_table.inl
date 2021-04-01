/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
namespace akaFrame { namespace cam { namespace id_table {

inline void* resolve(IdTable &t, id_t id)
{
        const auto &r = t._records[id._v._index];
        if (r._in_use && r._generation == id._v._generation) {
                return r._v._iu._p;
        } else {
                return NULL;
        }
}

inline void* relocate(IdTable &t, id_t id, void *new_address)
{
        CAM_ASSERT(new_address);
        auto &r = t._records[id._v._index];
        CAM_ASSERT(r._in_use && r._generation == id._v._generation && "stale id");
        void *const current = r._v._iu._p;
        r._v._iu._p = new_address;
        return current;
}

}}} // namespace akaFrame.cam.id_table
