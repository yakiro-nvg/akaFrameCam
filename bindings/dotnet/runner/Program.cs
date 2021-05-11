namespace akaFrameCam.NET.Runner
{
    class Program
    {
        static void Main(string[] args)
        {
            var cam = new Cam();
            var entry = cam.Resolve(args[0]);
            var mainFiber = cam.NewFiber(entry);
            mainFiber.Resume();
        }
    }
}
