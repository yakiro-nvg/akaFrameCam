using System;

namespace akaFrameCam
{
    public class CamException : Exception
    {
        public readonly int ErrorCode;

        public CamException(int ec)
        {
            ErrorCode = ec;
        }
    }
}
