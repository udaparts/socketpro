using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;


namespace Suntico
{
    namespace Server
    {
        class CPushPeer : SocketProAdapter.ServerSide.CClientPeer
        {
            protected override void OnFastRequestArrive(short sRequestID, int nLen)
            {

            }

            protected override int OnSlowRequestArrive(short sRequestID, int nLen)
            {
                return 0;
            }
        }
    }
}
