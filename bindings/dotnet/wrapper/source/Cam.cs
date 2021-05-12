using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace akaFrameCam
{
    public class Cam : IDisposable
    {
        private delegate void K(IntPtr cam, int fid, IntPtr ktx);
        private delegate int OnUnresolved(IntPtr cam, string name);
        private delegate void FiberEntry(IntPtr provider, int fid);
        private delegate void FiberLeave(IntPtr provider, int fid);
        private delegate void ProgramPrepare(
            IntPtr cam, int fid, int pid, IntPtr args, int arity);
        private delegate void ProgramExecute(IntPtr cam, int fid, int pid);

        private class ProgramRecord
        {
            public Func<Fiber, Address[], Task> Execute { get; set; }

            public Action Load { get; set; }

            public Action<Fiber> FiberEntry { get; set; }

            public Action<Fiber> FiberLeave { get; set; }

            public IntPtr ProviderPtr { get; set; }

            public IntPtr ProgramPtr { get; set; }
        }

        private IntPtr _cam;
        private readonly IList<byte[]> _chunks;
        private readonly IDictionary<int, Tuple<GCHandle, Fiber>> _fibers;
        private readonly IList<ProgramRecord> _programs;

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
            _fibers = new Dictionary<int, Tuple<GCHandle, Fiber>>();
            _programs = new List<ProgramRecord>();

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
            if (_cam == IntPtr.Zero) { return; }

            foreach (var fiber in _fibers.Values)
            {
                Native.cam_fiber_delete(_cam, fiber.Item2.Id);
                fiber.Item1.Free();
            }

            Native.cam_delete(_cam);
            _cam = IntPtr.Zero;

            foreach (var p in _programs)
            {
                Marshal.FreeHGlobal(p.ProviderPtr);
                Marshal.FreeHGlobal(p.ProgramPtr);
            }

            GC.SuppressFinalize(this);
        }

        public void Dispose()
        {
            Dispose(true);
        }

        private void NopK(IntPtr cam, int fid, IntPtr ktx)
        {
            // nop
        }

        private void OnFiberEntry(IntPtr provider, int fid)
        {
            var np = Marshal.PtrToStructure<Native.Provider>(provider);
            var program = _programs[np.UserData.ToInt32()];
            var fud = Native.cam_fiber_userdata(_cam, fid);
            var fiber = (Fiber)GCHandle.FromIntPtr(fud).Target;
            fiber.Id = fid; // earliest time for it
            program.FiberEntry?.Invoke(fiber);
        }

        private void OnFiberLeave(IntPtr provider, int fid)
        {
            var np = Marshal.PtrToStructure<Native.Provider>(provider);
            var program = _programs[np.UserData.ToInt32()];
            var fud = Native.cam_fiber_userdata(_cam, fid);
            var fiber = (Fiber)GCHandle.FromIntPtr(fud).Target;
            program.FiberLeave?.Invoke(fiber);
        }

        private unsafe void OnProgramPrepare(
            IntPtr cam, int fid, int pid, IntPtr args, int arity)
        {
            var pp = Native.cam_address_buffer(_cam, pid, fid);
            var np = Marshal.PtrToStructure<Native.Program>(pp);
            var program = _programs[np.UserData.ToInt32()];
            var fud = Native.cam_fiber_userdata(_cam, fid);
            var fiber = (Fiber)GCHandle.FromIntPtr(fud).Target;

            var dargs = new Address[arity];
            for (int i = 0; i < arity; ++i)
            {
                dargs[i].Value = ((int*)args)[i];
            }

            var task = program.Execute(fiber, dargs);
            if (task.IsCompleted)
            {
                Native.cam_go_back(_cam, fid);
            }
            else
            {
                task.ContinueWith(_ =>
                {
                    Native.cam_go_back(_cam, fid);
                    Native.cam_resume(_cam, fiber.Id);
                });

                Native.cam_yield(_cam, fid, Marshal.GetFunctionPointerForDelegate<K>(NopK), IntPtr.Zero);
            }
        }

        private void OnProgramExecute(IntPtr cam, int fid, int pid)
        {
            // nop
        }

        public void AddProgram(
            string name, Func<Fiber, Address[], Task> execute,
            Action<Fiber> fiberEntry = null, Action<Fiber> fiberLeave = null)
        {
            var pr = new ProgramRecord
            {
                Execute = execute,
                FiberEntry = fiberEntry,
                FiberLeave = fiberLeave
            };
            _programs.Add(pr);

            var provider = new Native.Provider { Name = new char[] { '.', 'N', 'E', 'T' } };
            provider.UserData = new IntPtr(_programs.Count - 1);
            provider.FiberEntry = Marshal.GetFunctionPointerForDelegate<FiberEntry>(OnFiberEntry);
            provider.FiberLeave = Marshal.GetFunctionPointerForDelegate<FiberLeave>(OnFiberLeave);
            pr.ProviderPtr = Marshal.AllocHGlobal(Marshal.SizeOf<Native.Provider>());
            Marshal.StructureToPtr(provider, pr.ProviderPtr, false);

            var program = new Native.Program { Provider = pr.ProviderPtr };
            program.UserData = new IntPtr(_programs.Count - 1);
            program.Prepare = Marshal.GetFunctionPointerForDelegate<ProgramPrepare>(OnProgramPrepare);
            program.Execute = Marshal.GetFunctionPointerForDelegate<ProgramExecute>(OnProgramExecute);
            pr.ProgramPtr = Marshal.AllocHGlobal(Marshal.SizeOf<Native.Program>());
            Marshal.StructureToPtr(program, pr.ProgramPtr, false);

            Native.cam_add_program(_cam, name, pr.ProgramPtr);
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

        public Address Resolve(string name)
        {
            var pid = Native.cam_resolve(_cam, name);
            return new Address { Value = pid };
        }

        private void FiberComplete(IntPtr cam, int fid, IntPtr ctx)
        {
            var ud = Native.cam_fiber_userdata(_cam, fid);
            var fiber = (Fiber)GCHandle.FromIntPtr(ud).Target;
            fiber.OnCompleted();
        }

        public unsafe Fiber NewFiber(Address entryPid)
        {
            int ec = 0;
            var fiber = new Fiber(this);
            var tpl = Tuple.Create(GCHandle.Alloc(fiber), fiber);
            var userdata = GCHandle.ToIntPtr(tpl.Item1);
            var k = Marshal.GetFunctionPointerForDelegate<K>(FiberComplete);
            var fid = Native.cam_fiber_new(
                _cam, (IntPtr)(&ec), userdata, entryPid.Value, null, 0, k, IntPtr.Zero);
            if (ec != Native.CEC_SUCCESS) {
                tpl.Item1.Free();
                throw new CamException(ec);
            }

            _fibers.Add(fid, tpl);
            return fiber;
        }

        internal void Delete(Fiber fiber)
        {
            Native.cam_fiber_delete(_cam, fiber.Id);
            _fibers[fiber.Id].Item1.Free();
            _fibers.Remove(fiber.Id);
        }
    }
}
