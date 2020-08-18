using System;

namespace SocketProAdapter
{
    namespace ClientSide
    {
        public class ErrInfo
        {
            public ErrInfo(int res, string errMsg)
            {
                ec = res;
                em = (null == errMsg) ? "" : errMsg;
            }
            public int ec = 0;
            public string em = "";

            public override string ToString()
            {
                string s = "ec: " + ec.ToString() + ", em: " + ((null == em) ? "" : em);
                return s;
            }
        };

        public class CSocketError : Exception
        {
            private ushort m_reqId = 0;
            private bool m_before = true;

            public CSocketError(int errCode, string errMsg, ushort reqId, bool before)
                : base((errMsg == null) ? "" : errMsg)
            {
                base.HResult = errCode;
                m_reqId = reqId;
                m_before = before;
            }

            public CSocketError(int errCode, string errMsg, ushort reqId)
                : base((errMsg == null) ? "" : errMsg)
            {
                base.HResult = errCode;
                m_reqId = reqId;
            }

            public int ErrCode {
                get {
                    return HResult;
                }
            }

            public ushort ReqId {
                get {
                    return m_reqId;
                }
            }

            public bool Before {
                get {
                    return m_before;
                }
            }

            public override string ToString()
            {
                String s = "ec: " + HResult.ToString() + ", em: " + Message + ", reqId: " + m_reqId.ToString() + (m_before ? ", before: true" : ", before: false");
                return s;
            }
        }
    }
}
