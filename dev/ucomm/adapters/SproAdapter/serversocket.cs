using System;

namespace SocketProAdapter
{
    namespace ServerSide
    {
        [AttributeUsage(AttributeTargets.Field | AttributeTargets.Class)]
        public class ServiceAttr : System.Attribute
        {
            public readonly uint ServiceID;
            public ServiceAttr(uint svsId)
            {
                ServiceID = svsId;
                ThreadApartment = tagThreadApartment.taNone;
            }
            public ServiceAttr(uint svsId, tagThreadApartment ta)
            {
                ServiceID = svsId;
                ThreadApartment = ta;
            }
            public readonly tagThreadApartment ThreadApartment = tagThreadApartment.taNone;
        }

        [AttributeUsage(AttributeTargets.Method)]
        public class RequestAttr : System.Attribute
        {
            public readonly ushort RequestID;
            public readonly bool SlowRequest = false;
            public RequestAttr(ushort reqId)
            {
                RequestID = reqId;
                SlowRequest = false;
            }
            public RequestAttr(ushort reqId, bool slowRequest)
            {
                RequestID = reqId;
                SlowRequest = slowRequest;
            }
        }
    }
}
