using System.Runtime.InteropServices;

using cam_ptr_t = System.IntPtr;
using cam_error_t = System.Int32;
using cam_error_ptr_t = System.IntPtr;
using voidptr_t = System.IntPtr;
using cam_address_t = System.Int32;
using cam_k_t = System.IntPtr;
using cam_on_unresolved_t = System.IntPtr;
using cam_provider_ptr_t = System.IntPtr;
using cam_provider_fiber_entry_t = System.IntPtr;
using cam_provider_fiber_leave_t = System.IntPtr;
using cam_provider_resolve_t = System.IntPtr;
using cam_program_ptr_t = System.IntPtr;
using cam_program_load_t = System.IntPtr;
using cam_program_prepare_t = System.IntPtr;
using cam_program_execute_t = System.IntPtr;

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

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct Provider
        {
            public int _;

            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
            public char[] Name;

            public voidptr_t UserData { get; set; }

            public cam_provider_fiber_entry_t FiberEntry { get; set; }

            public cam_provider_fiber_leave_t FiberLeave { get; set; }

            public cam_provider_resolve_t Resolve { get; set; }
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct Program
        {
            public cam_provider_ptr_t Provider { get; set; }

            public voidptr_t UserData { get; set; }

            public cam_program_load_t Load { get; set; }

            public cam_program_prepare_t Prepare { get; set; }

            public cam_program_execute_t Execute { get; set; }
        }

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern cam_ptr_t cam_new(cam_error_ptr_t out_ec);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void cam_delete(cam_ptr_t cam);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern void cam_add_program(cam_ptr_t cam, string name, cam_program_ptr_t program);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern cam_error_t cam_load_chunk(cam_ptr_t cam, byte[] buff, int buff_size);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern voidptr_t cam_address_buffer(cam_ptr_t cam, cam_address_t address, cam_address_t fid);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern cam_address_t cam_resolve(cam_ptr_t cam, string name);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern cam_address_t cam_on_unresolved(cam_ptr_t cam, cam_on_unresolved_t callback);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void cam_call(
            cam_ptr_t cam, cam_address_t fid, cam_address_t pid,
            cam_address_t[] args, int arity, cam_k_t k, voidptr_t ktx);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void cam_go_back(cam_ptr_t cam, cam_address_t fid);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern cam_address_t cam_fiber_new(
            cam_ptr_t cam, cam_error_ptr_t out_ec, voidptr_t userdata, cam_address_t entry_pid,
            cam_address_t[] args, int arity, cam_k_t k, voidptr_t ktx);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void cam_fiber_delete(cam_ptr_t cam, cam_address_t fid);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern voidptr_t cam_fiber_userdata(cam_ptr_t cam, cam_address_t fid);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void cam_yield(cam_ptr_t cam, cam_address_t fid, cam_k_t k, voidptr_t ktx);

        [DllImport(CamLibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void cam_resume(cam_ptr_t cam, cam_address_t fid);
    }
}
