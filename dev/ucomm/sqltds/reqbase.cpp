#include "reqbase.h"
#include "tdschannel.h"
#include "../include/odbc/uodbc.h"
#include <chrono>
using namespace std::chrono_literals;

namespace tds{

    CReqBase::CReqBase(CTdsChannel & channel) : m_channel(channel), m_buffer(*m_sb),
    m_tt(tagTokenType::ttZero), ResponseHeader(tagPacketType::ptInitial, 0), m_bWaiting(false) {
        if (m_buffer.GetMaxSize() >= SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE) {
            m_buffer.ReallocBuffer(SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE);
        }

        m_collation.CodePage = (unsigned short) GetSystemDefaultLCID();
        m_collation.Flags.fIgnoreCase = 1;
        m_collation.Flags.fIgnoreAccent = 1;
        m_collation.Flags.fIgnoreKana = 1;
        m_collation.SortOrder = 52;
    }

    CReqBase::~CReqBase() {
    }

    unsigned short CReqBase::ChangeEndian(unsigned short s) {
        return ((s & 0xff) << 8) + (s >> 8);
    }

    unsigned char CReqBase::GetFixLen(Token token) {
        assert((token & 12) == 12);
        token <<= 4;
        token &= 3;
        switch (token) {
            case 0:
                return 1;
            case 1:
                return 4;
            case 2:
                return 2;
            case 3:
                return 8;
            default:
                assert(false); //shouldn't come here
                break;
        }
        return 255;
    }

    unsigned int CReqBase::ChangeEndian(unsigned int s) {
        unsigned char* p = (unsigned char*) &s;
        unsigned char b = p[0];
        p[0] = p[3];
        p[3] = b;
        b = p[1];
        p[1] = p[2];
        p[2] = b;
        return s;
    }

    int CReqBase::ChangeEndian(int s) {
        unsigned char* p = (unsigned char*) &s;
        unsigned char b = p[0];
        p[0] = p[3];
        p[3] = b;
        b = p[1];
        p[1] = p[2];
        p[2] = b;
        return s;
    }

    unsigned int CReqBase::GetThreadId() {
#ifdef WIN32_64
        UTHREAD_ID tid = ::GetCurrentThreadId();
#else
        UTHREAD_ID tid = pthread_self();
#endif
        return (unsigned int) tid;
    }

    CReqBase::SPID CReqBase::GetSPID() {
#ifdef WIN32_64
        UTHREAD_ID tid = ::GetCurrentThreadId();
#else
        UTHREAD_ID tid = pthread_self();
#endif
        return ChangeEndian((SPID) tid);
    }

    CTdsChannel & CReqBase::GetChannel() {
        return m_channel;
    }

    const CReqBase::PacketHeader & CReqBase::GetResponseHeader() const {
        return ResponseHeader;
    }

    bool CReqBase::IsDone() {
        CAutoLock al(m_cs);
        return IsDoneInternal();
    }

    bool CReqBase::HasMore() {
        CAutoLock al(m_cs);
        return HasMoreInternal();
    }

    UINT64 CReqBase::GetCount() {
        CAutoLock al(m_cs);
        return GetCountInternal();
    }

    bool CReqBase::HasMoreInternal() const {
        return ((m_Done.Status & tagDoneStatus::dsMore) == tagDoneStatus::dsMore);
    }

    UINT64 CReqBase::GetCountInternal() const {
        if ((m_Done.Status & tagDoneStatus::dsCount) == tagDoneStatus::dsCount) {
            return m_Done.RowCount;
        }
        return INVALID_NUMBER;
    }

    bool CReqBase::IsDoneInternal() const {
        return (ResponseHeader.Status == tagPacketStatus::psEOM);
    }

    void CReqBase::Reset() {
        CAutoLock al(m_cs);
        ::memset(&ResponseHeader, 0, sizeof (ResponseHeader));
        memset(&m_Done, 0, sizeof (m_Done));
    }

    bool CReqBase::ParseDone() {
        if (m_buffer.GetSize() >= sizeof (m_Done)) {
            {
                CAutoLock al(m_cs);
                m_buffer >> m_Done;
            }
            m_tt = tagTokenType::ttZero;
            return true;
        }
        return false;
    }

