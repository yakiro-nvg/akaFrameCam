/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
namespace akaFrameCam
{
    public struct Address
    {
        public uint Value { get; set; }

        public bool IsZero() => Value == 0;

        public static Address From(uint value) => new Address { Value = value };
    }
}