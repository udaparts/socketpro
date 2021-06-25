#include "tdschannel.h"

namespace tds
{

    CTdsChannel::CTdsChannel(SPA::SessionHandle sh) : SPA::CBaseHandler(sh), m_buff(*m_sb) {
    }

    int CTdsChannel::Send(tds::CReqBase* rb, const unsigned char* buffer, unsigned int bytes) {
        assert(rb);
        assert(bytes > sizeof(CReqBase::PacketHeader));
        int fail = 0;
        do {
            m_cs.lock();
            m_deq.push_back(rb);
            m_cs.unlock();
            fail = SPA::CBaseHandler::Send(buffer, bytes);
            if (fail) {
                m_cs.lock();
                for (auto it = m_deq.begin(), end = m_deq.end(); it != end; ++it) {
                    if (rb == *it) {
                        m_deq.erase(it);
                        break;
                    }
                }
                m_cs.unlock();
                return fail;
            }
        } while (false);
        return fail;
    }

    size_t CTdsChannel::GetQueuedPackets() {
        m_cs.lock();
        size_t count = m_deq.size();
        m_cs.unlock();
        return count;
    }

    void CTdsChannel::OnClosed() {
#ifndef NDEBUG
        char errMsg[512];
        int errCode = GetErrorCode(errMsg, sizeof (errMsg));
        if (errCode) {
            std::cout << "Session closed: error code=" << errCode << ", error message=" << errMsg << "\n";
        }
#endif
        Reset();
    }

    void CTdsChannel::OnConnected() {
#ifndef NDEBUG
        char errMsg[512];
        int errCode = GetErrorCode(errMsg, sizeof (errMsg));
        if (errCode) {
            std::cout << "Session connected: error code=" << errCode << ", error message=" << errMsg << "\n";
        }
#endif
        Reset();
    }

    void CTdsChannel::Reset() {
        m_buff.SetSize(0);
        SPA::CSpinAutoLock al(m_cs);
        while (m_deq.size()) {
            CReqBase* f = m_deq.front();
            f->OnChannelClosed();
            m_deq.pop_front();
        }
    }

    void CTdsChannel::OnAvailable(const unsigned char* data, unsigned int bytes) {
        m_buff.Push(data, bytes);
        do {
            if (m_buff.GetSize() < sizeof (CReqBase::PacketHeader)) {
                break;
            }
            CReqBase::PacketHeader* ph = (CReqBase::PacketHeader *) m_buff.GetBuffer();
            unsigned int len = CReqBase::ChangeEndian(ph->Length);
            if (m_buff.GetSize() < len) {
                break;
            }
            m_cs.lock();
#ifndef NDEBUG
            if (!m_deq.size()) {
                assert(false);
                m_cs.unlock();
                break;
            }
#endif
            tds::CReqBase* rb = m_deq.front();
            m_cs.unlock();
            rb->OnResponse(m_buff.GetBuffer(), len);
#ifndef NDEBUG
            if (rb->m_buffer.GetSize()) {
                std::cout << "Remaining bytes: " << rb->m_buffer.GetSize() << "\n";
            }
#endif
            if (rb->IsDone()) {
                m_cs.lock();
                m_deq.pop_front();
                while (m_deq.size() && m_deq.front() == rb) {
                    m_deq.pop_front();
                }
                m_cs.unlock();
            }
            m_buff.Pop(len);
        } while (true);
    }

} //namespace tds