    int CReqBase::GetSQLError(SPA::CDBString & errMsg) {
        CAutoLock al(m_cs);
        bool bad = (m_errCode.Class > 10);
        unsigned int ec = m_errCode.Class;
        if (bad) {
            ec |= 0x80;
        }
        ec <<= 8;
        ec += (m_errCode.State << 1);
        ec <<= 16;
        ec += m_errCode.SQLErrorNumber;
        errMsg = m_errCode.ErrorMessage;
        return (int) ec;
    }

    int CReqBase::GetError(int fail, SPA::CDBString & errMsg) {
        if (!fail) {
            errMsg.clear();
            return 0;
        } else if (fail < 0) {
            switch (fail) {
                case SPA::Odbc::ER_NO_DB_OPENED_YET:
                    errMsg = u"No SQL server database opened yet";
                    return fail;
                case SPA::Odbc::ER_BAD_END_TRANSTACTION_PLAN:
                    errMsg = u"Bad end transaction plan";
                    return fail;
                case SPA::Odbc::ER_NO_PARAMETER_SPECIFIED:
                    errMsg = u"No parameter specified";
                    return fail;
                case SPA::Odbc::ER_BAD_PARAMETER_COLUMN_SIZE:
                    errMsg = u"Bad parameter info array size found";
                    return fail;
                case SPA::Odbc::ER_BAD_PARAMETER_DATA_ARRAY_SIZE:
                    errMsg = u"Bad parameter data array length";
                    return fail;
                case SPA::Odbc::ER_DATA_TYPE_NOT_SUPPORTED:
                    errMsg = u"Data type not supported";
                    return fail;
                case SPA::Odbc::ER_NO_DB_NAME_SPECIFIED:
                    errMsg = u"No database name specified";
                    return fail;
                case SPA::Odbc::ER_BAD_MANUAL_TRANSACTION_STATE:
                    errMsg = u"Bad manual transaction state";
                    return fail;
                case SPA::Odbc::ER_BAD_INPUT_PARAMETER_DATA_TYPE:
                    errMsg = u"Bad parameter data type found";
                    return fail;
                case SPA::Odbc::ER_BAD_PARAMETER_DIRECTION_TYPE:
                    errMsg = u"Bad parameter direction type";
                    return fail;
                case SPA::Odbc::ER_CORRECT_PARAMETER_INFO_NOT_PROVIDED_YET:
                    errMsg = u"Parameter information not provided yet";
                    return fail;
                case ER_SQL_REQUEST_TIMEDOUT:
                    errMsg = u"SQL request timed out";
                    return fail;
                case ER_SQL_SERVER_SESSION_CLOSED_GRACEFULLY:
                    errMsg = u"SQL server session closed gracefully";
                    return fail;
                case ER_BAD_DECIMAL_PRECSION_PROVIDED:
                    errMsg = u"Decimal precision cannot be zero";
                    return fail;
                case ER_BAD_OUTPUT_PARAMETER_DATA_TYPE:
                    errMsg = u"Bad output parameter data type found";
                    return fail;
                case ER_NO_PARAMETER_NAME_PROVIDED:
                    errMsg = u"Stored procedure parameter name not provided yet";
                    return fail;
                case ER_BAD_PARAMETER_INFO_COLUMN_SIZE:
                    errMsg = u"Wrong parameter info column size found";
                    return fail;
                default:
                {
                    int res = GetSQLError(errMsg);
                    assert(res == fail);
                }
                    return fail;
            }
        }
        SPA::CScopeUQueue sb;
        fail = m_channel.GetErrorCode((char*) sb->GetBuffer(), sb->GetMaxSize());
        const char* em = (const char*) sb->GetBuffer();
        errMsg.assign(em, em + strlen(em));
        return fail;
    }

    bool CReqBase::ParseError() {
        if (m_buffer.GetSize() > 2) {
            unsigned short len = *(unsigned short*) m_buffer.GetBuffer();
            if (len + sizeof (len) <= m_buffer.GetSize()) {
                {
                    CAutoLock al(m_cs);
                    TokenInfo& ti = m_errCode;
                    m_buffer >> len >> ti.SQLErrorNumber >> ti.State >> ti.Class;
                    m_buffer >> len;
                    const char16_t* str = (const char16_t*) m_buffer.GetBuffer();
                    ti.ErrorMessage.assign(str, str + len);
                    m_buffer.Pop(((unsigned int) len) << 1);
                    unsigned char byteLen;
                    m_buffer >> byteLen;
                    str = (const char16_t*) m_buffer.GetBuffer();
                    ti.ServerName.assign(str, str + byteLen);
                    m_buffer.Pop(((unsigned int) byteLen) << 1);
                    m_buffer >> ti.ProcessNameLength;
                    if (ti.ProcessNameLength) {
                        const char16_t* start = (const char16_t*) m_buffer.GetBuffer();
                        ti.ProcName.assign(start, ti.ProcessNameLength);
                        m_buffer.Pop(((unsigned int) ti.ProcessNameLength) << 1);
                    } else {
                        ti.ProcName.clear();
                    }
                    m_buffer >> ti.LineNumber;
                }
                m_tt = tagTokenType::ttZero;
                return true;
            }
        }
        return false;
    }

