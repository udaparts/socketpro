

using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using USOCKETLib;
using System.Collections.Generic;

namespace Suntico
{
    namespace Client
    {
        class CSunticoAsyncHandler : SocketProAdapter.ClientSide.CAsyncAdohandler
        {
            public CSunticoAsyncHandler()
                : base(Const.sidSunticoComm)
            {
                m_lstAsyncHandlers.Add(this);
            }

            public CSunticoAsyncHandler(CClientSocket cs)
                : base(Const.sidSunticoComm, cs)
            {
                m_lstAsyncHandlers.Add(this);
            }

            public CSunticoAsyncHandler(CClientSocket cs, IAsyncResultsHandler DefaultAsyncResultsHandler)
                : base(Const.sidSunticoComm, cs, DefaultAsyncResultsHandler)
            {
                m_lstAsyncHandlers.Add(this);
            }

            protected override void Dispose(bool value)
            {
                m_lstAsyncHandlers.Remove(this);
                base.Dispose(value);
            }

            internal CClientPoint m_CustomerPoint;
            internal static List<CSunticoAsyncHandler> m_lstAsyncHandlers;
            internal void SetChat()
            {
                GetAttachedClientSocket().m_OnBaseRequestProcessed += delegate(short sRequestID)
                {
                    switch (sRequestID)
                    {
                        case (short)USOCKETLib.tagChatRequestID.idEnter:
                        case (short)USOCKETLib.tagChatRequestID.idXEnter:
                            //{
                            //    int nGroup;
                            //    int nPort;
                            //    int nSvsID;
                            //    string strUID;
                            //    USOCKETLib.USocketClass socket = GetAttachedClientSocket().GetUSocket();
                            //    string strIPAddr = socket.GetInfo(0, out nGroup, out strUID, out nSvsID, out nPort);
                            //    strIPAddr = null;
                            //}
                            break;
                        case (short)USOCKETLib.tagChatRequestID.idExit:
                            break;
                        case (short)USOCKETLib.tagChatRequestID.idSpeak:
                        case (short)USOCKETLib.tagChatRequestID.idXSpeak:
                            {
                                int nGroup;
                                int nPort;
                                int nSvsID;
                                string strUID;
                                USOCKETLib.USocketClass socket = GetAttachedClientSocket().GetUSocket();
                                string strMsg = (string)socket.Message;
                                string strIPAddr = socket.GetInfo(0, out nGroup, out strUID, out nSvsID, out nPort);
                                m_CustomerPoint.m_msgCloud.OnCloudGeneralMessage(strMsg, nGroup, nSvsID);
                            }
                            break;
                        default:
                            break;
                    }
                };
            }

            private DRequestCompletedAtCloud m_pDataReader = null;
            internal bool Send(DRequestCompletedAtCloud p, System.Data.IDataReader dr)
            {
                m_pDataReader = p;
                if (dr != null)
                    return Send(dr);
                return true;
            }

            private DRequestCompletedAtCloud m_pDataTable = null;
            internal bool Send(DRequestCompletedAtCloud p, System.Data.DataTable dt)
            {
                m_pDataTable = p;
                if (dt != null)
                    return Send(dt);
                return true;
            }

            private DRequestCompletedAtCloud m_pDataSet = null;
            internal bool Send(DRequestCompletedAtCloud p, System.Data.DataSet ds)
            {
                m_pDataSet = p;
                if (ds != null)
                    return Send(ds);
                return true;
            }

            internal void Send(string str, StringObjectType sot, DRequestCompletedAtCloud rp)
            {
                bool b = SendRequest(Const.idClientSendString, str, (int)sot, delegate(CAsyncResult ar)
                    {
                        if (rp != null)
                            rp.Invoke();
                    }
               );
            }

            protected override void OnResultReturned(short sRequestID, CUQueue UQueue)
            {
                int len = UQueue.GetSize();
                switch (sRequestID)
                {
                    case Const.idClientConfirmation:
                        break;
                    case Const.idCloudStartTrans:
                        {
                            long Clue;
                            UQueue.Pop(out Clue);
                            CCloudMessage cm = (CCloudMessage)m_CustomerPoint.CloudMessage;
                            cm.OnCloudBeginTrans(Clue);
                        }
                        break;
                    case Const.idCloudEndTrans:
                        {
                            long Clue;
                            UQueue.Pop(out Clue);
                            CCloudMessage cm = (CCloudMessage)m_CustomerPoint.CloudMessage;
                            long res = cm.OnCloudEndTrans(Clue);

                            //reply a confirm message so that Suntico server knows that a client has processed objects safely
                            SendRequest(Const.idClientConfirmation, res);
                        }
                        break;
                    case CAsyncAdoSerializationHelper.idEndDataTable:
                        base.OnResultReturned(sRequestID, UQueue);
                        if (len > 0)
                        {
                            CCloudMessage cm = (CCloudMessage)m_CustomerPoint.CloudMessage;
                            cm.OnCloudDataTable(m_AdoSerialier.CurrentDataTable);
                        }
                        else if (m_pDataTable != null)
                            m_pDataTable.Invoke();
                        break;
                    case CAsyncAdoSerializationHelper.idEndDataReader:
                        base.OnResultReturned(sRequestID, UQueue);
                        if (len > 0)
                        {
                            CCloudMessage cm = (CCloudMessage)m_CustomerPoint.CloudMessage;
                            cm.OnCloudDataReader(m_AdoSerialier.CurrentDataTable);
                        }
                        else if (m_pDataReader != null)
                            m_pDataReader.Invoke();
                        break;
                    case CAsyncAdoSerializationHelper.idEndDataSet:
                        base.OnResultReturned(sRequestID, UQueue);
                        if (len > 0)
                        {
                            CCloudMessage cm = (CCloudMessage)m_CustomerPoint.CloudMessage;
                            cm.OnCloudDataSet(m_AdoSerialier.CurrentDataSet);
                        }
                        else if (m_pDataSet != null)
                            m_pDataSet.Invoke();
                        break;
                    case Const.idCloudSendObject:
                        {
                            long Clue;
                            UQueue.Pop(out Clue);
                            CCloudMessage cm = (CCloudMessage)m_CustomerPoint.CloudMessage;
                            cm.OnCloudObjectMessage(Clue, UQueue);
                        }
                        break;
                    case Const.idCloudSendString:
                        {
                            int objectType;
                            string str;
                            UQueue.Pop(out objectType);
                            UQueue.Load(out str);
                            CCloudMessage cm = (CCloudMessage)m_CustomerPoint.CloudMessage;
                            cm.OnCloudStringObject((StringObjectType)objectType, str);
                        }
                        break;
                    default:
                        base.OnResultReturned(sRequestID, UQueue);
                        break;
                }
            }
        }
    }
}
