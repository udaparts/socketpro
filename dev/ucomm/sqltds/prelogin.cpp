
#include "prelogin.h"

namespace tds {
	std::atomic<unsigned int> CPrelogin::g_sequence(1);
	CPrelogin::CPrelogin(bool mars_enabled, tagEncryptionType et)
		: m_bMars(mars_enabled ? 1 : 0),
		m_bFed(tagFedAuth::faRequired), //must be tagFedAuth::faRequired
		Version(0), SubBuild(0),
		m_bEncryption(et),
		InstFailed(0) {
	}

	bool CPrelogin::GetClientMessage(unsigned char packet_id, SPA::CUQueue &buffer, const char *instanceName) {
		Reset();
		SPA::CScopeUQueue sbHeader;
		Option option;
		option.Token = tagOptionToken::VERSION;
		option.Len = 6;
		SPA::CScopeUQueue sbData;
		sbHeader << option;
		sbData << ChangeEndian(CLIENT_EXE_VERSION) << BUILD_VERSION; //BUILD_VERSION little endian

		option.Token = tagOptionToken::ENCRYPTION;
		option.Len = 1;
		sbHeader << option;
		sbData << m_bEncryption;
		
		option.Token = tagOptionToken::INSTOPT;
		unsigned short len = (unsigned short)(instanceName ? (::strlen(instanceName) + 1) : 1);
		option.Len = len;
		sbHeader << option;
		sbData->Push(instanceName, len - 1);
		char null_terminated = 0;
		sbData << null_terminated;

		option.Token = tagOptionToken::THREADID;
		option.Len = 4;
		sbHeader << option;
		sbData << ChangeEndian(GetThreadId());

		option.Token = tagOptionToken::MARS;
		option.Len = 1;
		sbHeader << option;
		sbData << m_bMars;

		option.Token = tagOptionToken::TRACEID;
		option.Len = 36;
		sbHeader << option;
		GUID guid0, guid1;
		CoCreateGuid(&guid0); //ClientTraceID
		CoCreateGuid(&guid1);
		unsigned int seqId = ++g_sequence;
		sbData << guid0 << guid1 << seqId; //seqId little endian

		option.Token = tagOptionToken::FEDAUTHREQUIRED;
		option.Len = 1;
		sbHeader << option;
		sbData << m_bFed;

		unsigned int options = sbHeader->GetSize() / sizeof(Option);

		sbHeader << TOKEN_TERMINATOR;
		unsigned short total_len = (unsigned short) (sbHeader->GetSize() + sbData->GetSize() + sizeof(PacketHeader));
		Option *op = (Option*)sbHeader->GetBuffer();
		PacketHeader ph(tagPacketType::ptPrelogin, packet_id);
		ph.Spid = 0;
		ph.Length = ChangeEndian(total_len);
		unsigned short offset = (unsigned short) sbHeader->GetSize();
		for (unsigned int n = 0; n < options; ++n) {
			op->Offset = ChangeEndian(offset);
			offset += op->Len;
			op->Len = ChangeEndian(op->Len);
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
		ResponseHeader.Length = ChangeEndian(ResponseHeader.Length);
		assert(ResponseHeader.Length == bytes);
		std::vector<Option> Options;
#ifndef NDEBUG
		unsigned int data_len = 0;
#endif
		Option *op = (Option*) buff.GetBuffer();
		while (op->Token != tagOptionToken::TOKEN_TERMINATOR)
		{
			Option option;
			buff >> option;
			option.Len = ChangeEndian(option.Len);
#ifndef NDEBUG
			data_len += option.Len;
#endif
			option.Offset = ChangeEndian(option.Offset);
			Options.push_back(option);
			op = (Option*) buff.GetBuffer();
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
			case tagOptionToken::VERSION:
				assert(it->Len == sizeof(Version) + sizeof(SubBuild));
				buff >> Version >> SubBuild; //SubBuild little endian
				Version = ChangeEndian(Version);
				break;
			case tagOptionToken::ENCRYPTION:
				assert(it->Len == sizeof(m_bEncryption));
				buff >> m_bEncryption;
				break;
			case tagOptionToken::INSTOPT:
				assert(it->Len == sizeof(InstFailed)); //little endian
				buff >> InstFailed;
				break;
			case tagOptionToken::THREADID:
				assert(it->Len == 4);
				buff.Pop((unsigned int)4); //do nothing
				break;
			case tagOptionToken::MARS:
				assert(it->Len == sizeof(m_bMars));
				buff >> m_bMars;
				break;
			case tagOptionToken::TRACEID:
				assert(it->Len == 4);
				buff.Pop((unsigned int)4); //do nothing
				break;
			case tagOptionToken::FEDAUTHREQUIRED:
				assert(it->Len == sizeof(m_bFed));
				buff >> m_bFed;
				break;
			default:
				assert(false);
				break;
			}
		}
	}
}
