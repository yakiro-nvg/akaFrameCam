using System;
using System.Threading.Tasks;

namespace akaFrameCam
{
    public delegate void Continuation();

    public class Fiber : IDisposable
    {
        private Cam _cam;
        private int _fid;
        internal int Fid { get { return _fid; } }

        public event Continuation Completed;

        internal void RaiseCompleted()
        {
            Completed?.Invoke();
        }

        internal Fiber(Cam cam, int fid)
        {
            _cam = cam;
            _fid = fid;
        }

        public void Dispose()
        {
            if (_fid != 0)
            {
                _cam.Delete(this);
                _cam = null;
                _fid = 0;
            }
        }

        public void Resume()
        {
            Native.cam_resume(_cam.NativeCam, _fid);
        }
    }
}
