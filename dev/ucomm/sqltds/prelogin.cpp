
#include "prelogin.h"

namespace tds {
	Prelogin::Prelogin(bool mars_enabled, bool fed_auth_required) 
		: m_bInst(0), m_bMars(mars_enabled), m_bFed(fed_auth_required), Version(0), SubBuild(0), m_bEncryption(0), m_nThreadId(0) {
		memset(ClientTraceID, 0, sizeof(ClientTraceID));
		memset(ActivityID, 0, sizeof(ActivityID));
		memset(Nonce, 0, sizeof(Nonce));
	}

	bool Prelogin::GetClientMessage(unsigned char packet_id, SPA::CUQueue &buffer) {
		unsigned int options = 0;
		SPA::CScopeUQueue sbHeader;
		Option option;
		option.Token = tagOptionToken::VERSION;
		option.Len = ChangeEndian(6);
		SPA::CScopeUQueue sbData;
		sbHeader << option;
		sbData << TDS_VERSION << BUILD_VERSION;
		++options;

		option.Token = tagOptionToken::INSTOPT;
		option.Len = ChangeEndian(1);
		sbHeader << option;
		sbData << m_bInst;
		++options;

		option.Token = tagOptionToken::THREADID;
		option.Len = ChangeEndian(4);
		sbHeader << option;
		sbData << GetThreadId();
		++options;

		if (m_bMars) {
			option.Token = tagOptionToken::MARS;
			option.Len = ChangeEndian(1);
			unsigned char mars = 1;
			sbHeader << option;
			sbData << mars;
			++options;
		}

		if (m_bFed) {
			option.Token = tagOptionToken::FEDAUTHREQUIRED;
			option.Len = ChangeEndian(1);
			unsigned char fed = 1;
			sbHeader << option;
			sbData << fed;
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

	void Prelogin::OnResponse(const unsigned char *data, unsigned int bytes) {
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
			op = (tds::Prelogin::Option*) buff.GetBuffer();
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
			case tds::Prelogin::VERSION:
				assert(it->Len == sizeof(Version) + sizeof(SubBuild));
				buff >> Version >> SubBuild;
				SubBuild = tds::ChangeEndian(SubBuild);
				break;
			case tds::Prelogin::ENCRYPTION:
				assert(it->Len == sizeof(m_bEncryption));
				buff >> m_bEncryption;
				break;
			case tds::Prelogin::INSTOPT:
				assert(it->Len == sizeof(m_bInst));
				buff >> m_bInst;
				break;
			case tds::Prelogin::THREADID:
				assert(it->Len == sizeof(m_nThreadId));
				buff >> m_nThreadId;
				break;
			case tds::Prelogin::MARS:
				assert(it->Len == sizeof(m_bMars));
				buff >> m_bMars;
				break;
			case tds::Prelogin::TRACEID:
				assert(it->Len == sizeof(ClientTraceID) + sizeof(ActivityID));
				buff.Pop(ClientTraceID, sizeof(ClientTraceID));
				buff.Pop(ActivityID, sizeof(ActivityID));
				break;
			case tds::Prelogin::FEDAUTHREQUIRED:
				assert(it->Len == sizeof(m_bFed));
				buff >> m_bFed;
				break;
			case tds::Prelogin::NONCEOPT:
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
