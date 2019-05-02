using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using USOCKETLib;

namespace SocketProAdapter
{
	namespace ClientSide
	{
		public delegate void DOnClosing(int hSocket, int hWnd);
		public delegate void DOnBaseRequestProcessed(short sRequestID);
		public delegate void DOnConnecting(int hSocket, int hWnd);
		public delegate void DOnDataAvailable(int hSocket, int nBytes, int nError);
		public delegate void DOnGetHostByAddr(int hHandle, string strHostName, string strHostAlias, int nError);
		public delegate void DOnGetHostByName(int hHandle, string strHostName, string strHostAlias, string strIPAddr, int nError);
		public delegate void DOnOtherMessage(int hSocket, int nMsg, int wParam, int lParam);
		public delegate void DOnRequestProcessed(int hSocket, short sRequestID, int nLen, int nLenInBuffer, USOCKETLib.tagReturnFlag ReturnFlag);
		public delegate void DOnSendingData(int hSocket, int nError, int nSent);
		public delegate void DOnSocketClosed(int hSocket, int nError);
		public delegate void DOnSocketConnected(int hSocket, int nError);

        class CUPushClientImpl : IUPush
        {
            #region IUPush Members

            public bool Broadcast(object Message, int[] Groups)
            {
                try
                {
                    m_cs.GetUSocket().XSpeak(Message, Groups);
                }
                catch (Exception)
                {
                    return false;
                }
                return true;
            }

            public bool Broadcast(byte[] Message, int[] Groups)
            {
                if (Groups == null || Groups.Length == 0)
                    return false;
                uint []myGroups = new uint[Groups.Length];
                int nIndex = 0;
                foreach (int nGroup in Groups)
                {
                    myGroups[nIndex] = (uint)nGroup;
                    nIndex++;
                }
                try
                {
                    if (Message == null || Message.Length == 0)
                    {
                        byte b = 0;
                        m_cs.GetUSocket().XSpeakEx((uint)0, ref b, (uint)Groups.Length, ref myGroups[0]);
                    }
                    else
                        m_cs.GetUSocket().XSpeakEx((uint)Message.Length, ref Message[0], (uint)Groups.Length, ref myGroups[0]);
                }
                catch (Exception)
                {
                    return false;
                }
                return true;
            }

            public bool Enter(int[] Groups)
            {
                try
                {
                    if (Groups == null || Groups.Length == 0)
                        return Exit();
                    m_cs.GetUSocket().XEnter(Groups);
                }
                catch (Exception)
                {
                    return false;
                }
                return true;
            }

            public bool Exit()
            {
                try
                {
                    m_cs.GetUSocket().Exit();
                }
                catch (Exception)
                {
                    return false;
                }
                return true;
            }

            public bool SendUserMessage(object Message, string UserId)
            {
                try
                {
                    m_cs.GetUSocket().SendUserMessage(UserId, Message);
                }
                catch (Exception)
                {
                    return false;
                }
                return true;
            }

            public bool SendUserMessage(string UserId, byte[] Message)
            {
                try
                {
                    if (Message == null || Message.Length == 0)
                    {
                        byte b = 0;
                        m_cs.GetUSocket().SendUserMessageEx(UserId, (uint)0, ref b);
                    }
                    else
                        m_cs.GetUSocket().SendUserMessageEx(UserId, (uint)Message.Length, ref Message[0]);
                }
                catch (Exception)
                {
                    return false;
                }
                return true;
            }

            #endregion
            internal CClientSocket m_cs;
        }
		
		public class CClientSocket : IDisposable
		{
			public DOnBaseRequestProcessed m_OnBaseRequestProcessed;
			public DOnClosing m_OnClosing;
			public DOnConnecting m_OnConnecting;
			public DOnDataAvailable m_OnDataAvailable;
			public DOnGetHostByAddr m_OnGetHostByAddr;
			public DOnGetHostByName m_OnGetHostByName;
			public DOnOtherMessage m_OnOtherMessage;
			public DOnRequestProcessed m_OnRequestProcessed;
			public DOnSendingData m_OnSendingData;
			public DOnSocketClosed m_OnSocketClosed;
			public DOnSocketConnected m_OnSocketConnected;
			private bool m_bServerException;

