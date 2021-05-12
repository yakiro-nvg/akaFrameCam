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
                var len = fiber.GetInt(args[1]);
                var msg = fiber.GetString(args[0], len);
                var endLine = fiber.GetBool(args[2]);
                Console.Write(msg + (endLine ? "\n" : ""));
            });

            var entry = cam.Resolve(args[0]);
            var mainFiber = cam.NewFiber(entry);
            var t = mainFiber.RunAsync();
            Console.WriteLine("mainFiber.runAsync returned");
            t.Wait();
        }
    }
}
