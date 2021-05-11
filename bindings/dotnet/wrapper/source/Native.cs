using System.Runtime.InteropServices;

using cam_ptr_t = System.IntPtr;
using cam_error_t = System.Int32;
using cam_error_ptr_t = System.IntPtr;
using voidptr_t = System.IntPtr;
using cam_pid_t = System.Int32;
using cam_fid_t = System.Int32;
using cam_address_t = System.Int32;
using cam_k_t = System.IntPtr;
using cam_on_unresolved_t = System.IntPtr;

namespace akaFrameCam
{
    internal static class Native
    {
        private const string CamLibraryName = "akaFrameCam";

        public const int CEC_SUCCESS = 0;
        public const int CEC_UNEXPECTED = 1;
        public const int CEC_NOT_FOUND = 2;
        public const int CEC_NOT_PROGRAM = 3;
        public const int CEC_BAD_CHUNK = 4;
        public const int CEC_NOT_SUPPORTED = 5;
        public const int CEC_NO_MEMORY = 6;

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern cam_ptr_t cam_new(cam_error_ptr_t out_ec);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void cam_delete(cam_ptr_t cam);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern cam_error_t cam_load_chunk(cam_ptr_t cam, byte[] buff, int buff_size);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern cam_pid_t cam_resolve(cam_ptr_t cam, string name);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern cam_pid_t cam_on_unresolved(cam_ptr_t cam, cam_on_unresolved_t callback);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void cam_call(
            cam_ptr_t cam, cam_fid_t fid, cam_pid_t pid,
            cam_address_t[] parameters, int arity, cam_k_t k, voidptr_t ktx);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void cam_go_back(cam_ptr_t cam, cam_fid_t fid);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern cam_fid_t cam_fiber_new(
            cam_ptr_t cam, cam_error_ptr_t out_ec, cam_pid_t entry,
            cam_address_t[] parameters, int arity, cam_k_t k, voidptr_t ktx);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void cam_fiber_delete(cam_ptr_t cam, cam_fid_t fid);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void cam_yield(cam_ptr_t cam, cam_fid_t fid, cam_k_t k, voidptr_t ktx);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void cam_resume(cam_ptr_t cam, cam_fid_t fid);
    }
}