            private void CreateObj()
            {
                string errMsg = "Failed in create COM object!";
                try
                {
                    m_USocket = new USocketClass();
                }
                catch (COMException ex)
                {
                    errMsg += (" Can not find usocket.dll on the device! Forget registering usocket.dll on your device? " + ex.Message);
                    throw new CClientSocketException(ex.ErrorCode, errMsg);
                }
            }

			public CClientSocket()
			{
                m_ulBatchBalance = 0;
                m_UPush = new CUPushClientImpl();
                m_UPush.m_cs = this;
                m_lstAsyncServiceHandler = new List<CAsyncServiceHandler>();
                CreateObj();
				if(m_USocket != null)
				{
					Advise();
				}
			}
			public CClientSocket(bool bCreate)
			{
                m_ulBatchBalance = 0;
                m_UPush = new CUPushClientImpl();
                m_UPush.m_cs = this;
                m_lstAsyncServiceHandler = new List<CAsyncServiceHandler>();
				if (bCreate)
				{
                    CreateObj();
					if(m_USocket != null)
					{
						Advise();
					}
				}
			}
			
			public CClientSocket(USocketClass USocket)
			{
                m_ulBatchBalance = 0;
                m_UPush = new CUPushClientImpl();
                m_UPush.m_cs = this;
                m_lstAsyncServiceHandler = new List<CAsyncServiceHandler>(); 
				m_USocket = USocket;
				if(m_USocket != null)
				{
					Advise();
				}
			}

			public void SetUSocket(USocketClass USocket)
			{
                if (m_ulBatchBalance != 0)
                    throw new InvalidOperationException("Make sure BeginBatching and Commit/Rollback are balanced before calling the method SetUSocket!");
				if(m_USocket != null)
				{
					m_USocket.Disconnect();
					DetachAll();
					Unadvise();
					m_USocket = null;
				}
				m_USocket = USocket;
				if(m_USocket != null)
				{
					Advise();
					int []pParams = (int[])m_USocket.ServerParams;
					m_bServerException = ((pParams[4] & CSocketProServerException.TRANSFER_SERVER_EXCEPTION) == CSocketProServerException.TRANSFER_SERVER_EXCEPTION);
				}
			}

			//Destroy a usocket explicitly
			public void DestroyUSocket()
			{
                if (m_USocket != null)
                {
                    m_USocket.Disconnect();
                    DetachAll();
                    Unadvise();
                    m_USocket = null;
                }
			}

			private void Advise()
			{
				m_USocket.OnClosing += new _IUSocketEvent_OnClosingEventHandler(OnClosing);
				m_USocket.OnConnecting += new _IUSocketEvent_OnConnectingEventHandler(OnConnecting);
				m_USocket.OnDataAvailable += new _IUSocketEvent_OnDataAvailableEventHandler(OnDataAvailable);
				m_USocket.OnGetHostByAddr += new _IUSocketEvent_OnGetHostByAddrEventHandler(OnGetHostByAddr);
				m_USocket.OnGetHostByName += new _IUSocketEvent_OnGetHostByNameEventHandler(OnGetHostByName);
				m_USocket.OnOtherMessage += new _IUSocketEvent_OnOtherMessageEventHandler(OnOtherMessage);
				m_USocket.OnRequestProcessed += new _IUSocketEvent_OnRequestProcessedEventHandler(OnRequestProcessed);
				m_USocket.OnSendingData += new _IUSocketEvent_OnSendingDataEventHandler(OnSendingData);
				m_USocket.OnSocketClosed += new _IUSocketEvent_OnSocketClosedEventHandler(OnSocketClosed);
				m_USocket.OnSocketConnected += new _IUSocketEvent_OnSocketConnectedEventHandler(OnSocketConnected);
                
                //This is a hack for .NET compact
                m_USocket.ConnTimeout = -2;
			}

