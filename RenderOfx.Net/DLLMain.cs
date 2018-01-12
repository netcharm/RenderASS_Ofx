using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace RenderOfx.Net
{
    static class Exports
    {
        //List<OfxPlugin> plugins;

        public struct Foo
        {
            public int A;
            public int B;
            public int C;
        }

        //[DllExport("OfxGetNumberOfPlugins", CallingConvention = CallingConvention.StdCall)]
        [DllExport("OfxGetNumberOfPlugins")]
        public static int OfxGetNumberOfPlugins()
        {
            Foo foo = new Foo();
            foo.A = 22;
            foo.B = 44;
            foo.C = 55;
            return (0);
        }

        [DllExport("OfxGetPlugin")]
        public static IntPtr OfxGetPlugin(int nth)
        {
            switch (nth)
            {
                case 0:
                    return (IntPtr)0;
                default:
                    return (IntPtr)0;
            }
        }
    }
}
