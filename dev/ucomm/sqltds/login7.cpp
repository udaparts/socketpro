#include "login7.h"

namespace tds {

	CLogin7::CLogin7(FeatureExtension fe, bool integrated, bool dump, unsigned int packet_size, bool readOnlyIntent) 
		: m_fe(fe), TDSVersion(TDS_VERSION), PacketSize(packet_size), ClientProgVer(CLIENT_PROG_VERSION),
		ClientPID(0), ConnectionID(0), ClientTimeZone(0), ClientLCID(0), SSPI(nullptr), SSPILong(0) {
		memset(ClientID, 0, sizeof(ClientID));
		Option1.fDumpLoad = (dump ? 0 : 1);
		Option1.fUseDB = 1;
		Option1.fDatabase = 1;
		Option1.fSetLang = 1;
		Option2.fLanguage = 1;
		Option2.fODBC = 1;
		Option2.fIntSecurity = (integrated ? 1 : 0); //SSPI
		TypeFlags.fOLEDB = 1;
		TypeFlags.fReadOnlyIntent = (readOnlyIntent ? 1 : 0);
		if (fe.GetValue()) Option3.fExtension = 1;
	}

	bool CLogin7::GetClientMessage(unsigned char packet_id, SPA::CUQueue &buffer) {
		// Calculate total length by adding strings
		unsigned int totalPacketLength = FixedPacketLength
			+ (unsigned int)(HostName.size() << 1)
			+ (unsigned int)(UserID.size() << 1)
			+ (unsigned int)(Password.size() << 1)
			+ (unsigned int)(AppName.size() << 1)
			+ (unsigned int)(ServerName.size() << 1)
			+ (unsigned int)(LibraryName.size() << 1)
			+ (unsigned int)(Language.size() << 1)
			+ (unsigned int)(Database.size() << 1)
			+ (unsigned int)(AttchDBFile.size() << 1)
			+ (unsigned int)(ChangePassword.size() << 1)
			+ (SSPI ? SSPILong : 0)
			+ 0; //Feature extension

		SPA::CScopeUQueue sb, sbFeature, sbData;
		PacketHeader ph(tagPacketType::ptLogin7, packet_id);

		sb << ChangeEndian(totalPacketLength) << TDS_VERSION << ChangeEndian(PacketSize) << ClientProgVer << ChangeEndian(ClientPID);
		sb << ChangeEndian(ConnectionID) << Option1 << Option2 << TypeFlags << Option3 << ChangeEndian(ClientTimeZone);
		sb << ChangeEndian(ClientLCID);


		
		return false;
	}

	void CLogin7::OnResponse(const unsigned char *data, unsigned int bytes) {
		SPA::CScopeUQueue sb;
		sb->Push(data, bytes);
		SPA::CUQueue &buff = *sb;
		buff >> ResponseHeader;
		ResponseHeader.Length = ChangeEndian(ResponseHeader.Length);
	}

}