			private void Unadvise()
			{
                //This is a hack for .NET compact
                m_USocket.ConnTimeout = -3;

				m_USocket.OnClosing -= new _IUSocketEvent_OnClosingEventHandler(OnClosing);
				m_USocket.OnConnecting -= new _IUSocketEvent_OnConnectingEventHandler(OnConnecting);
				m_USocket.OnDataAvailable -= new _IUSocketEvent_OnDataAvailableEventHandler(OnDataAvailable);
				m_USocket.OnGetHostByAddr -= new _IUSocketEvent_OnGetHostByAddrEventHandler(OnGetHostByAddr);
				m_USocket.OnGetHostByName -= new _IUSocketEvent_OnGetHostByNameEventHandler(OnGetHostByName);
				m_USocket.OnOtherMessage -= new _IUSocketEvent_OnOtherMessageEventHandler(OnOtherMessage);
				m_USocket.OnRequestProcessed -= new _IUSocketEvent_OnRequestProcessedEventHandler(OnRequestProcessed);
				m_USocket.OnSendingData -= new _IUSocketEvent_OnSendingDataEventHandler(OnSendingData);
				m_USocket.OnSocketClosed -= new _IUSocketEvent_OnSocketClosedEventHandler(OnSocketClosed);
				m_USocket.OnSocketConnected -= new _IUSocketEvent_OnSocketConnectedEventHandler(OnSocketConnected);
			}

			public void DetachAll()
			{
				int n;
				CAsyncServiceHandler RAB;
				lock(m_lstAsyncServiceHandler)
				{
					int nSize = m_lstAsyncServiceHandler.Count;
					for(n=0; n<nSize; n++)
					{
						RAB = (CAsyncServiceHandler)m_lstAsyncServiceHandler[n];
						if(RAB != null)
						{
							RAB.Detach();
						}
					}
					m_lstAsyncServiceHandler.Clear();
				}
			}
			
			public void CleanTrack()
			{
				if(m_USocket != null)
				{
					m_USocket.CleanTrack();
				}
			}

            public bool EndJob()
            {
                if (m_USocket != null)
                {
                    try
                    {
                        m_USocket.EndJob();
                    }
                    catch (COMException)
                    {
                        return false;
                    }
                    return true;
                }
                return false;
            }

            public bool StartJob()
            {
                if (m_USocket != null)
                {
                    try
                    {
                        m_USocket.StartJob();
                    }
                    catch(COMException)
                    {
                        return false;
                    }
                    return true;
                }
                return false;
            }

			//close socket connection gracefully
			public void Shutdown()
			{
				if(m_USocket != null)
				{
					m_USocket.Shutdown((int)tagHowtoShutdown.hsSend);
				}
			}
			//abort a socket connection
			public void Disconnect()
			{
				if(m_USocket != null)
				{
					m_USocket.Disconnect();
				}
			}

			public void SetUID(string strUID)
			{
				if(m_USocket != null)
				{
					m_USocket.UserID = strUID;
				}
			}

			public void SetPassword(string strPassword)
			{
				if(m_USocket != null)
				{
					m_USocket.Password = strPassword;
				}
			}
			
			public bool IsConnected()
			{
				if(m_USocket != null)
				{
					int hSocket = m_USocket.Socket;
					if(hSocket != 0 && hSocket != -1)
						return true;
				}
				return false;
			}

			public int GetErrorCode()
			{
				if(m_USocket != null)
				{
					return m_USocket.Rtn;
				}
				return 0;
			}

			public string GetErrorMsg()
			{
				if(m_USocket != null)
				{
					return m_USocket.ErrorMsg;
				}
				return null;
			}
			
			//returns the count of requests to be processed in queue
			public int GetCountOfRequestsInQueue()
			{
				if(m_USocket != null)
				{
					return m_USocket.CountOfRequestsInQueue;
				}
				return 0;
			}
			
			public bool IsBatching()
			{
				if(m_USocket != null)
				{
					return m_USocket.IsBatching;
				}
				return false;
			}
			
			//check the size of batched requests in byte
			public int GetBytesBatched()
			{
				if(m_USocket != null)
				{
					return m_USocket.BytesBatched;
				}
				return 0;
			}

			public bool BeginBatching()
			{
				m_ulBatchBalance++;
				if(m_ulBatchBalance > 1)
					return true;
				if(m_USocket != null)
				{
					try
					{
						m_USocket.StartBatching();
					}
					catch(COMException)
					{
						return false;
					}
					return true;
				}
				return false;
			}
			
