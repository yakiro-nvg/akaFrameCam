/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
namespace akaFrame { namespace cam { namespace z390 {

Z390Loader& get_loader(cam_provider_t *provider)
{
        return *(Z390Loader*)((u8*)provider - offsetof(Z390Loader, _provider));
}

Z390Machine& get_machine(struct cam_s *cam, cam_tid_t tid, int index)
{
        return *(Z390Machine*)cam_get_tlpvs(cam, tid, index);
}

Z390Program& get_program(struct cam_s *cam, cam_pid_t pid)
{
        auto p = id_table::resolve<u8>(cam->_id_table, u32_address(pid._u));
        return *(Z390Program*)(p - offsetof(Z390Program, _p));
}

}}} // namespace akaFrame.cam.z390
