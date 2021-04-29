#include "login7.h"

namespace tds
{

	CDBString CLogin7::LibraryName(u"udaparts_client");

	CLogin7::CLogin7() {

	}

	void CLogin7::Reset() {
		memset(&m_CollationChange, 0, sizeof(m_CollationChange));
		m_vEventChange.clear();
		CReqBase::Reset();
	}

	bool CLogin7::GetClientMessage(unsigned char packet_id, const SqlLogin &rec, FeatureExtension requestedFeatures, SPA::CUQueue & buffer) {
		Reset();
		CDBString userName;
		std::vector<unsigned char> encryptedPassword;
		unsigned short encryptedPasswordLengthInBytes = 0;
		if (rec.credential.UserId.size() || rec.credential.Password.size()) {
			userName = rec.credential.UserId;
			encryptedPasswordLengthInBytes = (unsigned short)(rec.credential.Password.size() << 1);
		}
		else {
			userName = rec.userName;
			if (rec.password.size()) {
				encryptedPassword = ObfuscatePassword(rec.password);
				encryptedPasswordLengthInBytes = (unsigned short)encryptedPassword.size();
			}
		}
		unsigned short encryptedChangePasswordLengthInBytes = 0;
		std::vector<unsigned char> encryptedChangePassword;
		if (rec.newSecurePassword.size()) {
			encryptedChangePasswordLengthInBytes = (unsigned short)(rec.newSecurePassword.size() << 1);
		}
		else if (rec.newPassword.size()) {
			encryptedChangePassword = ObfuscatePassword(rec.newPassword);
			encryptedChangePasswordLengthInBytes = (unsigned short)encryptedChangePassword.size();
		}

		SPA::CScopeUQueue sb, sbFeature, sbData;

		// length in bytes
		unsigned int length = YUKON_LOG_REC_FIXED_LEN;
		CDBString clientInterfaceName = ApplicationName;

		length += (unsigned int)((rec.hostName.size() + rec.applicationName.size() +
			rec.serverName.size() + clientInterfaceName.size() +
			rec.language.size() + rec.database.size() +
			rec.attachDBFilename.size()) << 1);
		if (requestedFeatures.GetValue()) {
			length += 4;
		}
		std::vector<unsigned char> outSSPIBuff;
		unsigned short outSSPILength = 0;
		// only add lengths of password and username if not using SSPI or requesting federated authentication info
		if (!rec.useSSPI /*&& !(_connHandler._federatedAuthenticationInfoRequested || _connHandler._federatedAuthenticationRequested)*/) {
			length += (unsigned int)(userName.size() << 1) + encryptedPasswordLengthInBytes + encryptedChangePasswordLengthInBytes;
		}
		else {
			if (rec.useSSPI) {
				/*
				// now allocate proper length of buffer, and set length
						rentedSSPIBuff = ArrayPool<byte>.Shared.Rent((int)s_maxSSPILength);
						outSSPIBuff = rentedSSPIBuff;
						outSSPILength = s_maxSSPILength;

						// Call helper function for SSPI data and actual length.
						// Since we don't have SSPI data from the server, send null for the
						// byte[] buffer and 0 for the int length.
						Debug.Assert(SniContext.Snix_Login == _physicalStateObj.SniContext, $"Unexpected SniContext. Expecting Snix_Login, actual value is '{_physicalStateObj.SniContext}'");
						_physicalStateObj.SniContext = SniContext.Snix_LoginSspi;

						SSPIData(null, 0, ref outSSPIBuff, ref outSSPILength);

						if (outSSPILength > int.MaxValue)
						{
								throw SQL.InvalidSSPIPacketSize();  // SqlBu 332503
						}
						_physicalStateObj.SniContext = SniContext.Snix_Login;

						checked
						{
								length += (int)outSSPILength;
						}
				 */
			}
		}
		unsigned int feOffset = length;
		if (requestedFeatures.GetValue()) {
			/*
			if ((requestedFeatures & TdsEnums.FeatureExtension.SessionRecovery) != 0)
					{
							length += WriteSessionRecoveryFeatureRequest(recoverySessionData, false);
					}
					if ((requestedFeatures & TdsEnums.FeatureExtension.FedAuth) != 0)
					{
							Debug.Assert(fedAuthFeatureExtensionData != null, "fedAuthFeatureExtensionData should not null.");
							length += WriteFedAuthFeatureRequest(fedAuthFeatureExtensionData, write: false);
					}
					if ((requestedFeatures & TdsEnums.FeatureExtension.Tce) != 0)
					{
							length += WriteTceFeatureRequest(false);
					}
					if ((requestedFeatures & TdsEnums.FeatureExtension.GlobalTransactions) != 0)
					{
							length += WriteGlobalTransactionsFeatureRequest(false);
					}
					if ((requestedFeatures & TdsEnums.FeatureExtension.DataClassification) != 0)
					{
							length += WriteDataClassificationFeatureRequest(false);
					}
					if ((requestedFeatures & TdsEnums.FeatureExtension.UTF8Support) != 0)
					{
							length += WriteUTF8SupportFeatureRequest(false);
					}

					if ((requestedFeatures & TdsEnums.FeatureExtension.SQLDNSCaching) != 0)
					{
							length += WriteSQLDNSCachingFeatureRequest(false);
					}

					++length; // for terminator
			 */
		}

		unsigned int ConnectionID = 0, ClientTimeZone = 0, ClientLCID = 0; //not used

		OptionalFlags1 Option1;
		Option1.fUseDB = 1;
		Option1.fDatabase = 1;
		Option1.fSetLang = 1;
		OptionalFlags2 Option2;
		Option2.fLanguage = 1;
		Option2.fODBC = 1;
		if (rec.useReplication) {
			Option2.fUserType = 3;
		}
		if (rec.useSSPI) {
			Option2.fIntSecurity = 1;
		}
		TypeFlags TypeFlags;
		if (rec.readOnlyIntent) {
			TypeFlags.fReadOnlyIntent = 1;
		}
		OptionalFlags3 Option3;
		if (rec.newPassword.size() || rec.newSecurePassword.size()) {
			Option3.fChangePassword = 1;
		}
		if (rec.userInstance) {
			Option3.fUserInstance = 1;
		}
		if (requestedFeatures.GetValue()) {
			Option3.fExtension = 1;
		}
		unsigned int ClientPID = ::GetCurrentProcessId();
		sb << length << TDS_VERSION << rec.packetSize << CLIENT_DLL_VERSION << ClientPID;
		sb << ConnectionID << Option1 << Option2 << TypeFlags << Option3 << ClientTimeZone;
		sb << ClientLCID;

		unsigned short str_len, offset = (unsigned short)YUKON_LOG_REC_FIXED_LEN;
		str_len = (unsigned short)rec.hostName.size();
		sb << offset << str_len;
		offset += (str_len << 1);

		if (!rec.useSSPI /*&& !(_connHandler._federatedAuthenticationInfoRequested || _connHandler._federatedAuthenticationRequested)*/) {
			str_len = (unsigned short)userName.size();
			sb << offset << str_len;
			offset += (str_len << 1);

			str_len = (encryptedPasswordLengthInBytes >> 1);
			sb << offset << str_len;
			offset += (str_len << 1);
		}
		else {
			SPA::UINT64 not_used = 0;
			sb << not_used;
		}
		str_len = (unsigned short)rec.applicationName.size();
		sb << offset << str_len;
		offset += (str_len << 1);

		str_len = (unsigned short)rec.serverName.size();
		sb << offset << str_len;
		offset += (str_len << 1);

		sb << offset;
		if (requestedFeatures.GetValue()) {
			// length of ibFeatgureExtLong (which is a DWORD)
			str_len = 4;
			sb << str_len;
			offset += 4;
		}
		else {
			str_len = 0;
			// unused
			sb << str_len;
		}

		str_len = (unsigned short)clientInterfaceName.size();
		sb << offset << str_len;
		offset += (str_len << 1);

		str_len = (unsigned short)rec.language.size();
		sb << offset << str_len;
		offset += (str_len << 1);

		str_len = (unsigned short)rec.database.size();
		sb << offset << str_len;
		offset += (str_len << 1);

		//ClientID
		assert(TDS_NIC_ADDRESS.size() == 6);
		sb->Push(TDS_NIC_ADDRESS.data(), (unsigned int)TDS_NIC_ADDRESS.size());

		sb << offset;
		if (rec.useSSPI) {
			sb << outSSPILength;
			offset += outSSPILength;
		}
		else {
			str_len = 0;
			sb << str_len;
		}

		str_len = (unsigned short)rec.attachDBFilename.size();
		sb << offset << str_len;
		offset += (str_len << 1);

		//reset password offset
		str_len = (unsigned short)(encryptedChangePasswordLengthInBytes >> 1);
		sb << offset << str_len;

		// reserved for chSSPI
		sb << (unsigned int)0;

		// write variable length portion
		sb->Push((const unsigned char*)rec.hostName.c_str(), (unsigned int)(rec.hostName.size() << 1));

		// if we are using SSPI, do not send over username/password, since we will use SSPI instead
		// same behavior as Luxor
		if (!rec.useSSPI /*&& !(_connHandler._federatedAuthenticationInfoRequested || _connHandler._federatedAuthenticationRequested)*/) {
			sb->Push((const unsigned char*)userName.c_str(), (unsigned int)(userName.size() << 1));
			if (rec.credential.UserId.size() || rec.credential.Password.size()) {
				sb->Push((const unsigned char*)rec.credential.Password.c_str(), (unsigned int)(rec.credential.Password.size() << 1));
			}
			else {
				sb->Push(encryptedPassword.data(), encryptedPasswordLengthInBytes);
			}
		}
		sb->Push((const unsigned char*)rec.applicationName.c_str(), (unsigned int)(rec.applicationName.size() << 1));
		sb->Push((const unsigned char*)rec.serverName.c_str(), (unsigned int)(rec.serverName.size() << 1));
		if (requestedFeatures.GetValue()) {
			sb << feOffset;
		}
		sb->Push((const unsigned char*)clientInterfaceName.c_str(), (unsigned int)(clientInterfaceName.size() << 1));
		sb->Push((const unsigned char*)rec.language.c_str(), (unsigned int)(rec.language.size() << 1));
		sb->Push((const unsigned char*)rec.database.c_str(), (unsigned int)(rec.database.size() << 1));
		// send over SSPI data if we are using SSPI
		if (rec.useSSPI) {
			sb->Push(outSSPIBuff.data(), (unsigned int)outSSPIBuff.size());
		}
		sb->Push((const unsigned char*)rec.attachDBFilename.c_str(), (unsigned int)(rec.attachDBFilename.size() << 1));
		if (!rec.useSSPI/* && !(_connHandler._federatedAuthenticationInfoRequested || _connHandler._federatedAuthenticationRequested)*/) {
			if (rec.newSecurePassword.size()) {
				sb->Push((const unsigned char*)rec.newSecurePassword.c_str(), (unsigned int)(rec.newSecurePassword.size() << 1));
			}
			else {
				sb->Push(encryptedChangePassword.data(), (unsigned int)encryptedChangePasswordLengthInBytes);
			}
		}
		if (requestedFeatures.GetValue()) {
			/*
			if ((requestedFeatures & TdsEnums.FeatureExtension.SessionRecovery) != 0)
			{
					WriteSessionRecoveryFeatureRequest(recoverySessionData, true);
			}
			if ((requestedFeatures & TdsEnums.FeatureExtension.FedAuth) != 0)
			{
					SqlClientEventSource.Log.TryTraceEvent("<sc.TdsParser.TdsLogin|SEC> Sending federated authentication feature request");
					Debug.Assert(fedAuthFeatureExtensionData != null, "fedAuthFeatureExtensionData should not null.");
					WriteFedAuthFeatureRequest(fedAuthFeatureExtensionData, write: true);
			}
			if ((requestedFeatures & TdsEnums.FeatureExtension.Tce) != 0)
			{
					WriteTceFeatureRequest(true);
			}
			if ((requestedFeatures & TdsEnums.FeatureExtension.GlobalTransactions) != 0)
			{
					WriteGlobalTransactionsFeatureRequest(true);
			}
			if ((requestedFeatures & TdsEnums.FeatureExtension.DataClassification) != 0)
			{
					WriteDataClassificationFeatureRequest(true);
			}
			if ((requestedFeatures & TdsEnums.FeatureExtension.UTF8Support) != 0)
			{
					WriteUTF8SupportFeatureRequest(true);
			}

			if ((requestedFeatures & TdsEnums.FeatureExtension.SQLDNSCaching) != 0)
			{
					WriteSQLDNSCachingFeatureRequest(true);
			}
			sb << TOKEN_TERMINATOR;
			 */
		}
		PacketHeader ph(tagPacketType::ptLogin7, packet_id);
		ph.Length = (Packet_Length)(sb->GetSize() + sizeof(PacketHeader));
		ph.Length = ChangeEndian(ph.Length);
		ph.Spid = 0;
		buffer << ph;
		buffer.Push(sb->GetBuffer(), sb->GetSize());
		return true;
	}

