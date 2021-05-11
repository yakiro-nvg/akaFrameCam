using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;

namespace akaFrameCam
{
    public class Cam : IDisposable
    {
        private IntPtr _cam;
        private readonly IList<byte[]> _chunks;
        private readonly IDictionary<int, Fiber> _fibers;
        private delegate void K(IntPtr cam, int fid, IntPtr ktx);
        private delegate int OnUnresolved(IntPtr cam, string name);

        internal IntPtr NativeCam { get { return _cam; } }
        public IList<string> ChunkSearchPaths { get; private set; }

        private int LoadOnDemand(IntPtr cam, string name)
        {
            foreach (var searchPath in ChunkSearchPaths)
            {
                var path = Path.Combine(searchPath, name + ".cam");
                if (File.Exists(path))
                {
                    LoadChunkFile(path);
                    var pid = Native.cam_resolve(_cam, name);
                    if (pid != 0) // found
                    {
                        return pid;
                    }
                }
            }

            return 0;
        }

        public unsafe Cam()
        {
            int ec = 0;
            _cam = Native.cam_new((IntPtr)(&ec));
            if (ec != Native.CEC_SUCCESS)
            {
                throw new CamException(ec);
            }

            _chunks = new List<byte[]>();
            _fibers = new Dictionary<int, Fiber>();
            ChunkSearchPaths = new List<string> { Directory.GetCurrentDirectory() };

            var loadOnDemand = Marshal.GetFunctionPointerForDelegate<OnUnresolved>(LoadOnDemand);
            Native.cam_on_unresolved(_cam, loadOnDemand);
        }

        ~Cam()
        {
            Dispose(false);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (_cam == IntPtr.Zero) return;

            foreach (var fid in _fibers.Keys)
            {
                Native.cam_fiber_delete(_cam, fid);
            }

            Native.cam_delete(_cam);
            _cam = IntPtr.Zero;
            GC.SuppressFinalize(this);
        }

        public void Dispose()
        {
            Dispose(true);
        }

        public void LoadChunk(byte[] buff, int buffSize)
        {
            int ec = Native.cam_load_chunk(_cam, buff, buffSize);
            if (ec != Native.CEC_SUCCESS) { throw new CamException(ec); }
            _chunks.Add(buff);
        }

        public void LoadChunkFile(string path)
        {
            var buff = File.ReadAllBytes(path);
            LoadChunk(buff, buff.Length);
        }

        public ProgramId Resolve(string name)
        {
            var pid = Native.cam_resolve(_cam, name);
            return new ProgramId { Id = pid };
        }

        private void FiberComplete(IntPtr cam, int fid, IntPtr ctx)
        {
            var fiber = _fibers[fid];
            fiber.RaiseCompleted();
        }

        public unsafe Fiber NewFiber(ProgramId entry)
        {
            int ec = 0;
            var k = Marshal.GetFunctionPointerForDelegate<K>(FiberComplete);
            int fid = Native.cam_fiber_new(_cam, (IntPtr)(&ec), entry.Id, null, 0, k, IntPtr.Zero);
            if (ec != Native.CEC_SUCCESS) { throw new CamException(ec); }
            var fiber = new Fiber(this, fid);
            _fibers.Add(fid, fiber);
            return fiber;
        }

        internal void Delete(Fiber fiber)
        {
            Native.cam_fiber_delete(_cam, fiber.Fid);
            _fibers.Remove(fiber.Fid);
        }
    }
}
