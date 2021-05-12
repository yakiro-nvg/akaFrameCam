using System;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace akaFrameCam
{
    public delegate void Continuation();

    public class Fiber : IDisposable
    {
        private Cam _cam;
        private TaskCompletionSource<object> _tcs;

        public int Id { get; internal set; }

        internal void OnCompleted()
        {
            _tcs.SetResult(null);
        }

        internal Fiber(Cam cam)
        {
            _cam = cam;
            _tcs = new TaskCompletionSource<object>();
        }

        public void Dispose()
        {
            if (Id != 0)
            {
                _cam.Delete(this);
                _cam = null;
                Id = 0;
            }
        }

        public Task RunAsync()
        {
            Native.cam_resume(_cam.NativeCam, Id);
            return _tcs.Task;
        }

        public IntPtr GetBuffer(Address address)
        {
            return Native.cam_address_buffer(_cam.NativeCam, address.Value, Id);
        }

        public unsafe string GetString(Address address, int length)
        {
            var pp = (void**)GetBuffer(address).ToPointer();
            return Marshal.PtrToStringAnsi(new IntPtr(*pp), length);
        }

        public unsafe bool GetBool(Address address)
        {
            return *(bool*)GetBuffer(address);
        }

        public unsafe int GetInt(Address address)
        {
            return *(int*)GetBuffer(address);
        }
    }
}
