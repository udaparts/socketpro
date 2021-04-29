#include "transmanager.h"
#include <assert.h>

namespace tds {

	CTransManager::CTransManager() {

	}

	void CTransManager::Reset() {
		m_vTransChange.clear();
		CReqBase::Reset();
	}

	void CTransManager::ParseTransChange(tagEnvchangeType type, TransChange& tc) {
		tc.Type = type;
		unsigned char len;
		m_buffer >> len;
		if (len) {
			m_buffer >> tc.NewValue;
		}
		m_buffer >> len;
		if (len) {
			m_buffer >> tc.OldValue;
		}
	}

	bool CTransManager::GetClientMessage(unsigned char packet_id, tagRequestType rt, tagIsolationLevel il, SPA::UINT64 trans_decriptor, SPA::CUQueue &buffer) {
		Reset();
		TransactionDescriptor td(trans_decriptor);
		PacketHeader ph(tagPacketType::ptTransaction, packet_id);
		ph.Length = (Packet_Length)(sizeof(ph) + sizeof(td) + sizeof(rt) + sizeof(il));
		ph.Length = ChangeEndian(ph.Length);
		buffer << ph << td << rt << il;
		return true;
	}

	UINT64 CTransManager::GetTransDescriptor() {
		CAutoLock al(m_cs);
		if (m_vTransChange.size()) {
			return m_vTransChange.front().NewValue;
		}
		return INVALID_NUMBER;
	}

	bool CTransManager::ParseStream() {
		while (m_buffer.GetSize()) {
			if (m_tt == tagTokenType::ttZero) {
				m_buffer >> m_tt;
			}
			switch (m_tt) {
			case tagTokenType::ttTDS_ERROR:
			case tagTokenType::ttINFO:
				if (!ParseErrorInfo()) {
					return false;
				}
				break;
			case tagTokenType::ttDONE:
				if (!ParseDone()) {
					return false;
				}
				else if (IsDone() && !HasMore() && m_buffer.GetSize()) {
#ifndef NDEBUG
					std::cout << "CTransManager::ParseStream/Remaining bytes: " << m_buffer.GetSize() << "\n";
#endif
				}
				break;
			case tagTokenType::ttENVCHANGE:
				if (m_buffer.GetSize() > 2) {
					unsigned short len = *(unsigned short *)m_buffer.GetBuffer();
					if (len + sizeof(unsigned short) <= m_buffer.GetSize()) {
						tagEnvchangeType type;
						m_buffer >> len >> type;
						switch (type) {
						case tagEnvchangeType::begin_trans:
						case tagEnvchangeType::commit_trans:
						case tagEnvchangeType::rollback_trans:
						{
							TransChange tc;
							ParseTransChange(type, tc);
							m_vTransChange.push_back(tc);
						}
						break;
						default:
							assert(false);
							break;
						}
						m_tt = tagTokenType::ttZero;
						break;
					}
				}
				return false;
			default:
				assert(false);
				break;
			}
			if (m_tt != tagTokenType::ttZero) {
				assert(false); //shouldn't come here
				return false;;
			}
		}
		return true;
	}
}
