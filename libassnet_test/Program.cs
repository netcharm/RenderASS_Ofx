﻿using libass;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace libassnet_test
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.OutputEncoding = Encoding.UTF8;
            Render render = new Render();
            render.Resize(1280, 720);
            render.SetFPS(25);
            //render.LoadAss("test.ass", "UTF-8");
            using(MemoryStream ms = new MemoryStream())
            {
                TimeSpan time = new TimeSpan(0, 0, 0, 0, 0);
                var texts = File.ReadAllText("test.ass");
                render.LoadAss(texts);
                for (int i = 200; i < 750; i++)
                {
                    var watch = System.Diagnostics.Stopwatch.StartNew();
                    //Image img = render.GetAss((double)i, 1280, 720);
                    time = TimeSpan.FromSeconds(i/25.0);
                    Image img = render.GetAss(time);
                    if (img is Image)
                    {
                        var fn = $"test_{time.ToString("hhmmssfff")}.png";
                        //.PadLeft(3, '0');
                        img.Save(fn, System.Drawing.Imaging.ImageFormat.Png);
                        watch.Stop();
                        Console.WriteLine($"Saved subtitle file : {fn}, Elapsed:{watch.ElapsedMilliseconds}ms");
                    }
                }

            }
            Console.WriteLine("Press any key to exit...");
            Console.ReadKey();
        }
    }
}
