using System;
using System.Data;
using System.Configuration;
using System.Collections;
using System.Web;
using System.Web.Security;
using System.Web.SessionState;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using USOCKETLib;
using SocketProAdapter.ClientSide.RemoteDB;


namespace AdoWebAsync
{
    public class Global : System.Web.HttpApplication
    {
        //the pool for this sample AsyncADO service
        public static CSocketPool<CMyAdoHandler> m_AsyncADO;

        //the pool for UDAParts generic remote database service using MS OLEDB technology
        public static CSocketPool<CAsynDBLite> m_AsyncRdb;

        [MTAThread]
        protected void Application_Start(object sender, EventArgs e)
        {
            //your own ADO.NET based service
            m_AsyncADO = new CSocketPool<CMyAdoHandler>();

            //UDAParts remote database service using OLEDB technology
            m_AsyncRdb = new CSocketPool<CAsynDBLite>();

            int n;
            const int Count = 5;
            CConnectionContext[] pConnectionContext = new CConnectionContext[Count];
            for (n = 0; n < Count; n++)
                pConnectionContext[n] = new CConnectionContext();

            //set connection contexts
            pConnectionContext[0].m_strHost = "127.0.0.1";
            pConnectionContext[1].m_strHost = "localhost";
            pConnectionContext[2].m_strHost = "localhost";
            pConnectionContext[3].m_strHost = "127.0.0.1";
            pConnectionContext[4].m_strHost = "localhost";
            for (n = 0; n < Count; n++)
            {
                pConnectionContext[n].m_nPort = 20905;
                pConnectionContext[n].m_strPassword = "SocketPro";
                pConnectionContext[n].m_strUID = "PassOne";
                pConnectionContext[n].m_EncrytionMethod = USOCKETLib.tagEncryptionMethod.NoEncryption;
                pConnectionContext[n].m_bZip = false;
            }

            //error treatment ignored here
            bool ok = m_AsyncRdb.StartSocketPool(pConnectionContext, 2, 4);
            ok = m_AsyncADO.StartSocketPool(pConnectionContext, 1, 2);
        }

        protected void Application_End(object sender, EventArgs e)
        {
            if (m_AsyncRdb != null)
            {
                m_AsyncRdb.ShutdownPool();
                m_AsyncRdb.Dispose();
            }

            if (m_AsyncADO != null)
            {
                m_AsyncADO.ShutdownPool();
                m_AsyncADO.Dispose();
            }
        }
    }
}