			//delete all of batched requests
			public bool Rollback()
			{
                m_ulBatchBalance--;
                if (m_ulBatchBalance > 0)
                    return false;
				if(m_USocket != null)
				{
					try
					{
						m_USocket.AbortBatching();
					}
					catch(COMException)
					{
						return false;
					}
					return true;
				}
				return false;
			}
			
			//send a batch of requests onto a remote SocketPro server
			public bool Commit(bool bBatchingAtServer)
			{
                m_ulBatchBalance--;
                if (m_ulBatchBalance > 0)
                {
                    //we return here because the methods BeginBatching and Commit/Rollback not balanced yet
                    return true;
                }
				if(m_USocket != null)
				{
					try
					{
						m_USocket.CommitBatching(bBatchingAtServer);
					}
					catch(COMException)
					{
						return false;
					}
					return true;
				}
				return false;
			}

			public bool Commit()
			{
				return Commit(false);
			}
			
			public virtual bool SwitchTo(int nSvsID,  bool bAutoTransferServerException)
			{
				if(!IsConnected())
					return false;

				int []pParam = (int[])m_USocket.ClientParams;
				int []pNewParam = new int[3];
				if(bAutoTransferServerException)
				{
					pNewParam[2] = (pParam[4] | CSocketProServerException.TRANSFER_SERVER_EXCEPTION);
				}
				else
				{
					pNewParam[2] = (pParam[4] & (~CSocketProServerException.TRANSFER_SERVER_EXCEPTION));
				}
				pNewParam[0] = pParam[2];
				pNewParam[1] = pParam[3];
				m_USocket.ClientParams = pNewParam;

				m_USocket.SwitchTo(nSvsID);
				
				//for the sake of better security, clean a password ASAP
				m_USocket.Password = "";
				return true;
			}
			//ask for a service
			public virtual bool SwitchTo(int nSvsID)
			{
				return SwitchTo(nSvsID,  false);
			}

			public bool SwitchTo(CAsyncServiceHandler rahb)
			{
				if(rahb == null)
					return false;
				return SwitchTo(rahb.GetSvsID(), false);
			}
			
			public bool SwitchTo(CAsyncServiceHandler arh,  bool bAutoTransferServerException)
			{
				if(arh == null)
					return false;
				return SwitchTo(arh.GetSvsID(), bAutoTransferServerException);
			}

			public void DisableUI()
			{
				DisableUI(true);
			}

			public void DisableUI(bool bDisable)
			{
				if(m_USocket != null)
				{
					m_USocket.Frozen = bDisable;
				}
			}

			public bool Connect(string strHost, int nPort)
			{
				return Connect(strHost, nPort, false);
			}

			public bool Connect(string strHost, int nPort, bool bSyn)
			{
				if(strHost == null || strHost.Length == 0)
					return false;

				if(nPort == 0 || m_USocket == null)
					return false;

				try
				{
					m_USocket.Connect(strHost, nPort, bSyn, 0, 0, 0, 0);
				}
				catch(COMException)
				{
					return false;
				}
				return true;
			}
			
			public USocketClass GetUSocket()
			{
				return m_USocket;
			}

			public int GetCurrentServiceID()
			{
				if(m_USocket != null)
					return m_USocket.CurrentSvsID;
				return 0;
			}
			
			public int GetCountOfAttachedServiceHandlers()
			{
				lock(m_lstAsyncServiceHandler)
				{
					return m_lstAsyncServiceHandler.Count;
				}
			}

			protected List<CAsyncServiceHandler> GetRequestAsynHandlers()
			{
				lock(m_lstAsyncServiceHandler)
				{
					return m_lstAsyncServiceHandler;
				}
			}
			
			//-1 = infinite
			public bool WaitAll(int nTimeout)
			{
				if(m_USocket == null)
					return false;
                if (m_ulBatchBalance != 0)
                    throw new InvalidOperationException("Call the method WaitAll only after methods BeginBatching and Rollback/Commit are balanced!");
				bool bSuc = ((!m_USocket.WaitAll(nTimeout)) && IsConnected());
				return bSuc;
			}

