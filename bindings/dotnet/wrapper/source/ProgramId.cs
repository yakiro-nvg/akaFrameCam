namespace akaFrameCam
{
    public struct ProgramId
    {
        public int Id { get; set; }

        public bool IsZero() => Id == 0;
    }
}
