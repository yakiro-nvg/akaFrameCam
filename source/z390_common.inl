/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
namespace akaFrame { namespace cam { namespace z390 {

Z390Loader& get_loader(cam_provider_t *provider)
{
        return *(Z390Loader*)provider->userdata;
}

Z390Machine& get_machine(struct cam_s *cam, cam_address_t fid, int index)
{
        return *(Z390Machine*)cam_get_tlpvs(cam, fid, index);
}

Z390Program& get_program(cam_program_t *program)
{
        return *(Z390Program*)program->userdata;
}

Z390Program& get_program(struct cam_s *cam, cam_address_t pid)
{
        auto p = id_table::resolve<u8>(cam->_id_table, pid);
        return get_program((cam_program_t*)p);
}

}}} // namespace akaFrame.cam.z390
