using System;

namespace SocketProAdapter
{
    public class CServerError : Exception
    {
        private string m_errWhere = "";
        private ushort m_reqId = 0;
        public CServerError(int errCode, string errMsg, string where, ushort reqId)
            : base((errMsg == null) ? "" : errMsg)
        {
            base.HResult = errCode;
            m_errWhere = (where == null) ? "" : where;
            m_reqId = reqId;
        }
        public CServerError(int errCode, string errMsg)
            : base((errMsg == null) ? "" : errMsg)
        {
            base.HResult = errCode;
        }

        public int ErrCode {
            get {
                return HResult;
            }
        }

        public String Where {
            get {
                return m_errWhere;
            }
        }

        public ushort ReqId {
            get {
                return m_reqId;
            }
        }

        public override string ToString()
        {
            String s = "ec: " + HResult.ToString() + ", em: " + Message + ", where: " + m_errWhere + ", reqId: " + m_reqId.ToString();
            return s;
        }
    }
}
