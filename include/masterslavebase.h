
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

    protected:

        static std::wstring ToWide(const VARIANT &data) {
            if (data.vt == VT_BSTR) {
                return data.bstrVal;
            }
            const char *s;
            assert(data.vt == (VT_ARRAY | VT_I1));
            ::SafeArrayAccessData(data.parray, (void**) &s);
            std::wstring ws = Utilities::ToWide(s, data.parray->rgsabound->cElements);
            ::SafeArrayUnaccessData(data.parray);
            return ws;
        }

        static CDBString ToUTF16(const VARIANT &data) {
#ifndef NATIVE_UTF16_SUPPORTED
            if (data.vt == VT_BSTR) {
                return data.bstrVal;
            }
            const char *s;
            assert(data.vt == (VT_ARRAY | VT_I1));
            ::SafeArrayAccessData(data.parray, (void**) &s);
            std::wstring ws = Utilities::ToWide(s, data.parray->rgsabound->cElements);
            ::SafeArrayUnaccessData(data.parray);
            return ws;
#else
            SPA::CScopeUQueue sb;
            if (data.vt == VT_BSTR) {
                Utilities::ToUTF16(data.bstrVal, (~0), *sb, true);
            }
            const char *s;
            assert(data.vt == (VT_ARRAY | VT_I1));
            ::SafeArrayAccessData(data.parray, (void**) &s);
            Utilities::ToUTF16(s, data.parray->rgsabound->cElements, *sb, true);
            ::SafeArrayUnaccessData(data.parray);
            return (const UTF16*) sb->GetBuffer();
#endif
        }

    private:
        std::wstring m_dbDefalut;
    };
} //namespace SPA

#endif