

using System;
using System.Threading.Tasks;
using System.Collections.Generic;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

namespace iCOS
{
    public struct CRCode
    {
        public string CountryCode;
        public string RegionCode;
    }

    struct MyStruct
    {
        internal uint Ip;
        internal TaskCompletionSource<uint> Tcs;
    }

    public sealed class CGeoIpAsyncHandler : CAsyncServiceHandler
    {
        public CGeoIpAsyncHandler()
            : base(IpLookupConst.sidGeoIp)
        {
        }

        internal Dictionary<ulong, MyStruct> m_dicIdTaskProcessing = new Dictionary<ulong, MyStruct>(2 * 1024);
    }
}
