
#include "stdafx.h"
#include "dbupdateimpl.h"
#include <sstream>

CDBUpdateImpl CDBUpdateImpl::DBUpdate;

CDBUpdateImpl::CDBUpdateImpl() {


}

CDBUpdateImpl::~CDBUpdateImpl() {
    SPA::CAutoLock al(m_cs);
    if (m_pPool) {
        m_pPool->ShutdownPool();
    }
}

unsigned int CDBUpdateImpl::SetSocketProConnectionString(const wchar_t *connectionString) {
    unsigned int count = 0;
    using namespace std;
    typedef SPA::ClientSide::CConnectionContext* PCConnectionContext;
    PCConnectionContext p = nullptr;
    wstringstream ss(connectionString);
    wstring item;
    vector<wstring> tokens;
    while (getline(ss, item, L'*')) {
        tokens.push_back(item);
    }
    {
        SPA::CAutoLock al(m_cs);
        if (m_pPool) {
            m_pPool->ShutdownPool();
        } else {
            m_pPool.reset(new SPA::ClientSide::CSocketPool<CAsyncDBUpdate>(true, SPA::ClientSide::DEFAULT_RECV_TIMEOUT * 4, SPA::ClientSide::DEFAULT_CONN_TIMEOUT / 15));
            m_pPool->DoSslServerAuthentication = [](SPA::ClientSide::CSocketPool<CAsyncDBUpdate> *sender, SPA::ClientSide::CClientSocket * cs)->bool {
                int errCode;
                SPA::IUcert *cert = cs->GetUCert();
                const char* res = cert->Verify(&errCode);

                //do ssl server certificate authentication here

                return (errCode == 0); //true -- user id and password will be sent to server
            };
            m_pPool->SocketPoolEvent = [this](SPA::ClientSide::CSocketPool<CAsyncDBUpdate> *sender, SPA::ClientSide::tagSocketPoolEvent spe, CAsyncDBUpdate * handler) {
                switch (spe) {
                    case SPA::ClientSide::speUSocketCreated:
                        if (handler) {
                            size_t pos = sender->GetSocketHandlerMap().size() - 1;
                            handler->SetSvsID(m_vCC[pos].second);
                        }
                        break;
                    case SPA::ClientSide::speConnected:
                        if (handler) {
                            size_t pos = 0;
                            auto map = sender->GetSocketHandlerMap();
                            for (auto it = map.begin(), end = map.end(); it != end; ++it, ++pos) {
                                if (it->second.get() == handler) {
                                    int zip = m_vZip[pos];
                                    if (zip) {
                                        handler->GetAttachedClientSocket()->SetZipLevel(SPA::zlBestSpeed);
                                    }
                                    break;
                                }
                            }
                        }
                        break;
                    default:
                        break;
                }
            };
        }
        m_vCC.clear();
        m_cert.clear();
        m_vZip.clear();
        for (auto it = tokens.begin(), end = tokens.end(); it != end; ++it) {
            Parse(it->c_str());
        }
        count = (unsigned int) m_vCC.size();
        if (!count) {
            return 0;
        }
        p = new SPA::ClientSide::CConnectionContext[count];
        for (unsigned int n = 0; n < count; ++n) {
            p[n] = m_vCC[n].first;
        }
        SPA::ClientSide::CClientSocket::SSL::SetVerifyLocation(m_cert.c_str());
        PCConnectionContext ppCCs[1] = {p};
        bool ok = m_pPool->StartSocketPool(ppCCs, 1, count, true, SPA::taNone);
        m_vCC.clear();
    }
    delete []p;
    return count;
}

void CDBUpdateImpl::Trim(std::wstring & s) {
    static const wchar_t *WHITESPACE = L" \r\n\t\v\f\v";
    auto pos = s.find_first_of(WHITESPACE);
    while (pos == 0) {
        s.erase(s.begin());
        pos = s.find_first_of(WHITESPACE);
    }
    pos = s.find_last_of(WHITESPACE);
    while (s.size() && pos == s.size() - 1) {
        s.pop_back();
        pos = s.find_last_of(WHITESPACE);
    }
}

