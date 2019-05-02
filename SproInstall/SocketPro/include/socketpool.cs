using System;
using System.Runtime.InteropServices;
using System.Collections;
using System.Collections.Generic;
using USOCKETLib;

namespace SocketProAdapter.ClientSide
{
    public class CSocketPool<THandler> : IDisposable
        where THandler : CRequestAsynHandlerBase, new()
    {
        ~CSocketPool()
        {
            Dispose(false);
        }

        public void Dispose()
        {
            Dispose(true);
        }

        protected virtual void Dispose(bool disposeManagedResources)
        {
            if (disposeManagedResources)
            {
                ShutdownPool();
            }
        }

        public bool StartSocketPool(string strHost, int lPort, string strUID, string strPassword, byte bSocketsPerThread)
        {
            return StartSocketPool(strHost, lPort, strUID, strPassword, bSocketsPerThread, 0, tagEncryptionMethod.NoEncryption, false);
        }

        public bool StartSocketPool(string strHost, int lPort, string strUID, string strPassword, byte bSocketsPerThread, byte bThreads, USOCKETLib.tagEncryptionMethod EncrytionMethod, bool bZip)
        {
            if (System.Threading.Thread.CurrentThread.GetApartmentState() == System.Threading.ApartmentState.MTA)
                throw new Exception("COM MTA required with socket pool!");
            if (IsStarted())
            {
                ShutdownPool();
            }
            m_USocketPool = new USocketPoolClass();
            m_USocketPool.StartPool(bThreads, bSocketsPerThread);
            THandler temp = new THandler();
            try
            {
                m_USocketPool.ConnectAll(strHost, lPort, temp.GetSvsID(), strUID, strPassword, (short)EncrytionMethod, bZip);
                if (m_USocketPool.ConnectedSockets == 0)
                {
                    ShutdownPool();
                    throw new COMException("No socket connection available!", (int)USOCKETLib.tagSocketPoolErrorCode.specNoOpenedSocket);
                }
            }
            catch (COMException myerr)
            {
                ShutdownPool();
                throw myerr;
            }
            COMException err = null;
            USocket pIUSocekt = null;
            m_cs.Lock();
            try
            {
                pIUSocekt = m_USocketPool.LockASocket(10, null);
            }
            catch (COMException cError)
            {
                err = cError;
            }
            while (pIUSocekt != null)
            {
                IntPtr p = System.Runtime.InteropServices.Marshal.GetIUnknownForObject(pIUSocekt);
                CClientSocket ClientSocket = new CClientSocket(p);
                if (p != IntPtr.Zero)
                {
                    System.Runtime.InteropServices.Marshal.Release(p);
                }
                THandler Handler = new THandler();
                m_mapSocket.Add(ClientSocket, Handler);
                Handler.Attach(ClientSocket);
                try
                {
                    pIUSocekt = null;
                    pIUSocekt = m_USocketPool.LockASocket(10, null);
                }
                catch (COMException cError)
                {
                    err = cError;
                }
            }
            foreach (CClientSocket cs in m_mapSocket.Keys)
            {
                m_USocketPool.UnlockASocket(cs.GetUSocket());
            }
            m_cs.Unlock();
            return true;
        }

        public bool IsStarted()
        {
            return (m_USocketPool != null);
        }

        public USocketPoolClass GetUSocketPool()
        {
            return m_USocketPool;
        }

        public void Unlock(THandler Handler)
        {
            if (Handler != null)
            {
                Unlock(Handler.GetAttachedClientSocket());
            }
        }

        public void Unlock(CClientSocket ClientSocket)
        {
            if (ClientSocket != null && m_USocketPool != null)
            {
                m_USocketPool.UnlockASocket(ClientSocket.GetUSocket());
            }
        }

        public THandler Lock(uint nTimeout)
        {
            return Lock(nTimeout, (USocketClass)null);
        }

        public THandler Lock(uint nTimeout, CClientSocket USocketSameThread)
        {
            if (USocketSameThread == null)
                return Lock(nTimeout, (USocketClass)null);
            return Lock(nTimeout, USocketSameThread.GetUSocket());
        }

        public THandler Lock(uint nTimeout, USocketClass USocketSameThread)
        {
            if (m_USocketPool == null)
                return null;
            THandler Hander = null;
            USocket pIUSocekt = null;
            try
            {
                pIUSocekt = m_USocketPool.LockASocket((int)nTimeout, USocketSameThread);
            }
            catch (COMException err)
            {
                string strErrorMessage = err.Message;
                return null;
            }
            m_cs.Lock();
            foreach (CClientSocket cs in m_mapSocket.Keys)
            {
                if (cs.GetUSocket().Socket == pIUSocekt.Socket)
                {
                    Hander = (THandler)m_mapSocket[cs];
                    break;
                }
            }
            m_cs.Unlock();
            return Hander;
        }

        public bool IsCompleted(THandler Handler)
        {
            if (Handler == null)
                return true;
            return IsCompleted(Handler.GetAttachedClientSocket());
        }

        public bool IsCompleted(CClientSocket ClientSocket)
        {
            if (ClientSocket != null)
            {
                return (ClientSocket.GetCountOfRequestsInQueue() == 0);
            }
            return true;
        }

        public void ShutdownPool()
        {
            CleanHandlers();
            if (m_USocketPool != null)
            {
                try
                {
                    m_USocketPool.DisconnectAll();
                }
                catch (COMException err)
                {
                    string strErrorMessage = err.Message;
                    strErrorMessage = null;
                }
                try
                {
                    m_USocketPool.ShutdownPool();
                }
                catch (COMException err)
                {
                    string strErrorMessage = err.Message;
                    strErrorMessage = null;
                }
                m_USocketPool = null;
            }
        }
        private void CleanHandlers()
		{
            m_cs.Lock();
            m_mapSocket.Clear();
            m_cs.Unlock();
		}
        private USocketPoolClass m_USocketPool = null;
        private CCriticalSection m_cs = new CCriticalSection();
        private Hashtable m_mapSocket = new Hashtable();
    }
}
