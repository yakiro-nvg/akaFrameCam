/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
using System;
using System.Threading.Tasks;

namespace akaFrameCam.NET.Runner
{
    class Program
    {
        static void Main(string[] args)
        {
            var cam = new Cam();

            cam.AddProgram("CONSOLE-WRITE", async (fiber, args) =>
            {
                await Task.Delay(500);
                var len = args[0];
                var msg = fiber.GetString(Address.From(args[1]), (int)len);
                var endLine = args[2] != 0;
                Console.Write(msg + (endLine ? "\n" : ""));
            });

            var entry = cam.Resolve(args[0]);
            var mainFiber = cam.NewFiber(entry);
            var t = mainFiber.RunAsync();
            Console.WriteLine("mainFiber.runAsync returned");
            t.Wait();
            cam.Dispose();
        }
    }
}
