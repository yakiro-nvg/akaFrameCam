namespace akaFrameCam
{
    public struct Address 
    {
        public int Value { get; set; }

        public bool IsZero() => Value == 0;
    }
}