using System;
using System.Data;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Security;
using System.Web.SessionState;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.UDB;

namespace SharedPools
{
    public class Global : System.Web.HttpApplication
    {
        private static CSocketPool<CMysql> m_spUpdate = new CSocketPool<CMysql>();
        private static CSocketPool<CMysql> m_spRead = new CSocketPool<CMysql>();
        private static DataSet m_dsCache = new DataSet();

        protected void Application_Start(object sender, EventArgs e)
        {

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
            m_spRead.ShutdownPool();
            m_spUpdate.ShutdownPool();
        }
    }
}