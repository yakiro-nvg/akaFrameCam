/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
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
