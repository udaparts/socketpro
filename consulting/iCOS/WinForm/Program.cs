using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

namespace WinForm
{
    static class Program
    {
        static CConnectionContext[,] loadConnectionContexts()
        {
            CConnectionContext[,] ccs = new CConnectionContext[1, 1];
            ccs[0, 0] = new CConnectionContext("localhost", 20901, "SocketPro", "PassOne");
            return ccs;
        }
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            CConnectionContext[,] ccs = loadConnectionContexts();
            int threads = ccs.GetLength(0);
            int sockets_per_thread = ccs.GetLength(1);

            //start one instance of lookup with one socket pool connected to a number of remote SocketPro servers
            bool working = iCOS.Lookup.Initialize(  ccs,
                                                    2, //must not be larger than DATASIZE defined in icosdefines.h
                                                    5000); 
            Application.Run(new frmMain());
            //shutdown the single instance at last
            iCOS.Lookup.Shutdown();
        }
    }
}
