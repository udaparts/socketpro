using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Security;
using System.Web.SessionState;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

namespace WebForm
{
    public class Global : System.Web.HttpApplication
    {
        private CConnectionContext[,] LoadConnectionContexts()
        {
            CConnectionContext[,] ccs = new CConnectionContext[1, 1];
            ccs[0, 0] = new CConnectionContext("localhost", 20901, "SocketPro", "PassOne");
            return ccs;
        }

        protected void Application_Start(object sender, EventArgs e)
        {
            //set a working directory
            CClientSocket.QueueConfigure.WorkDirectory = "c:\\cyetest";

            CConnectionContext[,] ccs = LoadConnectionContexts();
            int threads = ccs.GetLength(0);
            int sockets_per_thread = ccs.GetLength(1);

            //start one instance of lookup with one socket pool connected to a number of remote SocketPro servers
            bool working = iCOS.Lookup.Initialize(  ccs,
                                                    2, //must not be larger than DATASIZE defined in icosdefines.h
                                                    5000);
        }

        protected void Session_Start(object sender, EventArgs e)
        {

        }

        protected void Application_BeginRequest(object sender, EventArgs e)
        {

        }

        protected void Application_AuthenticateRequest(object sender, EventArgs e)
        {

        }

        protected void Application_Error(object sender, EventArgs e)
        {

        }

        protected void Session_End(object sender, EventArgs e)
        {

        }

        protected void Application_End(object sender, EventArgs e)
        {
            //shutdown the single instance at last
            iCOS.Lookup.Shutdown();
        }
    }
}