    int CReqBase::Wait(unsigned int milliseconds) {
        CAutoLock al(m_cs);
        if (IsDoneInternal()) {
            return 0;
        }
        m_bWaiting = true;
        if (m_cv.wait_for(al, milliseconds * 1ms) != std::cv_status::no_timeout) {
            m_bWaiting = false;
            return ER_SQL_REQUEST_TIMEDOUT;
        }
        m_bWaiting = false;
        int fail = m_channel.GetErrorCode(nullptr, 0);
        if (fail) {
            return fail;
        }
        if (!m_channel.IsConnected()) {
            fail = ER_SQL_SERVER_SESSION_CLOSED_GRACEFULLY;
        }
        return fail;
    }

    void CReqBase::OnChannelClosed() {
        Reset();
        CAutoLock al(m_cs);
        if (m_bWaiting) {
            m_cv.notify_all();
        }
    }

    int CReqBase::Send(const unsigned char* buffer, unsigned int bytes, unsigned int milliseconds, bool sync) {
        assert(buffer);
        assert(bytes >= sizeof (PacketHeader));
        int fail = 0;
        {
            CAutoLock al(m_csSend);
            {
                CAutoLock al(m_cs);
                if (!m_channel.GetQueuedPackets()) {
                    PacketHeader* ph = (PacketHeader*) buffer;
                    if (ph->Type == tagPacketType::ptAttention) {
                        return 0;
                    }
                }
                m_errCode.Reset();
                ResponseHeader.Status = tagPacketStatus::psNormal;
                m_Done.Status = tagDoneStatus::dsMore;
            }
            fail = m_channel.Send(this, buffer, bytes);
            if (fail) {
                return fail;
            }
            if (!sync) {
                if (!m_channel.IsConnected()) {
                    fail = ER_SQL_SERVER_SESSION_CLOSED_GRACEFULLY;
                }
                return fail;
            }
        }
        CAutoLock al(m_cs);
        m_bWaiting = true;
        if (ResponseHeader.Status != tagPacketStatus::psEOM) {
            if (m_cv.wait_for(al, milliseconds * 1ms) == std::cv_status::no_timeout) {
                fail = m_channel.GetErrorCode(nullptr, 0);
                if (!fail && !m_channel.IsConnected()) {
                    fail = ER_SQL_SERVER_SESSION_CLOSED_GRACEFULLY;
                }
            } else {
                fail = ER_SQL_REQUEST_TIMEDOUT;
            }
        }
        m_bWaiting = false;
        return fail;
    }

    void CReqBase::OnResponse(const unsigned char* data, unsigned int bytes) {
        assert(bytes >= sizeof (ResponseHeader));
        {
            CAutoLock al(m_cs);
            memcpy(&ResponseHeader, data, sizeof (ResponseHeader));
            ResponseHeader.Length = ChangeEndian(ResponseHeader.Length);
            assert(ResponseHeader.Length == bytes);
            ResponseHeader.Spid = ChangeEndian(ResponseHeader.Spid);
        }
        data += sizeof (ResponseHeader);
        bytes -= sizeof (ResponseHeader);
        m_buffer.Push(data, bytes);
        try
        {
            ParseStream();
        }

        catch(SPA::CUException & ex) {
#ifndef NDEBUG
            std::cout << "serialization error: " << ex.what() << "\n";
#endif
        }

        catch(std::exception & ex) {
#ifndef NDEBUG
            std::cout << "stl error: " << ex.what() << "\n";
#endif
        }

        catch(...) {
#ifndef NDEBUG
            std::cout << "Unknown error\n";
#endif
        }
        CAutoLock al(m_cs);
        if (m_bWaiting && IsDoneInternal()) {
            m_cv.notify_all();
        }
    }
}
