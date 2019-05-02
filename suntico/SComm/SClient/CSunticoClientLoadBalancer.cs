using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Suntico
{
    namespace Client
    {
        class CSunticoClientLoadBalancer : SocketProAdapter.ClientSide.CSocketPoolEx<CSunticoAsyncHandler>
        {
            protected override void OnAllSocketsDisconnected()
            {
                m_ClientPoint.OnAllSocketsDisconnected();
                base.OnAllSocketsDisconnected();
            }

            protected override bool OnFailover(CSunticoAsyncHandler pHandler, SocketProAdapter.IJobContext JobContext)
            {
                return m_ClientPoint.OnFailover(JobContext);
            }

            protected override void OnJobDone(CSunticoAsyncHandler Handler, SocketProAdapter.IJobContext JobContext)
            {
                m_Client.OnJobDone(JobContext);
            }

            internal CClientMessage m_Client;
            internal CClientPoint m_ClientPoint;
        }
    }
}
