/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
namespace akaFrameCam
{
    public struct Address 
    {
        public int Value { get; set; }

        public bool IsZero() => Value == 0;
    }
}