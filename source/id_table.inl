/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
namespace akaFrame { namespace cam { namespace id_table {

template <typename T> T* resolve(IdTable &t, cam_address_t id)
{
        CAM_ASSERT(id._v._space == CAS_ID);
        const auto &r = t._records[id._i._index];
        if (r._in_use && r._generation == id._i._generation) {
                return (T*)r._v._iu._p;
        } else {
                return nullptr;
        }
}

void* relocate(IdTable &t, cam_address_t id, void *new_address)
{
        CAM_ASSERT(new_address);
        CAM_ASSERT(id._v._space == CAS_ID);
        auto &r = t._records[id._i._index];
        CAM_ASSERT(r._in_use && r._generation == id._i._generation && "stale id");
        void *const current = r._v._iu._p;
        r._v._iu._p = new_address;
        return current;
}

}}} // namespace akaFrame.cam.id_table
