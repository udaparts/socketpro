using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;

namespace Suntico
{
    namespace Client
    {

        /// <summary>
        /// Interface for tansaction from Suntico cloud server to customer.
        /// Note that all events are always raised from SocketPro socket pool threads.
        /// </summary>
        public interface ICloudMessage
        {
            CClientPoint ClientPoint { get; }

            /// <summary>
            /// Event for tracking a starting point of a transaction from Suntico cloud server
            /// </summary>
            event DStartTrans OnStartTrans;

            /// <summary>
            /// Event for tracking an end point of a transaction from Suntico cloud server. 
            /// </summary>
            event DEndTrans OnEndTrans;

            /// <summary>
            /// Event for tracking an object message originated from Suntico cloud server. 
            /// </summary>
            event DGenericObject OnGenericObject;

            /// <summary>
            /// A general message like system shutdown maintainence, news, and others from Suntico cloud server
            /// </summary>
            event DGeneralMessage OnGeneralMessage;

            event DDataReader OnDataReader;

            event DDataSet OnDataSet;

            event DDataTable OnDataTable;

            event DStringObject OnStringObject;
        }

        class CCloudMessage : ICloudMessage
        {
            private CClientPoint m_ClientPoint;
            private long m_lConfirmationNumber = 0;

            public CCloudMessage(CClientPoint p)
            {
                m_ClientPoint = p;
            }

            public long OnCloudEndTrans(long Clue)
            {
                if (OnEndTrans != null)
                    OnEndTrans.Invoke(Clue);
                ++m_lConfirmationNumber;
                return m_lConfirmationNumber;
            }

            public void OnCloudBeginTrans(long Clue)
            {
                if (OnStartTrans != null)
                    OnStartTrans.Invoke(Clue);
            }

            public void OnCloudGeneralMessage(string msg, int Group, int ServiceId)
            {
                if (OnGeneralMessage != null)
                    OnGeneralMessage.Invoke(msg, Group, ServiceId);
            }

            public void OnCloudObjectMessage(long Clue, SocketProAdapter.CUQueue Queue)
            {
                if (OnGenericObject != null)
                    OnGenericObject.Invoke(Clue, Queue);
            }

            public void OnCloudDataSet(DataSet ds)
            {
                if (OnDataSet != null)
                    OnDataSet.Invoke(ds);
            }

            public void OnCloudDataTable(DataTable dt)
            {
                if (OnDataTable != null)
                    OnDataTable.Invoke(dt);
            }

            public void OnCloudDataReader(DataTable dt)
            {
                if (OnDataReader != null)
                    OnDataReader.Invoke(dt);
            }

            public void OnCloudStringObject(StringObjectType sot, string str)
            {
                if (OnStringObject != null)
                    OnStringObject.Invoke(sot, str);
            }

            #region ICloudMessage Members

            public CClientPoint ClientPoint { get { return m_ClientPoint; } }

            public event DStartTrans OnStartTrans;

            public event DEndTrans OnEndTrans;

            public event DGenericObject OnGenericObject;

            public event DGeneralMessage OnGeneralMessage;

            public event DDataReader OnDataReader;

            public event DDataSet OnDataSet;

            public event DDataTable OnDataTable;

            public event DStringObject OnStringObject;

            #endregion
        }
    }
}
