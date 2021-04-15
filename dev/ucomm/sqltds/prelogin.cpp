
#include "prelogin.h"

namespace tds {
	CPrelogin::CPrelogin(bool mars_enabled, tagFedAuth fa)
		: m_bInst(0),
		m_bMars(mars_enabled ? 1 : 0),
		m_bFed(fa),
		Version(0), SubBuild(0),
		m_bEncryption(tagEncryptionType::etOff),
		m_nThreadId(0) {
		memset(ClientTraceID, 0, sizeof(ClientTraceID));
		memset(ActivityID, 0, sizeof(ActivityID));
		memset(Nonce, 0, sizeof(Nonce));
	}

	bool CPrelogin::GetClientMessage(unsigned char packet_id, SPA::CUQueue &buffer) {
		SPA::CScopeUQueue sbHeader;
		Option option;
		option.Token = tagOptionToken::VERSION;
		option.Len = ChangeEndian((unsigned short)6);
		SPA::CScopeUQueue sbData;
		sbHeader << option;
		sbData << TDS_VERSION << BUILD_VERSION;
		
		option.Token = tagOptionToken::INSTOPT;
		option.Len = ChangeEndian((unsigned short)1);
		sbHeader << option;
		sbData << m_bInst;

		option.Token = tagOptionToken::THREADID;
		option.Len = ChangeEndian((unsigned short)4);
		sbHeader << option;
		sbData << ChangeEndian(GetThreadId());
		unsigned int options = 3;

		if (m_bMars) {
			option.Token = tagOptionToken::MARS;
			option.Len = ChangeEndian((unsigned short)1);
			sbHeader << option;
			sbData << m_bMars;
			++options;
		}

		if (m_bFed) {
			option.Token = tagOptionToken::FEDAUTHREQUIRED;
			option.Len = ChangeEndian((unsigned short)1);
			sbHeader << option;
			sbData << m_bFed;
			++options;
		}

		sbHeader << TOKEN_TERMINATOR;
		unsigned short total_len = (unsigned short) (sbHeader->GetSize() + sbData->GetSize() + sizeof(PacketHeader));
		Option *op = (Option*)sbHeader->GetBuffer();
		PacketHeader ph(tagPacketType::ptPrelogin, packet_id);
		ph.Spid = GetSPID();
		ph.Length = ChangeEndian(total_len);
		unsigned short offset = (unsigned short) sbHeader->GetSize();
		for (unsigned int n = 0; n < options; ++n) {
			op->Offset = ChangeEndian(offset);
			offset += ChangeEndian(op->Len);
			++op;
		}
		buffer << ph;
		buffer.Push(sbHeader->GetBuffer(), sbHeader->GetSize());
		buffer.Push(sbData->GetBuffer(), sbData->GetSize());
		return true;
	}

	void CPrelogin::OnResponse(const unsigned char *data, unsigned int bytes) {
		SPA::CScopeUQueue sb;
		sb->Push(data, bytes);
		SPA::CUQueue &buff = *sb;
		buff >> ResponseHeader;
		ResponseHeader.Length = tds::ChangeEndian(ResponseHeader.Length);
#ifndef NDEBUG
		unsigned int data_len = 0;
#endif
		Option *op = (Option*) buff.GetBuffer();
		while (op->Token != tds::TOKEN_TERMINATOR)
		{
			Option option;
			buff >> option;
			option.Len = tds::ChangeEndian(option.Len);
#ifndef NDEBUG
			data_len += option.Len;
#endif
			option.Offset = tds::ChangeEndian(option.Offset);
			Options.push_back(option);
			op = (tds::CPrelogin::Option*) buff.GetBuffer();
		}
		buff.Pop((unsigned int)1);
#ifndef NDEBUG
		assert(data_len == buff.GetSize());
#endif
		for (auto it = Options.begin(), end = Options.end(); it != end; ++it) {
			if (!it->Len) {
				continue;
			}
			switch (it->Token)
			{
			case tds::CPrelogin::VERSION:
				assert(it->Len == sizeof(Version) + sizeof(SubBuild));
				buff >> Version >> SubBuild;
				Version = tds::ChangeEndian(Version);
				SubBuild = tds::ChangeEndian(SubBuild);
				break;
			case tds::CPrelogin::ENCRYPTION:
				assert(it->Len == sizeof(m_bEncryption));
				buff >> m_bEncryption;
				break;
			case tds::CPrelogin::INSTOPT:
				assert(it->Len == sizeof(m_bInst));
				buff >> m_bInst;
				break;
			case tds::CPrelogin::THREADID:
				assert(it->Len == sizeof(m_nThreadId));
				buff >> m_nThreadId;
				break;
			case tds::CPrelogin::MARS:
				assert(it->Len == sizeof(m_bMars));
				buff >> m_bMars;
				break;
			case tds::CPrelogin::TRACEID:
				assert(it->Len == sizeof(ClientTraceID) + sizeof(ActivityID));
				buff.Pop(ClientTraceID, sizeof(ClientTraceID));
				buff.Pop(ActivityID, sizeof(ActivityID));
				break;
			case tds::CPrelogin::FEDAUTHREQUIRED:
				assert(it->Len == sizeof(m_bFed));
				buff >> m_bFed;
				break;
			case tds::CPrelogin::NONCEOPT:
				assert(it->Len == sizeof(Nonce));
				buff.Pop(Nonce, sizeof(Nonce));
				break;
			default:
				assert(false);
				break;
			}
		}
	}
}