			public bool WaitAll()
			{
				return WaitAll(-1); //infinte
			}
			
			//-1 = infinite
			//0 = current service id
			public bool Wait(short sRequestID, int nTimeout, int nSvsID)
			{
				if(!IsConnected())
					return true;
                if(m_ulBatchBalance != 0)
                    throw new InvalidOperationException("Call the method Wait only after methods BeginBatching and Rollback/Commit are balanced!");
				if (nSvsID == 0)
				{
					nSvsID = GetCurrentServiceID();
				}
				try
				{
					return ((!m_USocket.Wait(sRequestID, nTimeout, nSvsID)) && IsConnected());
				}
				catch(COMException)
				{
				}
				return false;
			}
			
			public bool Wait(short sRequestID, int nTimeout)
			{
				return Wait(sRequestID, nTimeout, 0);
			}

			public bool Wait(short sRequestID)
			{
				return Wait(sRequestID, -1);
			}
			
			public void Cancel()
			{
				Cancel(-1);
			}

			public void Cancel(int nRequests)
			{
				if(m_USocket != null)
				{
					m_USocket.Cancel(nRequests);
				}
			}

			private void OnDataAvailable(int hSocket, int nBytes, int nError)
			{
				if(m_OnDataAvailable != null)
				{
					m_OnDataAvailable.Invoke(hSocket, nBytes, nError);
				}
			}

			private void OnOtherMessage(int hSocket, int nMsg, int wParam, int nParam)
			{
				if(m_OnOtherMessage != null)
				{
					m_OnOtherMessage.Invoke(hSocket, nMsg, wParam, nParam);
				}
			}

			private void OnSocketClosed(int hSocket, int nError)
			{
				if(m_OnSocketClosed != null)
				{
					m_OnSocketClosed.Invoke(hSocket, nError);
				}

                lock (m_lstAsyncServiceHandler)
                {
                    foreach (CAsyncServiceHandler ash in m_lstAsyncServiceHandler)
                    {
                        ash.RemovePairs(0x7FFFFFFF);
                    }
                }
			}

			private void OnSocketConnected(int hSocket, int nError)
			{
				if(m_OnSocketConnected != null)
				{
					m_OnSocketConnected.Invoke(hSocket, nError);
				}
			}

			private void OnConnecting(int hSocket, int hWnd)
			{
                lock (m_lstAsyncServiceHandler)
                {
                    foreach (CAsyncServiceHandler ash in m_lstAsyncServiceHandler)
                    {
                        ash.RemovePairs(0x7FFFFFFF);
                    }
                }
                m_ulBatchBalance = 0;
				if(m_OnConnecting != null)
				{
					m_OnConnecting.Invoke(hSocket, hWnd);
				}
			}

			private void OnSendingData(int hSocket, int nError, int nSent)
			{
				if(m_OnSendingData != null)
				{
					m_OnSendingData.Invoke(hSocket, nError, nSent);
				}
			}

			private void OnGetHostByAddr(int nHandle, string bstrHostName, string bstrHostAlias, int nError)
			{
				if(m_OnGetHostByAddr != null)
				{
					m_OnGetHostByAddr.Invoke(nHandle, bstrHostName, bstrHostAlias, nError);
				}
			}

			private void OnGetHostByName(int nHandle, string bstrHostName, string bstrAlias, string bstrIPAddr, int nError)
			{
				if(m_OnGetHostByName != null)
				{
					m_OnGetHostByName.Invoke(nHandle, bstrHostName, bstrAlias, bstrIPAddr, nError);
				}
			}

			private void OnClosing(int hSocket, int hWnd)
			{
				if(m_OnClosing != null)
				{
					m_OnClosing.Invoke(hSocket, hWnd);
				}
			}

			private CAsyncServiceHandler FindRequestAsynHandler()
			{
				int n;
				CAsyncServiceHandler p;
				int nCurSvsID = GetCurrentServiceID();
				lock(m_lstAsyncServiceHandler)
				{
					int nSize = m_lstAsyncServiceHandler.Count;
					for(n=0; n<nSize; n++)
					{
						p = (CAsyncServiceHandler)m_lstAsyncServiceHandler[n];
						if(p != null && p.GetSvsID() == nCurSvsID)
							return p;
					}
				}
				return null;
			}