	bool CLogin7::ParseStream() {
		SPA::CUQueue &buff = m_buffer;
		while (buff.GetSize()) {
			if (m_tt == tagTokenType::ttZero) {
				m_buffer >> m_tt;
			}
			if (buff.GetSize() < sizeof(unsigned short)) {
				return false;
			}
			switch (m_tt) {
			case tagTokenType::ttTDS_ERROR:
			case tagTokenType::ttINFO:
				if (!ParseErrorInfo()) {
					return false;
				}
				break;
			case tagTokenType::ttLOGINACK:
			{
				unsigned short len = *(unsigned short*)m_buffer.GetBuffer();
				if (len + sizeof(len) > buff.GetSize()) {
					return false;
				}
				unsigned char b;
				buff >> len >> b >> m_LoginAck.Tds_Version;
				assert(b == 1); //Interface
				buff >> b; //byte length
				const char16_t *str = (const char16_t *)buff.GetBuffer();
				m_LoginAck.ServerName.assign(str, str + b);
				buff.Pop((unsigned int)(m_LoginAck.ServerName.size() << 1));
				buff >> m_LoginAck.ServerVersion;
				m_tt = tagTokenType::ttZero;
			}
			break;
			case tagTokenType::ttENVCHANGE:
			{
				unsigned short len = *(unsigned short*)m_buffer.GetBuffer();
				if (len + sizeof(len) > buff.GetSize()) {
					return false;
				}
				tagEnvchangeType type;
				buff >> len >> type;
				switch (type) {
				case tagEnvchangeType::packet_size:
				case tagEnvchangeType::database:
				case tagEnvchangeType::language:
				{
					StringEventChange sec;
					m_vEventChange.push_back(sec);
					ParseStringChange(type, m_vEventChange.back());
					m_tt = tagTokenType::ttZero;
				}
				break;
				case tagEnvchangeType::collation:
					if (!ParseCollation(m_CollationChange))
					{
						return false;
					}
					break;
				default:
					assert(false); //not implemented
					break;
				}
			}
			break;
			case tagTokenType::ttDONE:
				if (!ParseDone()) {
					return false;
				}
				else if (IsDone() && !HasMore() && m_buffer.GetSize()) {
#ifndef NDEBUG
					std::cout << "CLogin7::ParseStream/Remaining bytes: " << m_buffer.GetSize() << "\n";
#endif
				}
				break;
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
