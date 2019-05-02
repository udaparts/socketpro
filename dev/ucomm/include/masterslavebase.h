
#ifndef _UDAPARTS_MASTER_SLAVE_POOL_BASE_H_
#define _UDAPARTS_MASTER_SLAVE_POOL_BASE_H_

#include "tablecache.h"

namespace SPA {

    template<typename THandler, typename TCS = ClientSide::CClientSocket>
    class CMasterSlaveBase : public ClientSide::CSocketPool < THandler, TCS > {
    public:
        typedef ClientSide::CSocketPool < THandler, TCS> CBase;

        CMasterSlaveBase(const wchar_t *defaultDb, unsigned int recvTimeout = ClientSide::DEFAULT_RECV_TIMEOUT, unsigned int svsId = 0)
        : CBase(true, recvTimeout, ClientSide::DEFAULT_CONN_TIMEOUT, svsId),
        m_dbDefalut(defaultDb ? defaultDb : L"") {
        }

    public:

        inline const std::wstring& GetDefaultDBName() const {
            return m_dbDefalut;
        }

        static std::wstring ToWide(const VARIANT &data) {
            const char *s;
            assert(data.vt == (VT_ARRAY | VT_I1));
            ::SafeArrayAccessData(data.parray, (void**) &s);
            std::wstring ws = Utilities::ToWide(s, data.parray->rgsabound->cElements);
            ::SafeArrayUnaccessData(data.parray);
            return ws;
        }

    private:
        std::wstring m_dbDefalut;
    };
} //namespace SPA

#endif