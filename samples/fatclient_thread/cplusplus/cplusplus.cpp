
#include "stdafx.h"

using namespace SPA::UDB;

typedef SPA::ClientSide::CSqliteBase CMyHandler;
typedef SPA::ClientSide::CSocketPool<SPA::ClientSide::CSqlite> CMyPool;
typedef SPA::ClientSide::CConnectionContext CMyConnContext;
typedef std::pair<CDBColumnInfoArray, CDBVariantArray> CPColumnRowset;
typedef std::vector<CPColumnRowset> CRowsetArray;

/*
//This is bad implementation for original SPA::ClientSide::CAsyncDBHandler::Open method!!!!
virtual bool Open(const wchar_t* strConnection, DResult handler, unsigned int flags = 0) {
    std::wstring s;
    CAutoLock al(m_csDB);
    m_flags = flags;
    if (strConnection) {
        s = m_strConnection;
        m_strConnection = strConnection;
    }
	//self cross-SendRequest dead-locking here !!!!
    if (SendRequest(UDB::idOpen, strConnection, flags, [handler, this](CAsyncResult & ar) {
            int res, ms;
            std::wstring errMsg;
            ar >> res >> errMsg >> ms;
            this->m_csDB.lock(); //self dead-lock !!!!
            this->CleanRowset();
            this->m_dbErrCode = res;
            this->m_lastReqId = idOpen;
            if (res == 0) {
                this->m_strConnection = std::move(errMsg);
                errMsg.clear();
            } else {
                this->m_strConnection.clear();
            }
            this->m_dbErrMsg = errMsg;
            this->m_ms = (tagManagementSystem) ms;
            this->m_parameters = 0;
            this->m_outputs = 0;
            this->m_csDB.unlock();
            if (handler) {
                handler(*this, res, errMsg);
            }
        })) {
		return true;
	}
    if (strConnection) {
        m_strConnection = s;
    }
    return false;
}
*/

#define sample_database L"mysample.db"
void Demo_Cross_Locking_Dead_Lock(CMyPool::PHandler sqlite) {
    unsigned int count = 1000000;
    do
    {
        bool ok = sqlite->Open(sample_database, [](CMyHandler &handler, int res, const std::wstring &errMsg) {
            if (res != 0)
				std::cout << "Open: res = " << res << ", errMsg: "; std::wcout << errMsg << std::endl;
        });
        --count;
    } while (count > 0);
}

int main(int argc, char* argv[])
{
	CMyConnContext cc;
    std::cout << "Remote host: " << std::endl;
    std::getline(std::cin, cc.Host);
    //cc.Host = "localhost";
    cc.Port = 20901;
    cc.UserId = L"MyUserId";
    cc.Password = L"MyPassword";
    CMyPool spSqlite;

	bool ok = spSqlite.StartSocketPool(cc, 1, 2);
    if (!ok) {
        std::cout << "No connection to sqlite server and press any key to close the demo ......" << std::endl;
        ::getchar(); return 0;
    }
	CMyPool::PHandler sqlite = spSqlite.GetAsyncHandlers()[0];
	//Use the above bad implementation to replace original SPA::ClientSide::CAsyncDBHandler::Open method
    //at file socketpro/include/udb_client.h
    std::cout << "Doing Demo_Cross_Locking_Dead_Lock ......" << std::endl;
	Demo_Cross_Locking_Dead_Lock(sqlite);

	std::cout << "Press any key to close the application ......" << std::endl;
    ::getchar();
	return 0;
}