void CDBUpdateImpl::Parse(const wchar_t *s) {
    using namespace std;
    if (!wcsstr(s, L"="))
        return;
    wstringstream ss(s);
    wstring item;
    vector<wstring> tokens;
    while (getline(ss, item, L';')) {
        tokens.push_back(item);
    }
    int zip = 0;
    unsigned int service_id = 0;
    SPA::ClientSide::CConnectionContext cc;
    for (auto it = tokens.begin(), end = tokens.end(); it != end; ++it) {
        auto pos = it->find(L'=');
        if (pos == string::npos)
            continue;
        wstring left = it->substr(0, pos);
        wstring right = it->substr(pos + 1);
        Trim(left);
        Trim(right);
        transform(left.begin(), left.end(), left.begin(), ::tolower);
        if (left == L"port")
            cc.Port = (unsigned int) std::atoi(SPA::Utilities::ToUTF8(right.c_str()).c_str());
        else if (left == L"pwd" || left == L"password")
            cc.Password = right;
        else if (left == L"host" || left == L"server")
            cc.Host = SPA::Utilities::ToUTF8(right.c_str());
        else if (left == L"user" || left == L"uid" || left == L"userid" || left == L"user-id")
            cc.UserId = right;
        else if (left == L"service-id" || left == L"serviceid" || left == L"svsid")
            service_id = (unsigned int) std::atoi(SPA::Utilities::ToUTF8(right.c_str()).c_str());
        else if (left == L"cert" || left == L"certificate") {
            if (right.size()) {
                cc.EncrytionMethod = SPA::TLSv1;
                m_cert = SPA::Utilities::ToUTF8(right.c_str());
            }
        } else if (left == L"zip") {
            if (right.size()) {
                cc.Zip = true;
                zip = std::atoi(SPA::Utilities::ToUTF8(right.c_str()).c_str());
            }
        } else if (left == L"v6" || left == L"ipv6") {
            int v6 = std::atoi(SPA::Utilities::ToUTF8(right.c_str()).c_str());
            if (v6) {
                cc.V6 = true;
            }
        } else {
            //!!! not implemented
            assert(false);
        }
    }
    m_vCC.push_back(std::pair<SPA::ClientSide::CConnectionContext, unsigned int>(cc, service_id));
    m_vZip.push_back(zip);
}

SPA::UINT64 CDBUpdateImpl::NotifySocketProDatabaseEvent(unsigned int *group, unsigned int count, SPA::UDB::tagUpdateEvent dbEvent, const wchar_t *queryFilter, unsigned int *index, unsigned int size) {
    if (size) {
        ::memset(index, 0, size * sizeof (unsigned int));
    }
    unsigned int connected = 0, disconnected = 0;
    {
        unsigned int n = 0;
        std::wstring str = std::to_wstring((SPA::UINT64)dbEvent) + L"/";
        str += queryFilter;
        SPA::CAutoLock mal(m_cs);
        if (!m_pPool)
            return 0;
        auto sockets = m_pPool->GetSockets();
        for (auto it = sockets.begin(), end = sockets.end(); it != end; ++it, ++n) {
            auto s = *(it);
#ifndef WIN32_64
            bool ok = s->GetPush().Publish(str, group, count);
#else
            bool ok = s->GetPush().Publish(str.c_str(), group, count);
#endif
            if (ok) {
                ++connected;
            } else {
                if (n < size) {
                    index[n] = 1;
                }
                ++disconnected;
            }
        }
    }
    SPA::UINT64 data = disconnected;
    data <<= 32;
    data += connected;
    return data;
}

SPA::UINT64 CDBUpdateImpl::GetSocketProConnections(unsigned int *index, unsigned int size) {
    if (size) {
        ::memset(index, 0, size * sizeof (unsigned int));
    }
    unsigned int connected = 0, disconnected = 0;
    {
        unsigned int n = 0;
        SPA::CAutoLock mal(m_cs);
        if (!m_pPool)
            return 0;
        auto sockets = m_pPool->GetSockets();
        for (auto it = sockets.begin(), end = sockets.end(); it != end; ++it, ++n) {
            bool sendable = (*it)->Sendable();
            if (sendable) {
                ++connected;
            } else {
                if (n < size) {
                    index[n] = 1;
                }
                ++disconnected;
            }
        }
    }
    SPA::UINT64 data = disconnected;
    data <<= 32;
    data += connected;
    return data;
}

