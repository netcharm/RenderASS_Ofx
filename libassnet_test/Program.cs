using libass;
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
            render.SetFPS(25);
            //render.LoadAss("test.ass", "UTF-8");
            using(MemoryStream ms = new MemoryStream())
            {
                var texts = File.ReadAllText("test.ass");
                render.LoadAss(texts);
                for (int i = 100; i < 300; i++)
                {
                    Image img = render.GetAss((double)i, 1280, 720);
                    if (img is Image)
                    {
                        var fn = $"test_{i:000}.png";
                        //.PadLeft(3, '0');
                        img.Save(fn, System.Drawing.Imaging.ImageFormat.Png);
                        Console.WriteLine($"Saved subtitle file : {fn}.");
                    }
                }

            }
            Console.WriteLine("Press any key to exit...");
            Console.ReadKey();
        }
    }
}