			private void OnRequestProcessed(int hSocket, short sRequestID, int nLen, int nLenInBuffer, short sFlag)
			{
				if(sFlag != (short)USOCKETLib.tagReturnFlag.rfCompleted)
					return;
				if(sRequestID == (short)USOCKETLib.tagBaseRequestID.idSwitchTo)
				{
					int []pParams = (int[])m_USocket.ServerParams;
					m_bServerException = ((pParams[4] & CSocketProServerException.TRANSFER_SERVER_EXCEPTION) == CSocketProServerException.TRANSFER_SERVER_EXCEPTION);
				}
				CAsyncServiceHandler p = FindRequestAsynHandler();
				if(sRequestID >= 0 && (ushort)sRequestID < 46) //idClose for window remote file
				{
					OnBaseRequestProcessed(sRequestID);
					if(m_OnRequestProcessed != null)
					{
						m_OnRequestProcessed.Invoke(hSocket, sRequestID, 0, 0, (USOCKETLib.tagReturnFlag)sFlag);
					}
					return;
				}
				if(p != null)
                    p.OnRR(hSocket, sRequestID, nLen, sFlag);
				else
				{
					if(m_OnRequestProcessed != null)
					{
						m_OnRequestProcessed.Invoke(hSocket, sRequestID, nLen, nLenInBuffer, (USOCKETLib.tagReturnFlag)sFlag);
					}
				}
			}

			private void OnBaseRequestProcessed(short sRequestID)
			{
				if(m_OnBaseRequestProcessed != null)
				{
					m_OnBaseRequestProcessed.Invoke(sRequestID);
				}
			}

            public bool AutoTransferServerException
            {
                get
                {
                    return m_bServerException;
                }
            }

			public long BytesReceived 
			{ 
				get
				{
					long lData = 0;
					if(m_USocket != null)
					{
						int nHigh;
						int nLow = m_USocket.GetBytesReceived(out nHigh);
						lData = nHigh;
						lData = (uint)nLow + (lData << 32);
				
					}
					return lData;
				}
			}
			public long BytesSent
			{ 
				get
				{
					long lData = 0;
					if(m_USocket != null)
					{
						int nHigh;
						int nLow = m_USocket.GetBytesSent(out nHigh);
						lData = nHigh;
						lData = (uint)nLow + (lData << 32);
				
					}
					return lData;
				}
			}

			public USOCKETLib.tagEncryptionMethod EncryptionMethod
			{
				get
				{
					if(m_USocket == null)
						return USOCKETLib.tagEncryptionMethod.NoEncryption;
					short nMethod = m_USocket.EncryptionMethod;
					return (USOCKETLib.tagEncryptionMethod)nMethod;
				}
				set
				{
					if(m_USocket != null)
					{
						m_USocket.EncryptionMethod = (short)value;
					}
				}
			}

			public bool Syn
			{
				get
				{
					if(m_USocket != null)
					{
						return m_USocket.Syn;
					}
					return false;
				}
				set
				{
					if(m_USocket != null)
					{
						m_USocket.Syn = value;
					}
				}
			}
			
			public int Socket
			{
				get
				{
					if(m_USocket != null)
						return m_USocket.Socket;
					return 0;
				}
			}
            public bool ReturnRandom 
            {
                get
                {
                    if (m_USocket == null)
                        return false;
                    short sMinor = 0;
                    short sMajor = m_USocket.GetClientUSockVersion(out sMinor);
                    return ((sMinor & 0x4000) == 0x4000);
                }
            }

            public IUPush Push
            {
                get
                {
                    return m_UPush;
                }
            }

			internal List<CAsyncServiceHandler>		m_lstAsyncServiceHandler;
			internal USocketClass	m_USocket;
			//internal CUQueue		m_UQueue;
            private CUPushClientImpl m_UPush;
            private uint m_ulBatchBalance;
            #region IDisposable Members

            public void Dispose()
            {
                DestroyUSocket();
            }

            #endregion
        }
	} //ClientSide
} //SocketProAdapter
