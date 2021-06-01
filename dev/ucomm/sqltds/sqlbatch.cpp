#include "sqlbatch.h"
#include <iostream>

namespace tds
{
    CDBString CSqlBatch::LibraryName(u"udaparts_sql_server_client");

    CSqlBatch::CSqlBatch(SPA::CBaseHandler& channel, bool meta)
        : CReqBase(channel), m_out(*m_sbOut), m_meta(meta), m_cols(0), m_posCol(INVALID_COL), m_lenLarge(0),
        m_endLarge(0), m_rs(0), m_inputs(0), m_outputs(0), m_returned(false) {
    }

    void CSqlBatch::Reset() {
        m_vInfo.clear();
        m_vTransChange.clear();
        memset(&m_CollationChange, 0, sizeof(m_CollationChange));
		m_vEventChange.clear();
        m_vCol.clear();
        m_cols = 0;
        m_vDT.clear();
        m_posCol = INVALID_COL;
		m_dip.Status = tagDoneStatus::dsInitial;
		m_rs = 0;
        CReqBase::Reset();
    }

    CDBString CSqlBatch::Prepare(const char16_t* sql, unsigned int& parameters, bool& returned, CDBString& procName, CDBString& catalogSchema) {
        assert(sql);
        assert(SPA::GetLen(sql));
        catalogSchema.clear();
        procName.clear();
        bool called = false;
        returned = false;
        parameters = 0;
        CDBString s = sql ? sql : u"";
        SPA::Trim(s);
        if (s.size() && s.front() == '{' && s.back() == '}') {
            s.pop_back();
            s.erase(s.begin(), s.begin() + 1);
            SPA::Utilities::Trim(s);
        }
        if (!s.size()) {
            return s;
        }
        returned = (s.front() == '?');
        if (returned) {
            s.erase(s.begin(), s.begin() + 1); //remove '?'
            SPA::Utilities::Trim(s);
            if (s.front() != '=')
                return u"";
            s.erase(s.begin(), s.begin() + 1); //remove '='
            SPA::Utilities::Trim(s);
        }
        CDBString s_copy = s;
        SPA::ToLower(s);
        called = (s.find(u"call ") == 0);
        if (called) {
            auto pos = s_copy.find('(');
            if (pos != CDBString::npos) {
                if (s_copy.back() != ')')
                    return u"";
                procName.assign(s_copy.begin() + 5, s_copy.begin() + pos);
            }
            else {
                if (s_copy.back() == ')')
                    return u"";
                procName = s_copy.substr(5);
            }
            SPA::Trim(procName);
            pos = procName.rfind('.');
            if (pos != CDBString::npos) {
                catalogSchema = procName.substr(0, pos);
                SPA::Trim(catalogSchema);
                procName = procName.substr(pos + 1);
                SPA::Trim(procName);
            }
            s = catalogSchema;
            if (s.size()) {
                s.push_back('.');
            }
            s += procName;
            pos = s_copy.find('(');
            if (pos != CDBString::npos) {
                s += s_copy.substr(pos);
            }
        }
        else {
            if (returned) {
                return u"";
            }
            s = s_copy;
        }
        const char16_t quote = '\'', slash = '\\', question = '?';
        bool b_slash = false, balanced = true;
        size_t len = s.size();
        for (size_t n = 0; n < len; ++n) {
            char16_t c = s[n];
            if (c == slash) {
                b_slash = true;
                continue;
            }
            if (c == quote && b_slash) {
                b_slash = false;
                continue; //ignore a quote if there is a slash ahead
            }
            b_slash = false;
            if (c == quote) {
                balanced = (!balanced);
                continue;
            }
            if (balanced) {
                if (c == question) {
                    s[n] = '@';
                    std::string str = std::to_string(parameters);
                    str = 'p' + str;
                    auto temp = SPA::Utilities::ToUTF16(str);
                    size_t length = temp.size();
                    s.insert(n + 1, temp);
                    n += length;
                    len += length;
                    ++parameters;
                }
            }
        }
        return std::move(s);
    }

    int CSqlBatch::Prepare(const char16_t* sql, CParameterInfoArray& params, unsigned int& parameters, SPA::UINT64 trans_decriptor) {
        m_sqlPrepare = Prepare(sql, parameters, m_returned, m_procName, m_catalogSchema);
        if (!m_sqlPrepare.size()) {
            return -1;
        }
        if (params.size() && params.size() != parameters) {
            return -1;
        }
        m_vParamInfo = std::move(params);
        m_outputs = 0;
        for (auto it = m_vParamInfo.cbegin(), end = m_vParamInfo.cend(); it != end; ++it)
        {
            if (it->Direction != tagParameterDirection::pdInput && it->Direction != tagParameterDirection::pdUnknown) {
                ++m_outputs;
            }
        }
        m_inputs = (unsigned short)(parameters - m_outputs);
        parameters = m_outputs;
        parameters <<= 16;
        parameters += m_inputs;
        return 0;
    }

    int CSqlBatch::ToString(const CDBVariantArray& vData, CDBString& s, std::vector<CDBString>& vP) {
        char param[16];
        s.clear();
        std::string str;
        size_t size = vData.size();
        for (size_t n = 0; n < size; ++n) {
            if (str.size()) {
                str.push_back(',');
            }
            const CDBVariant& v = vData[n];
#ifdef WIN32_64
            unsigned char bytes = (unsigned char)::sprintf_s(param, "@p%d", (int)n);
#else
            unsigned char bytes = (unsigned char)::sprintf_s(param, "@p%d", (int)n);
#endif
            str += param;
            vP.push_back(CDBString(param, param + strlen(param)));
            str.push_back(' ');
            switch (v.vt)
            {
            case VT_CLSID:
                str += "uniqueidentifier";
                break;
            case VT_I1:
            case VT_UI1:
                str += "tinyint";
                break;
            case VT_I2:
            case VT_UI2:
                str += "smallint";
                break;
            case VT_I4:
            case VT_UI4:
            case VT_INT:
            case VT_UINT:
                str += "int";
                break;
            case VT_I8:
            case VT_UI8:
                str += "bigint";
                break;
            case VT_R4:
                str += "real";
                break;
            case VT_R8:
                str += "float";
                break;
            case VT_BOOL:
                str += "bit";
                break;
            case VT_DATE:
            {
                SPA::UDateTime udt(v.ullVal);
                std::tm tm = udt.GetCTime();
                if (tm.tm_mday) {
                    str += "datetime2(6)";
                }
                else {
                    str += "time(6)";
                }
            }
                break;
            case VT_CY:
                str += "money";
                break;
            case VT_DECIMAL:
                if (v.decVal.Hi32) {
                    str += "decimal(28,";
                }
                else {
                    str += "decimal(19,";
                }
                str += std::to_string(v.decVal.scale);
                str.push_back(')');
                break;
            case (VT_ARRAY | VT_I1):
                str += "varchar(";
                {
                    unsigned int len = v.parray->rgsabound[0].cElements;
                    if (n > 8000) {
                        str += "max";
                    }
                    else {
                        if (!len) len = 1;
                        str += std::to_string(len);
                    }
                }
                str.push_back(')');
                break;
            case VT_BSTR:
                str += "nvarchar(";
                {
                    unsigned int len = ::SysStringLen(v.bstrVal);
                    if (n > 4000) {
                        str += "max";
                    }
                    else {
                        if (!len) len = 1;
                        str += std::to_string(len);
                    }
                }
                str.push_back(')');
                break;
            case (VT_ARRAY | VT_UI1):
                if (v.VtExt == tagVTExt::vteGuid) {
                    str += "uniqueidentifier";
                }
                else {
                    str += "varbinary(";
                    unsigned int len = v.parray->rgsabound[0].cElements;
                    if (n > 8000) {
                        str += "max";
                    }
                    else {
                        if (!len) len = 1;
                        str += std::to_string(len);
                    }
                    str.push_back(')');
                }
                break;
            case VT_NULL:
            case VT_EMPTY:
                str += "varchar(16)";
                break;
            default:
                return -1;
            }
        }
        s = SPA::Utilities::ToUTF16(str);
        return 0;
    }

    void CSqlBatch::ToParameter(const Collation& collation, const CDBVariant& v, const CDBString &p, SPA::CUQueue& buffer, unsigned char p_status) {
        tagDataType dt;
        unsigned char b_len = 0;
        unsigned char p_len = 0; //(unsigned char)p.size();
        buffer << p_len;
        //buffer.Push(p.c_str(), (unsigned int)p.size());
        buffer << p_status;
        switch (v.vt)
        {
        case VT_CY:
            dt = tagDataType::MONEYN;
            b_len = sizeof(CY); //length;
            buffer << dt << b_len << b_len << v.cyVal.Hi << v.cyVal.Lo;
            break;
        case VT_DATE:
        {
            unsigned int us;
            SPA::UDateTime udt(v.ullVal);
            std::tm tm = udt.GetCTime(&us);
            if (tm.tm_mday) {
                dt = tagDataType::DATETIME2N;
                p_len = 6; //scale
                b_len = 8; //length;
                buffer << dt << p_len << b_len;
                int dt = tds::ToTdsJDN(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
                Date d;
                d.Low = (unsigned short)(dt & USHORT_NULL_LEN);
                d.High = (char)((dt >> 16) & 0xff);
                SPA::UINT64 time = tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;
                time *= 1000000;
                time += us;
                unsigned int low = (unsigned int)(time & UINT_NULL_LEN);
                time >>= 32;
                unsigned char high = (unsigned char)(time & 0xff);
                buffer << low << high << d;
            }
            else {
                dt = tagDataType::TIMEN;
                p_len = 6; //scale
                b_len = 5; //length;
                buffer << dt << p_len << b_len;
                SPA::UINT64 time = tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;
                time *= 1000000;
                time += us;
                unsigned int low = (unsigned int)(time & UINT_NULL_LEN);
                time >>= 32;
                unsigned char high = (unsigned char)(time & 0xff);
                buffer << low << high;
            }
        }
            break;
        case VT_DECIMAL:
            dt = tagDataType::DECIMAL;
            b_len = v.decVal.Hi32 ? 13 : 9; //max length;
            p_len = v.decVal.Hi32 ? 28 : 19; //precision;
            buffer << dt << b_len << p_len << v.decVal.scale << b_len;
            p_len = v.decVal.sign ? 0 : 1; //sign
            buffer << p_len << v.decVal.Lo64;
            if (v.decVal.Hi32) {
                buffer << v.decVal.Hi32;
            }
            break;
        case SPA::VT_XML:
        case VT_BSTR:
            dt = tagDataType::NVARCHAR;
            {
                unsigned int len = SysStringLen(v.bstrVal);
                len <<= 1;
                if (len <= 8000) {
                    buffer << dt << (unsigned short)len << collation << (unsigned short)len;
                    buffer.Push(v.bstrVal, len >> 1);
                }
                else {

                }
            }
            break;
        case VT_R4:
            dt = tagDataType::FLTN;
            b_len = sizeof(float);
            buffer << dt << b_len << b_len << v.fltVal;
            break;
        case VT_R8:
            dt = tagDataType::FLTN;
            b_len = sizeof(double);
            buffer << dt << b_len << b_len << v.dblVal;
            break;
        case VT_I4:
        case VT_UI4:
        case VT_INT:
        case VT_UINT:
            dt = tagDataType::INTN;
            b_len = sizeof(int);
            buffer << dt << b_len << b_len << v.intVal;
            break;
        case VT_I8:
        case VT_UI8:
            dt = tagDataType::INTN;
            b_len = sizeof(SPA::INT64);
            buffer << dt << b_len << b_len << v.llVal;
            break;
        case VT_I2:
        case VT_UI2:
            dt = tagDataType::INTN;
            b_len = sizeof(short);
            buffer << dt << b_len << b_len << v.iVal;
            break;
        case VT_BOOL:
            dt = tagDataType::INTN;
            b_len = sizeof(unsigned char);
            {
                unsigned char c = v.boolVal ? 1 : 0;
                buffer << dt << b_len << b_len << c;
            }
            break;
        case VT_I1:
        case VT_UI1:
            dt = tagDataType::INTN;
            b_len = sizeof(unsigned char);
            buffer << dt << b_len << b_len << v.bVal;
            break;
        case VT_NULL:
        case VT_EMPTY:
            dt = tagDataType::VARCHAR;
            {
                unsigned short len = 16;
                buffer << dt << len << collation;
                len = USHORT_NULL_LEN;
                buffer << len;
            }
            break;
        case (VT_I1|VT_ARRAY):
            dt = tagDataType::VARCHAR;
            {
                const char* s;
                unsigned int len = v.parray->rgsabound[0].cElements;
                SafeArrayAccessData(v.parray, (void**)&s);
                if (len <= 8000) {
                    buffer << dt << (unsigned short)len << collation << (unsigned short)len;
                    buffer.Push(s, len);
                }
                else {

                }
                SafeArrayUnaccessData(v.parray);
            }
            break;
        case (VT_UI1 | VT_ARRAY):
        {
            const unsigned char* s;
            unsigned int len = v.parray->rgsabound[0].cElements;
            SafeArrayAccessData(v.parray, (void**)&s);
            if (v.VtExt == tagVTExt::vteGuid) {
                dt = tagDataType::UNIQUEIDENTIFIER;
                b_len = sizeof(GUID);
                buffer << dt << b_len << b_len;
                buffer.Push(s, b_len);
            }
            else {
                dt = tagDataType::VARBINARY;
                if (len <= 8000) {
                    buffer << dt << (unsigned short)len << (unsigned short)len;
                    buffer.Push(s, len);
                }
                else {

                }
            }
            SafeArrayUnaccessData(v.parray);
        }
            break;
        default:
            break;
        }
    }

    int CSqlBatch::SendMessage(const SqlLogin& rec, FeatureExtension requestedFeatures) {
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

        SPA::CScopeUQueue sbFeature, sbData;

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
        SPA::CScopeUQueue sb;
        PacketHeader ph(tagPacketType::ptLogin7, 1);
        sb << ph;
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
        PacketHeader* pHeader = (PacketHeader*)sb->GetBuffer();
        pHeader->Length = ChangeEndian((Packet_Length)sb->GetSize());
        return m_channel.Send(sb->GetBuffer(), sb->GetSize(), this);
    }

    int CSqlBatch::SendMessage(CDBVariantArray& vParam, SPA::UINT64 trans_decriptor) {
        assert(vParam.size());
        assert(m_sqlPrepare.size());
        unsigned int parameters = m_inputs + m_outputs;
        assert(parameters);
        assert(0 == (vParam.size() % parameters));
        size_t cycles = vParam.size() / parameters;
        CDBString sql = m_sqlPrepare;
        for (size_t n = 1; n < cycles; ++n) {
            sql.push_back(';');
            sql += m_sqlPrepare;
        }
        SPA::CScopeUQueue sb;
        //Query packet
        TransactionDescriptor td(trans_decriptor);
        sb << td;
        if (m_procName.size()) {

        }
        else {
            unsigned short p_name_length = USHORT_NULL_LEN;
            sb << p_name_length;
            unsigned short s_proc_id = 10; //sp_executesql
            sb << s_proc_id;
        }
        unsigned short optionFlags = 0x2; //no meta data
        sb << optionFlags;

        unsigned char name_len = 0;
        sb << name_len;
        unsigned char status = 0;
        sb << status;
        tagDataType dt = tagDataType::NVARCHAR;
        unsigned short max_len = (unsigned short)(sql.size() << 1);
        Collation collation;
        collation.CodePage = (unsigned short)GetSystemDefaultLCID();
        collation.Flags.fIgnoreCase = 1;
        collation.Flags.fIgnoreWidth = 1;
        collation.Flags.fIgnoreKana = 1;
        collation.CharsetId = 52;

        sb << dt << max_len << collation << max_len;
        sb->Push(sql.c_str(), (unsigned int)sql.size());

        //
        name_len = 0;
        status = 0;
        sb << name_len << status << dt;
        CDBString p;
        std::vector<CDBString> vP;
        int res = ToString(vParam, p, vP);
        size_t len = p.size();
        max_len = (unsigned short)(len << 1);
        sb << max_len << collation << max_len;
        sb->Push(p.c_str(), (unsigned int)len);

        //
        len = vParam.size();
        for (size_t n = 0; n < len; ++n) {
            ToParameter(collation, vParam[n], vP[n], *sb);
        }

        PacketHeader ph(tagPacketType::ptRpc, 1);
        ph.Length = (Packet_Length)(sb->GetSize() + sizeof(ph));
        ph.Length = ChangeEndian(ph.Length);

        SPA::CScopeUQueue sbEnd;
        sbEnd << ph;
        sbEnd->Push(sb->GetBuffer(), sb->GetSize());
        return m_channel.Send(sbEnd->GetBuffer(), sbEnd->GetSize(), this);
    }

    int CSqlBatch::SendMessage(const char16_t *sql, SPA::UINT64 trans_decriptor) {
        Reset();
        SPA::CScopeUQueue sb;
        PacketHeader ph(tagPacketType::ptBatch, 1);
        //Query packet
		TransactionDescriptor td(trans_decriptor);
        sb << ph << td;
        sb->Push((const unsigned char*) sql, (unsigned int) (SPA::GetLen(sql) << 1));
        PacketHeader* pHeader = (PacketHeader*)sb->GetBuffer();
        pHeader->Length = ChangeEndian((Packet_Length)sb->GetSize());
        return m_channel.Send(sb->GetBuffer(), sb->GetSize(), this);
    }

	bool CSqlBatch::ParseDoneInProc() {
		if (m_buffer.GetSize() >= sizeof(m_dip)) {
			m_buffer >> m_dip;
			m_tt = tagTokenType::ttZero;
			return true;
		}
		return false;
	}

	bool CSqlBatch::ParseReturnStatus() {
		if (m_buffer.GetSize() >= sizeof(m_rs)) {
			m_buffer >> m_rs;
			m_tt = tagTokenType::ttZero;
			return true;
		}
		return false;
	}

    bool CSqlBatch::ParseCollation(CollationChange& cc) {
        if (m_buffer.GetSize() >= sizeof(cc) + sizeof(unsigned char) + sizeof(unsigned char)) {
            unsigned char b;
            m_buffer >> b;
            assert(b == 5);
            m_buffer >> cc.NewValue;
            m_buffer >> b;
            if (b) {
                m_buffer >> cc.OldValue;
            }
            m_tt = tagTokenType::ttZero;
            return true;
        }
        return false;
    }

    void CSqlBatch::ParseStringChange(tagEnvchangeType type, StringEventChange& sec) {
        unsigned char b;
        m_buffer >> b;
        sec.Type = type;
        const char16_t* str = (const char16_t*)m_buffer.GetBuffer();
        sec.NewValue.assign(str, str + b);
        m_buffer.Pop(((unsigned int)b) << 1);
        m_buffer >> b;
        if (b) {
            str = (const char16_t*)m_buffer.GetBuffer();
            sec.OldValue.assign(str, str + b);
            m_buffer.Pop(((unsigned int)b) << 1);
        }
    }

    bool CSqlBatch::ParseErrorInfo() {
        if (m_buffer.GetSize() > 2) {
            unsigned short len = *(unsigned short*)m_buffer.GetBuffer();
            if (len + sizeof(len) <= m_buffer.GetSize()) {
                TokenInfo ti;
                m_buffer >> len >> ti.SQLErrorNumber >> ti.State >> ti.Class;
                m_buffer >> len;
                const char16_t* str = (const char16_t*)m_buffer.GetBuffer();
                ti.ErrorMessage.assign(str, str + len);
                m_buffer.Pop(((unsigned int)len) << 1);
                unsigned char byteLen;
                m_buffer >> byteLen;
                str = (const char16_t*)m_buffer.GetBuffer();
                ti.ServerName.assign(str, str + byteLen);
                m_buffer.Pop(((unsigned int)byteLen) << 1);
                m_buffer >> ti.ProcessNameLength >> ti.LineNumber;
                m_vInfo.push_back(ti);
                m_tt = tagTokenType::ttZero;
                return true;
            }
        }
        return false;
    }

    bool CSqlBatch::ParseMeta() {
        do {
            if (!m_cols) {
                if (m_buffer.GetSize() < 2) {
                    return false;
                }
                m_vCol.clear();
                m_buffer >> m_cols;
            }
            if (m_buffer.GetSize() < 10) {
                return false;
            }
            unsigned char col_len;
            unsigned short col_name_len;

            MetaInfoHeader cmh;
            CDBColumnInfo ci;
            MetaInfoHeader *mih = (MetaInfoHeader*) m_buffer.GetBuffer();
            switch (mih->SqlType) {
                case tagDataType::SMALLINT:
                case tagDataType::TINYINT:
                case tagDataType::DATEN:
                case tagDataType::DATETIME:
                case tagDataType::INT:
                case tagDataType::BIGINT:
                case tagDataType::BIT:
                case tagDataType::REAL:
                case tagDataType::FLOAT:
                case tagDataType::SMALLMONEY:
                case tagDataType::MONEY:
                {
                    col_name_len = *m_buffer.GetBuffer(sizeof (MetaInfoHeader));
                    col_name_len <<= 1;
                    if (col_name_len + sizeof (MetaInfoHeader) + sizeof (unsigned char) > m_buffer.GetSize()) {
                        return false;
                    }
                    m_buffer >> cmh >> col_len;
                }
                    break;
                case tagDataType::IMAGE:
                {
                    unsigned char tbl_name_parts;
                    unsigned int max_len;
                    unsigned short tbl_name_len, Offset = sizeof (MetaInfoHeader) + sizeof (max_len) + sizeof (tbl_name_parts);
                    if (Offset >= m_buffer.GetSize()) {
                        return false;
                    }
                    tbl_name_len = *(unsigned short*) m_buffer.GetBuffer(Offset);
                    tbl_name_len <<= 1;
                    Offset += (sizeof (tbl_name_len) + tbl_name_len);
                    if (Offset >= m_buffer.GetSize()) {
                        return false;
                    }
                    col_name_len = *m_buffer.GetBuffer(Offset);
                    col_name_len <<= 1;
                    if (col_name_len + Offset + sizeof (col_len) > m_buffer.GetSize()) {
                        return false;
                    }
                    m_buffer >> cmh >> max_len >> tbl_name_parts >> tbl_name_len;
                    ci.ColumnSize = max_len;
                    if (m_meta) {
                        ci.TablePath.assign((const char16_t*) m_buffer.GetBuffer(), tbl_name_len);
                    }
                    tbl_name_len <<= 1;
                    m_buffer.Pop(tbl_name_len);
                    m_buffer >> col_len;

                }
                    break;
                case tagDataType::NTEXT:
                case tagDataType::TEXT:
                {
                    unsigned char tbl_name_parts;
                    unsigned int max_len;
                    unsigned short tbl_name_len, Offset = sizeof (MetaInfoHeader) + sizeof (max_len) + sizeof (Collation) + sizeof (tbl_name_parts);
                    if (Offset >= m_buffer.GetSize()) {
                        return false;
                    }
                    tbl_name_len = *(unsigned short*) m_buffer.GetBuffer(Offset);
                    tbl_name_len <<= 1;
                    Offset += (sizeof (tbl_name_len) + tbl_name_len);
                    if (Offset >= m_buffer.GetSize()) {
                        return false;
                    }
                    col_name_len = *m_buffer.GetBuffer(Offset);
                    col_name_len <<= 1;
                    if (col_name_len + Offset + sizeof (col_len) > m_buffer.GetSize()) {
                        return false;
                    }
                    Collation collation;
                    m_buffer >> cmh >> max_len >> collation >> tbl_name_parts >> tbl_name_len;
                    ci.ColumnSize = max_len;
                    if (m_meta) {
                        if (collation != m_collation) {
                            ci.Collation = collation.GetString();
                            m_collation = collation;
                        }
                        ci.TablePath.assign((const char16_t*) m_buffer.GetBuffer(), tbl_name_len);
                    }
                    tbl_name_len <<= 1;
                    m_buffer.Pop(tbl_name_len);
                    m_buffer >> col_len;

                }
                    break;
                case tagDataType::CHAR:
                case tagDataType::VARCHAR:
                case tagDataType::NCHAR:
                case tagDataType::NVARCHAR:
                {
                    unsigned short col_chars;
                    constexpr unsigned short Offset = sizeof (MetaInfoHeader) + sizeof (col_chars) + sizeof (Collation);
                    col_name_len = *m_buffer.GetBuffer(Offset);
                    col_name_len <<= 1;
                    if (col_name_len + Offset + sizeof (col_len) > m_buffer.GetSize()) {
                        return false;
                    }
                    Collation collation;
                    m_buffer >> cmh >> col_chars >> collation >> col_len;
                    if ((cmh.SqlType == tagDataType::NCHAR || cmh.SqlType == tagDataType::NVARCHAR) && col_chars != 0xffff) {
                        col_chars >>= 1;
                    }
                    ci.ColumnSize = col_chars;
                    if (m_meta) {
                        if (collation != m_collation) {
                            ci.Collation = collation.GetString();
                            m_collation = collation;
                        }
                    }
                }
                    break;
                case tagDataType::BINARY:
                case tagDataType::VARBINARY:
                {
                    unsigned short col_chars;
                    constexpr unsigned short Offset = sizeof (MetaInfoHeader) + sizeof (col_chars);
                    col_name_len = *m_buffer.GetBuffer(Offset);
                    col_name_len <<= 1;
                    if (col_name_len + Offset + sizeof (col_len) > m_buffer.GetSize()) {
                        return false;
                    }
                    m_buffer >> cmh >> col_chars >> col_len;
                    ci.ColumnSize = col_chars;
                }
                    break;
                case tagDataType::INTN:
                case tagDataType::DATETIME2N: //datetime2
                case tagDataType::DATETIMEOFFSETN:
                case tagDataType::DATETIMN: //smalldatetime
                case tagDataType::TIMEN:
                case tagDataType::MONEYN:
                case tagDataType::UNIQUEIDENTIFIER:
                {
                    unsigned char bytes;
                    constexpr unsigned short Offset = sizeof (MetaInfoHeader) + sizeof (bytes);
                    col_name_len = *m_buffer.GetBuffer(Offset);
                    col_name_len <<= 1;
                    if (col_name_len + Offset + sizeof (col_len) > m_buffer.GetSize()) {
                        return false;
                    }
                    m_buffer >> cmh >> bytes >> col_len;
                    if (cmh.SqlType == tagDataType::DATETIME2N || cmh.SqlType == tagDataType::DATETIMEOFFSETN || cmh.SqlType == tagDataType::TIMEN) {
                        ci.Scale = bytes;
                    } else {
                        ci.ColumnSize = bytes;
                    }
                }
                    break;
                case tagDataType::UDT:
                {
                    unsigned short max_bytes;
                    unsigned short Offset = sizeof (MetaInfoHeader) + sizeof (max_bytes);
                    if (Offset + sizeof (col_len) > m_buffer.GetSize()) {
                        return false;
                    }
                    col_name_len = col_len = *m_buffer.GetBuffer(Offset);
                    col_name_len <<= 1;
                    if (col_name_len + Offset + sizeof (col_len) > m_buffer.GetSize()) {
                        return false;
                    }
                    Offset += (col_name_len + sizeof (col_len));
                    unsigned short pos = *m_buffer.GetBuffer(Offset); //schema
                    Offset += (sizeof (col_len) + (pos << 1));
                    if (m_buffer.GetSize() <= Offset) {
                        return false;
                    }
                    pos = *m_buffer.GetBuffer(Offset); //type name
                    Offset += (sizeof (col_len) + (pos << 1));
                    if (m_buffer.GetSize() <= Offset) {
                        return false;
                    }
                    unsigned short type_len = *(unsigned short*) m_buffer.GetBuffer(Offset); //assembly
                    Offset += (sizeof (type_len) + (type_len << 1));
                    if (m_buffer.GetSize() <= Offset) {
                        return false;
                    }
                    pos = *m_buffer.GetBuffer(Offset); //column name
                    Offset += (sizeof (col_len) + (pos << 1));
                    if (m_buffer.GetSize() < Offset) {
                        return false;
                    }

                    m_buffer >> cmh >> max_bytes >> col_len;
                    ci.ColumnSize = max_bytes;
                    if (m_meta) {
                        ci.DBPath.assign((const char16_t*) m_buffer.GetBuffer(), col_len);
                    }
                    m_buffer.Pop(col_name_len);
                    CDBString schema, assembly;
                    m_buffer >> col_len;
                    col_name_len = col_len;
                    col_name_len <<= 1;
                    if (m_meta) {
                        schema.assign((const char16_t*) m_buffer.GetBuffer(), col_len);
                    }
                    m_buffer.Pop(col_name_len);

                    m_buffer >> col_len;
                    col_name_len = col_len;
                    col_name_len <<= 1;
                    ci.DeclaredType.assign((const char16_t*) m_buffer.GetBuffer(), col_len);
                    m_buffer.Pop(col_name_len);

                    m_buffer >> type_len; //2 bytes
                    if (m_meta) {
                        assembly.assign((const char16_t*) m_buffer.GetBuffer(), type_len);
                    }
                    type_len <<= 1;
                    m_buffer.Pop(type_len);
                    if (m_meta) {
                        ci.Collation = schema + u'+' + assembly;
                    }
                    m_buffer >> col_len;
                    col_name_len = col_len;
                    col_name_len <<= 1;
                }
                    break;
                case tagDataType::XML:
                {
                    unsigned char schema = *m_buffer.GetBuffer(sizeof (MetaInfoHeader));
                    unsigned short Offset = sizeof (MetaInfoHeader) + sizeof (schema);
                    if (Offset + sizeof (col_len) > m_buffer.GetSize()) {
                        return false;
                    }
                    col_name_len = col_len = *m_buffer.GetBuffer(Offset);
                    col_name_len <<= 1;
                    if (col_name_len + Offset + sizeof (col_len) > m_buffer.GetSize()) {
                        return false;
                    }
                    if (schema) {
                        Offset += (col_name_len + sizeof (col_len));
                        unsigned short pos = *m_buffer.GetBuffer(Offset); //schema
                        Offset += (sizeof (col_len) + (pos << 1));
                        if (m_buffer.GetSize() <= Offset) {
                            return false;
                        }
                        unsigned short type_len = *(unsigned short*) m_buffer.GetBuffer(Offset); //collection
                        Offset += (sizeof (type_len) + (type_len << 1));
                        if (m_buffer.GetSize() <= Offset) {
                            return false;
                        }
                        pos = *m_buffer.GetBuffer(Offset); //column name
                        Offset += (sizeof (col_len) + (pos << 1));
                        if (m_buffer.GetSize() < Offset) {
                            return false;
                        }
                        m_buffer >> cmh >> schema >> col_len;
                        if (m_meta) {
                            ci.DBPath.assign((const char16_t*) m_buffer.GetBuffer(), col_len);
                        }
                        m_buffer.Pop(col_name_len);
                        CDBString ownerName, collection;
                        m_buffer >> col_len;
                        col_name_len = col_len;
                        col_name_len <<= 1;
                        if (m_meta) {
                            ownerName.assign((const char16_t*) m_buffer.GetBuffer(), col_len);
                        }
                        m_buffer.Pop(col_name_len);

                        m_buffer >> type_len; //2 bytes
                        if (m_meta) {
                            collection.assign((const char16_t*) m_buffer.GetBuffer(), type_len);
                        }
                        type_len <<= 1;
                        m_buffer.Pop(type_len);

                        m_buffer >> col_len;
                        col_name_len = col_len;
                        col_name_len <<= 1;
                        if (m_meta) {
                            ci.Collation = ownerName + u'.' + collection;
                        }
                    } else {
                        m_buffer >> cmh >> schema >> col_len;
                    }
                }
                    break;
                case tagDataType::SQL_VARIANT:
                {
                    unsigned int bytes;
                    col_name_len = *m_buffer.GetBuffer(sizeof (MetaInfoHeader) + sizeof (bytes));
                    col_name_len <<= 1;
                    if (col_name_len + sizeof (MetaInfoHeader) + sizeof (bytes) + sizeof (col_len) > m_buffer.GetSize()) {
                        return false;
                    }
                    m_buffer >> cmh >> bytes >> col_len;
                    ci.ColumnSize = bytes;
                }
                    break;
                case tagDataType::NUMERIC:
                case tagDataType::DECIMAL:
                {
                    unsigned char type, precision, scale;
                    constexpr unsigned short Offset = sizeof (MetaInfoHeader) + sizeof (type) + sizeof (precision) + sizeof (scale);
                    col_name_len = *m_buffer.GetBuffer(Offset);
                    col_name_len <<= 1;
                    if (col_name_len + Offset + sizeof (unsigned char) > m_buffer.GetSize()) {
                        return false;
                    }
                    m_buffer >> cmh >> type >> precision >> scale >> col_len;
                    ci.ColumnSize = type;
                    ci.Precision = precision;
                    ci.Scale = scale;
                }
                    break;
                default:
                    assert(false);
                    break;
            }
            if (m_meta) {
                ci.DisplayName.assign((const char16_t*) m_buffer.GetBuffer(), col_len);
            }
            m_buffer.Pop(col_name_len);
            ci.DataType = GetVarType(cmh.SqlType, ci.ColumnSize);
            ci.Flags = ToUDBFlags(cmh.Flags);
            if (cmh.SqlType == tagDataType::XML) {
                ci.Flags |= CDBColumnInfo::FLAG_XML;
            }
            if (!m_meta) {
                m_vCol.push_back(ci);
            } else {
                if (!ci.DeclaredType.size()) {
                    ci.DeclaredType = GetSqlDeclaredType(cmh.SqlType, ci.ColumnSize);
                }
                m_vCol.push_back(std::move(ci));
            }
            m_vDT.push_back(mih->SqlType);
            --m_cols;
        } while (m_cols);
        m_tt = tagTokenType::ttZero;
        size_t count = (m_vCol.size() >> 3) + ((m_vCol.size() % 8) ? 1 : 0);
        m_vNull.assign(count, 0);
        m_posCol = INVALID_COL;
        return true;
    }

    bool CSqlBatch::ParseVariant(CDBColumnInfo * cinfo) {
        unsigned int bytes;
        unsigned char prop_bytes;
        tagDataType dt;
        m_buffer >> bytes >> dt >> prop_bytes;
        bytes -= (prop_bytes + sizeof (prop_bytes) + sizeof (dt));
        switch (dt) {
            case tagDataType::BINARY:
            case tagDataType::VARBINARY:
                assert(prop_bytes == 2);
                if (prop_bytes > 2) {
                    m_buffer.Pop(prop_bytes - 2, 2); //skip unknown properties
                }
                return ParseData(dt, cinfo);
            case tagDataType::CHAR:
            case tagDataType::VARCHAR:
            case tagDataType::NCHAR:
            case tagDataType::NVARCHAR:
            {
                assert(prop_bytes == 7);
                Collation collation;
                m_buffer >> collation; //skip collation
                prop_bytes -= sizeof (Collation);
                if (prop_bytes > 2) {
                    m_buffer.Pop(prop_bytes - 2, 2); //skip unknown properties
                }
                unsigned short max_len;
                m_buffer >> max_len;
                max_len = (unsigned short) bytes;
                m_buffer.Insert(&max_len, 0);
                return ParseData(dt, cinfo);
            }
                break;
            case tagDataType::TIMEN:
            case tagDataType::DATETIME2N:
            case tagDataType::DATETIMEOFFSETN:
            {
                assert(prop_bytes == 1);
                unsigned char scale;
                m_buffer >> scale;
                --prop_bytes;
                if (prop_bytes) {
                    m_buffer.Pop(prop_bytes); //skip unknown properties
                }
                return ParseData(dt, (unsigned char) bytes, scale);
            }
                break;
            case tagDataType::DECIMAL:
            case tagDataType::NUMERIC:
            {
                unsigned char precision, scale;
                assert(prop_bytes == 2);
                m_buffer >> precision >> scale;
                prop_bytes -= 2;
                if (prop_bytes) {
                    m_buffer.Pop(prop_bytes); //skip unknwon properties
                }
                return ParseData(dt, (unsigned char) bytes, scale);
            }
                break;
            case tagDataType::DATEN:
            case tagDataType::DATETIM4:
            case tagDataType::DATETIME:
            case tagDataType::BITN:
            case tagDataType::MONEYN:
                return ParseData(dt, (unsigned char) bytes, 0);
            default:
                return ParseData(dt, (unsigned char) bytes, 0);
        }
        return true;
    }

    bool CSqlBatch::ParseRow() {
        bool done = IsDone();
        unsigned short cols = (unsigned short) m_vCol.size();
        CDBColumnInfo *cinfo = m_vCol.data();
        tagDataType *pdt = m_vDT.data();
        if (m_posCol == INVALID_COL) {
            m_posCol = 0;
        }
        cinfo += m_posCol;
        for (unsigned short n = m_posCol; n < cols; ++n, ++cinfo, ++m_posCol) {
            if (!done) {
                if (m_buffer.GetSize() <= sizeof (PacketHeader) + sizeof (m_Done) + sizeof (tagTokenType)) {
                    /*UINT64 len = *(UINT64*)m_buffer.GetBuffer();
                    unsigned int size = *(unsigned int*)m_buffer.GetBuffer(sizeof(UINT64));*/
                    return false;
                }
            }
            tagDataType dt = pdt[n];
            unsigned char nullable = (!(cinfo->Flags & CDBColumnInfo::FLAG_NOT_NULL));
            switch (dt) {
                case tagDataType::SQL_VARIANT:
                    if (!ParseVariant(cinfo)) {
                        return false;
                    }
                    break;
                case tagDataType::NTEXT:
                    if (cinfo->ColumnSize == MAX_NTEXT_LEN) {
                        unsigned int len = *(unsigned int*) m_buffer.GetBuffer();
                        if (len == UINT_NULL_LEN) {
                            m_out << (VARTYPE) VT_NULL;
                            m_buffer.Pop(4);
                            continue;
                        }
                    } else {
                        assert(false);
                    }
                    if (!ParseData(dt, cinfo)) {
                        return false;
                    }
                    break;
                case tagDataType::TEXT:
                case tagDataType::IMAGE:
                    if (cinfo->ColumnSize == MAX_IMAGE_TEXT_LEN) {
                        unsigned int len = *(unsigned int*) m_buffer.GetBuffer();
                        if (len == UINT_NULL_LEN) {
                            m_out << (VARTYPE) VT_NULL;
                            m_buffer.Pop(4);
                            continue;
                        }
                    } else {
                        assert(false);
                    }
                    if (!ParseData(dt, cinfo)) {
                        return false;
                    }
                    break;
                case tagDataType::XML:
                {
                    unsigned int len = *(unsigned int*) m_buffer.GetBuffer();
                    if (len == UINT_NULL_LEN) {
                        m_out << (VARTYPE) VT_NULL;
                        m_buffer.Pop(4);
                        continue;
                    }
                    if (!ParseData(dt, cinfo)) {
                        return false;
                    }
                }
                    break;
                case tagDataType::UDT:
                {
                    unsigned int len = *(unsigned int*) m_buffer.GetBuffer();
                    if (len == UINT_NULL_LEN) {
                        m_out << (VARTYPE) VT_NULL;
                        m_buffer.Pop(4);
                        continue;
                    }
                    if (!ParseData(dt, cinfo)) {
                        return false;
                    }
                }
                    break;
                case tagDataType::NVARCHAR:
                case tagDataType::NCHAR:
                case tagDataType::BINARY:
                case tagDataType::CHAR:
                case tagDataType::VARBINARY:
                case tagDataType::VARCHAR:
                    if (cinfo->ColumnSize == VAR_MAX) {
                        unsigned int len = *(unsigned int*) m_buffer.GetBuffer();
                        if (len == UINT_NULL_LEN) {
                            m_out << (VARTYPE) VT_NULL;
                            m_buffer.Pop(4);
                            continue;
                        }
                    } else {
                        unsigned short len = *(unsigned short*) m_buffer.GetBuffer();
                        if (len == 0xffff) {
                            m_out << (VARTYPE) VT_NULL;
                            m_buffer.Pop(2);
                            continue;
                        }
                    }
                    if (!ParseData(dt, cinfo)) {
                        return false;
                    }
                    break;
                case tagDataType::DECIMAL:
                case tagDataType::NUMERIC:
                case tagDataType::DATETIME2N:
                case tagDataType::DATETIMN:
                case tagDataType::TIMEN:
                case tagDataType::INTN:
                case tagDataType::DATEN:
                case tagDataType::DATETIMEOFFSETN:
                case tagDataType::MONEYN:
                case tagDataType::UNIQUEIDENTIFIER:
                {
                    unsigned char bytes;
                    m_buffer >> bytes;
                    if (!bytes) {
                        m_out << (VARTYPE) VT_NULL;
                        continue;
                    } else if (!ParseData(dt, bytes, cinfo->Scale)) {
                        return false;
                    }
                }
                    break;
                default:
                {
                    unsigned char bytes = 0;
                    if (nullable) {
                        m_buffer >> bytes;
                        if (!bytes) {
                            m_out << (VARTYPE) VT_NULL;
                            continue;
                        }
                    }
                    if (!ParseData(dt, bytes, cinfo->Scale)) {
                        return false;
                    }
                }
                    break;
            }
        }
        m_posCol = INVALID_COL;
        m_tt = tagTokenType::ttZero;
        return true;
    }

    bool CSqlBatch::ParseData(tagDataType dt, unsigned char bytes, unsigned char scale) {
        switch (dt) {
            case tagDataType::INTN:
            {
                switch (bytes) {
                    case 1:
                    {
                        unsigned char n;
                        m_buffer >> n;
                        m_out << (VARTYPE) VT_UI1 << n;
                    }
                        break;
                    case 2:
                    {
                        short n;
                        m_buffer >> n;
                        m_out << (VARTYPE) VT_I2 << n;
                    }
                        break;
                    case 4:
                    {
                        int n;
                        m_buffer >> n;
                        m_out << (VARTYPE) VT_I4 << n;
                    }
                        break;
                    case 8:
                    {
                        INT64 n;
                        m_buffer >> n;
                        m_out << (VARTYPE) VT_I8 << n;
                    }
                        break;
                    default:
                        assert(false);
                        break;
                }
            }
                break;
            case tagDataType::MONEY:
                bytes = 8; //use MONEYN
            case tagDataType::MONEYN:
            {
                switch (bytes) {
                    case 4:
                    {
                        int money;
                        m_buffer >> money;
                        m_out << (VARTYPE) VT_I4 << money;
                    }
                        break;
                    case 8:
                    {
                        unsigned int high, low;
                        m_buffer >> high >> low;
                        INT64 money = high;
                        money <<= 32;
                        money += low;
                        m_out << (VARTYPE) VT_I8 << money;
                    }
                        break;
                    default:
                        assert(false);
                        break;
                }
            }
                break;
            case tagDataType::UNIQUEIDENTIFIER:
            {
                GUID guid;
                m_buffer >> guid;
                m_out << (VARTYPE) VT_CLSID << guid;
            }
                break;
            case tagDataType::BIT:
            {
                unsigned char b;
                m_buffer >> b;
                VARIANT_BOOL vb = b ? VARIANT_TRUE : VARIANT_FALSE;
                m_out << (VARTYPE) VT_BOOL << vb;
            }
                break;
            case tagDataType::TINYINT:
                m_out << (VARTYPE) VT_UI1;
                m_out.Push(m_buffer.GetBuffer(), 1);
                m_buffer.Pop(1);
                break;
            case tagDataType::SMALLINT:
                m_out << (VARTYPE) VT_I2;
                m_out.Push(m_buffer.GetBuffer(), 2);
                m_buffer.Pop(2);
                break;
            case tagDataType::INT:
                m_out << (VARTYPE) VT_I4;
                m_out.Push(m_buffer.GetBuffer(), 4);
                m_buffer.Pop(4);
                break;
            case tagDataType::BIGINT:
                m_out << (VARTYPE) VT_I8;
                m_out.Push(m_buffer.GetBuffer(), 8);
                m_buffer.Pop(8);
                break;
            case tagDataType::DECIMAL:
            case tagDataType::NUMERIC:
            {
                DECIMAL dec;
                memset(&dec, 0, sizeof (dec));
                unsigned char sign;
                m_buffer >> sign;
                if (!sign) {
                    dec.sign = 0x80;
                }
                dec.scale = scale;
                --bytes;
                switch (bytes) {
                    case 4:
                        m_buffer >> dec.Lo32;
                        break;
                    case 8:
                        m_buffer >> dec.Lo64;
                        break;
                    case 12:
                        m_buffer >> dec.Lo64 >> dec.Hi32;
                        break;
                    case 16:
                        m_buffer >> dec.Lo64 >> dec.Hi32;
                        m_buffer.Pop(4); //data truncated
                        break;
                    default:
                        assert(false);
                        break;
                }
                m_out << (VARTYPE) VT_DECIMAL << dec;
            }
                break;
            case tagDataType::DATETIME:
            {
				unsigned int us;
                DateTime dt;
                m_buffer >> dt;
				std::tm tm;
				ToDateTime(dt, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
				SPA::UDateTime udt(tm, us);
                m_out << (VARTYPE) VT_DATE << udt.time;
            }
                break;
            case tagDataType::DATETIMEOFFSETN:
            {
				short offset;
				unsigned int us;
				std::tm tm;
				Date dt;
				assert(scale <= 7);
				if (scale >= 5 && scale <= 7) {
					unsigned int low;
					unsigned char high;
					m_buffer >> low >> high;
					SPA::UINT64 time = high;
					time <<= 32;
					time += low;
					m_buffer >> dt;
					ToDateTime(dt, time, scale, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
				}
				else if (scale == 3 || scale == 4) {
					unsigned int time;
					m_buffer >> time >> dt;
					ToDateTime(dt, time, scale, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
				}
				else {
					unsigned short low;
					unsigned char high;
					m_buffer >> low >> high >> dt;
					unsigned int time = high;
					time <<= 16;
					time += low;
					ToDateTime(dt, time, scale, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
				}
				char datetime[32] = { 0 };
				tm.tm_year -= 1900;
				tm.tm_mon -= 1;
				SPA::UDateTime udt(tm, us);
				udt.ToDBString(datetime, sizeof(datetime));
				m_buffer >> offset;
				bool neg = false;
				if (offset < 0) {
					offset = (-offset);
					neg = true;
				}
				char os[8] = { 0 };
#ifdef WIN32_64
				int len = ::sprintf_s(os, sizeof(os), " %s%02d:%02d", neg ? "-" : "+", offset / 60, (offset % 60));
#else
				int len ::sprintf(os, " %s%02d:%02d", neg ? "-" : "+", offset / 60, (offset % 60));
#endif
				unsigned int len0 = (unsigned int) ::strlen(datetime);
				m_out << (VARTYPE)(VT_ARRAY | VT_I1);
				m_out << (len0 + (unsigned int)len);
				m_out.Push((const unsigned char*)datetime, len0);
				m_out.Push((const unsigned char*)os, (unsigned int)len);
            }
                break;
            case tagDataType::DATETIME2N:
            case tagDataType::DATETIMN:
			{
				unsigned int us;
				std::tm tm;
				tm.tm_isdst = -1;
				tm.tm_wday = 0;
				tm.tm_yday = 0;
				if (bytes == 4 && scale == 0) { //smalldatetime
					unsigned short days, minutes;
					m_buffer >> days >> minutes;
					unsigned int time = minutes;
					time *= 18000; //60 * 300
					DateTime dt;
					dt.Day = days;
					dt.SecCount = time;
					ToDateTime(dt, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
				}
				else if (bytes == 8 && scale == 0) { //datetime
					DateTime dt;
					m_buffer >> dt;
					ToDateTime(dt, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
				}
				else {
					Date dt;
					assert(scale <= 7);
					if (scale >= 5 && scale <= 7) {
						unsigned int low;
						unsigned char high;
						m_buffer >> low >> high;
						SPA::UINT64 time = high;
						time <<= 32;
						time += low;
						m_buffer >> dt;
						ToDateTime(dt, time, scale, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
					}
					else if (scale == 3 || scale == 4) {
						unsigned int time;
						m_buffer >> time >> dt;
						ToDateTime(dt, time, scale, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
					}
					else {
						unsigned short low;
						unsigned char high;
						m_buffer >> low >> high >> dt;
						unsigned int time = high;
						time <<= 16;
						time += low;
						ToDateTime(dt, time, scale, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
					}
				}
				tm.tm_year -= 1900;
				tm.tm_mon -= 1;
				SPA::UDateTime udt(tm, us);
				m_out << (VARTYPE)VT_DATE << udt.time;
			}
                break;
            case tagDataType::TIMEN:
            {
				unsigned int us;
				assert(scale <= 7);
				std::tm tm;
				::memset(&tm, 0, sizeof(tm));
				if (scale >= 5 && scale <= 7) {
					unsigned int low;
					unsigned char high;
					m_buffer >> low >> high;
					SPA::UINT64 time = high;
					time <<= 32;
					time += low;
					ToTime(time, scale, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
				}
				else if (scale == 3 || scale == 4) {
					unsigned int time;
					m_buffer >> time;
					ToTime(time, scale, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
				}
				else {
					unsigned short low;
					unsigned char high;
					m_buffer >> low >> high;
					unsigned int time = high;
					time <<= 16;
					time += low;
					ToTime(time, scale, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
				}
				SPA::UDateTime udt(tm, us);
				m_out << (VARTYPE)VT_DATE << udt.time;
            }
                break;
            case tagDataType::FLOAT:
                m_out << (VARTYPE) VT_R8;
                m_out.Push(m_buffer.GetBuffer(), 8);
                m_buffer.Pop(8);
                break;
            case tagDataType::REAL:
                m_out << (VARTYPE) VT_R4;
                m_out.Push(m_buffer.GetBuffer(), 4);
                m_buffer.Pop(4);
                break;
            case tagDataType::DATEN:
                assert(bytes == 3);
            {
                UINT64 dt = 0;
                Date date;
                m_buffer >> date;
				int year, month, month_day;
				ToDate(date, year, month, month_day);
                m_out << (VARTYPE) VT_DATE << dt;
            }
                break;
            default:
                assert(false);
                break;
        }
        return true;
    }

    bool CSqlBatch::ParseData(tagDataType dt, CDBColumnInfo * cinfo) {
        switch (dt) {
            case tagDataType::XML:
                if (m_out.GetSize()) {
                    std::cout << "Blob size: " << m_out.GetSize() - 6 << "\n";
                    m_out.SetSize(0);
                }
            {
                unsigned int remain;
                m_buffer >> m_lenLarge >> remain;
                assert(m_lenLarge == UNKNOWN_XML_LEN);
                assert(remain > 0 && remain <= DEFAULT_PACKET_SIZE - sizeof (m_lenLarge) - sizeof (remain));
                m_out << cinfo->DataType;
                bool all_fetched = (m_buffer.GetSize() >= (remain + sizeof (remain)));
                if (all_fetched) {
                    m_out << remain;
                } else {
                    m_out << (unsigned int) m_lenLarge;
                }
                m_out.Push(m_buffer.GetBuffer(), remain);
                m_buffer.Pop(remain);
                if (all_fetched) {
#ifndef NDEBUG
                    remain = *(unsigned int*) m_buffer.GetBuffer();
                    assert(!remain);
#endif
                    m_buffer.Pop(sizeof (remain));
                    assert(m_lenLarge == UNKNOWN_XML_LEN);
                    m_lenLarge = 0;
                    return true;
                }
                assert(!m_buffer.GetSize());
                return false;
            }
                break;
            case tagDataType::NTEXT:
            case tagDataType::IMAGE:
            case tagDataType::TEXT:
            {
                if (m_out.GetSize()) {
                    std::cout << "Blob size: " << m_out.GetSize() - 6 << "\n";
                    m_out.SetSize(0);
                }
                unsigned char skipped_bytes;
                m_buffer >> skipped_bytes;
                m_buffer.Pop(skipped_bytes);
                UINT64 timestamp;
                m_buffer >> timestamp;
                unsigned int image_bytes;
                m_buffer >> image_bytes;
                m_lenLarge = image_bytes;
                m_out << cinfo->DataType << (unsigned int) m_lenLarge;
                if (image_bytes > m_buffer.GetSize()) {
                    image_bytes = m_buffer.GetSize();
                }
                m_out.Push(m_buffer.GetBuffer(), image_bytes);
                m_buffer.Pop(image_bytes);
                m_lenLarge -= image_bytes;
                if (m_lenLarge) {
                    assert(!m_endLarge);
                    m_endLarge = (dt == tagDataType::NTEXT) ? MAX_NTEXT_LEN : MAX_IMAGE_TEXT_LEN;
                    return false;
                }
            }
                break;
            case tagDataType::UDT:
            {
                if (m_out.GetSize()) {
                    std::cout << "Blob size: " << m_out.GetSize() - 6 << "\n";
                    m_out.SetSize(0);
                }
                unsigned int remain;
                m_buffer >> m_lenLarge >> remain;
                assert(remain <= DEFAULT_PACKET_SIZE - sizeof (m_lenLarge) - sizeof (remain));
                m_out << cinfo->DataType << (unsigned int) m_lenLarge;
                if (!remain) {
                    return true; //no data
                }
                m_out.Push(m_buffer.GetBuffer(), remain);
                m_buffer.Pop(remain);
                m_lenLarge -= remain;
                m_endLarge = UINT_NULL_LEN;
                if (m_lenLarge) {
                    return false;
                }
                if (m_buffer.GetSize() >= sizeof (unsigned int)) {
                    m_buffer >> m_endLarge;
                    assert(!m_endLarge);
                } else {
                    return false;
                }
            }
                break;
            case tagDataType::NCHAR:
            case tagDataType::NVARCHAR:
            case tagDataType::CHAR:
            case tagDataType::VARCHAR:
            case tagDataType::BINARY:
            case tagDataType::VARBINARY:
                if (cinfo->ColumnSize == 0xffff) {
                    if (m_out.GetSize()) {
                        std::cout << "Blob size: " << m_out.GetSize() - 6 << "\n";
                        m_out.SetSize(0);
                    }
                    unsigned int remain;
                    m_buffer >> m_lenLarge >> remain;
                    assert(remain > 0 && remain <= DEFAULT_PACKET_SIZE - sizeof (m_lenLarge) - sizeof (remain));
                    m_out << cinfo->DataType << (unsigned int) m_lenLarge;
                    m_out.Push(m_buffer.GetBuffer(), remain);
                    m_buffer.Pop(remain);
                    m_lenLarge -= remain;
                    m_endLarge = UINT_NULL_LEN;
                    if (m_lenLarge) {
                        return false;
                    }
                    if (m_buffer.GetSize() >= sizeof (unsigned int)) {
                        m_buffer >> m_endLarge;
                        assert(!m_endLarge);
                    } else {
                        return false;
                    }
                } else {
                    const wchar_t *str = (const wchar_t*) m_buffer.GetBuffer();
                    unsigned short len = *(unsigned short*) m_buffer.GetBuffer();
                    if (len > m_buffer.GetSize() - sizeof (len)) {
                        return false;
                    }
                    m_buffer.Pop(2);
                    m_out << GetVarType(dt, 0) << (unsigned int) len;
                    m_out.Push(m_buffer.GetBuffer(), len);
                    m_buffer.Pop(len);
                }
                break;
            default:
                assert(false);
                break;
        }
        return true;
    }

    bool CSqlBatch::ParseNBCRow() {
        CDBColumnInfo *cinfo = m_vCol.data();
        unsigned short cols = (unsigned short) m_vCol.size();
        unsigned char *nulls = m_vNull.data();
        tagDataType *pdt = m_vDT.data();
        if (m_posCol == INVALID_COL) {
            if (m_buffer.GetSize() <= m_vNull.size()) {
                return false;
            }
            m_buffer.Pop(nulls, (unsigned int) m_vNull.size());
            m_posCol = 0;
        }
        VARTYPE vt;
        cinfo += m_posCol;
        bool done = IsDone();
        for (unsigned short n = m_posCol; n < cols; ++n, ++m_posCol, ++cinfo) {
            if (!m_buffer.GetSize()) {
                return false;
            }
            bool is_null = (nulls[n >> 3] & (1 << (n % 8)));
            if (is_null) {
                vt = VT_NULL;
                m_out << vt;
                continue;
            }
            if (!done) {
                if (m_buffer.GetSize() <= sizeof (PacketHeader) + sizeof (m_Done) + sizeof (tagTokenType)) {
                    return false;
                }
            }
            tagDataType dt = pdt[m_posCol];
            switch (dt) {
                case tagDataType::SQL_VARIANT:
                    if (!ParseVariant(cinfo)) {
                        return false;
                    }
                    break;
                case tagDataType::UDT:
                case tagDataType::TEXT:
                case tagDataType::NTEXT:
                case tagDataType::IMAGE:
                case tagDataType::XML:
                case tagDataType::BINARY:
                case tagDataType::CHAR:
                case tagDataType::VARBINARY:
                case tagDataType::VARCHAR:
                case tagDataType::NVARCHAR:
                case tagDataType::NCHAR:
                    if (!ParseData(dt, cinfo)) {
                        return false;
                    }
                    break;
                case tagDataType::DATETIME2N:
                case tagDataType::DATETIMN:
                case tagDataType::DECIMAL:
                case tagDataType::NUMERIC:
                case tagDataType::TIMEN:
                case tagDataType::INTN:
                case tagDataType::DATEN:
                case tagDataType::UNIQUEIDENTIFIER:
                case tagDataType::MONEYN:
                case tagDataType::DATETIMEOFFSETN:
                {
                    unsigned char bytes;
                    m_buffer >> bytes;
                    if (!ParseData(dt, bytes, cinfo->Scale)) {
                        return false;
                    }
                }
                    break;
                default:
                    if (!ParseData(dt, cinfo->ColumnSize, cinfo->Scale)) {
                        return false;
                    }
                    break;
            }
        }
        m_posCol = INVALID_COL;
        m_tt = tagTokenType::ttZero;
        return true;
    }

    bool CSqlBatch::ParseOrder() {
        unsigned short len;
        if (m_buffer.GetSize() < sizeof (len)) {
            return false;
        }
        len = *(unsigned short*) m_buffer.GetBuffer();
        if (m_buffer.GetSize() < len + sizeof (len)) {
            return false;
        }
        m_buffer.Pop(2);
        const unsigned short *start = (const unsigned short *) m_buffer.GetBuffer();
        m_vOrder.assign(start, start + (len >> 1));
        m_buffer.Pop(len);
        m_tt = tagTokenType::ttZero;
        return true;
    }

    bool CSqlBatch::ParseLoginAck() {
        unsigned short len = *(unsigned short*)m_buffer.GetBuffer();
        if (len + sizeof(len) > m_buffer.GetSize()) {
            return false;
        }
        unsigned char b;
        m_buffer >> len >> b >> m_LoginAck.Tds_Version;
        assert(b == 1); //Interface
        m_buffer >> b; //byte length
        const char16_t* str = (const char16_t*)m_buffer.GetBuffer();
        m_LoginAck.ServerName.assign(str, str + b);
        m_buffer.Pop((unsigned int)(m_LoginAck.ServerName.size() << 1));
        m_buffer >> m_LoginAck.ServerVersion;
        m_tt = tagTokenType::ttZero;
        return true;
    }

	bool CSqlBatch::ParseStream() {
		if (m_lenLarge == UNKNOWN_XML_LEN) {
			unsigned int len;
			m_buffer >> len;
			assert(len > 0 && len <= DEFAULT_PACKET_SIZE - 12);
			assert(m_buffer.GetSize() >= len);
			m_out.Push(m_buffer.GetBuffer(), len);
			m_buffer.Pop(len);
			if (m_buffer.GetSize() >= sizeof(len)) {
				len = *(unsigned int*)m_buffer.GetBuffer();
				if (len == 0) {
					m_buffer.Pop(sizeof(len));
					m_lenLarge = 0;
					++m_posCol;
					if (m_posCol == m_vCol.size()) {
						m_posCol = INVALID_COL;
						m_tt = tagTokenType::ttZero;
					}
				}
				else {
					return false;
				}
			}
			else {
				return false;
			}
		}
		else {
			if (m_lenLarge && (m_endLarge == MAX_IMAGE_TEXT_LEN || m_endLarge == MAX_NTEXT_LEN)) {
				unsigned int len = *(unsigned int*)m_buffer.GetBuffer();
				len = m_buffer.GetSize();
				if (len > m_lenLarge) {
					len = (unsigned int)m_lenLarge;
				}
				m_out.Push(m_buffer.GetBuffer(), len);
				m_buffer.Pop(len);
				m_lenLarge -= len;
				if (!m_lenLarge) {
					assert(m_endLarge == MAX_IMAGE_TEXT_LEN || m_endLarge == MAX_NTEXT_LEN);
					m_endLarge = 0;
					++m_posCol;
					if (m_posCol == m_vCol.size()) {
						m_posCol = INVALID_COL;
						m_tt = tagTokenType::ttZero;
					}
				}
				else {
					assert(!m_buffer.GetSize());
				}
			}
			else {
				while (m_lenLarge && m_buffer.GetSize()) {
					unsigned int len;
					m_buffer >> len;
					assert(len > 0 && len <= DEFAULT_PACKET_SIZE - 12);
					assert(m_buffer.GetSize() >= len);
					m_out.Push(m_buffer.GetBuffer(), len);
					m_buffer.Pop(len);
					m_lenLarge -= len;
					if (!m_lenLarge) {
						++m_posCol;
						if (m_posCol == m_vCol.size()) {
							m_posCol = INVALID_COL;
							m_tt = tagTokenType::ttZero;
						}
					}
					else {
						assert(m_lenLarge < 0x7fffffff);
					}
				}
				if (!m_lenLarge && m_endLarge == UINT_NULL_LEN) {
					if (m_buffer.GetSize() >= sizeof(m_endLarge)) {
						m_buffer >> m_endLarge;
						assert(!m_endLarge);
					}
					else {
						return false;
					}
				}
			}
		}
		while (m_buffer.GetSize()) {
			if (m_tt == tagTokenType::ttZero) {
				m_buffer >> m_tt;
			}
			switch (m_tt) {
			case tagTokenType::ttORDER:
				if (!ParseOrder()) {
					return false;
				}
				break;
			case tagTokenType::ttNBCROW:
				if (!ParseNBCRow()) {
					return false;
				}
				break;
			case tagTokenType::ttROW:
				if (!ParseRow()) {
					return false;
				}
				break;
			case tagTokenType::ttCOLMETADATA:
				if (!ParseMeta()) {
					return false;
				}
				break;
			case tagTokenType::ttTDS_ERROR:
			case tagTokenType::ttINFO:
				if (!ParseErrorInfo()) {
					return false;
				}
				break;
            case tagTokenType::ttLOGINACK:
                if (!ParseLoginAck()) {
                    return false;
                }
                break;
			case tagTokenType::ttENVCHANGE:
				if (!ParseEventChange()) {
					return false;
				}
				break;
			case tagTokenType::ttDONEINPROC:
				if (!ParseDoneInProc()) {
					return false;
				}
				break;
			case tagTokenType::ttRETURNSTATUS:
				if (!ParseReturnStatus()) {
					return false;
				}
				break;
			case tagTokenType::ttDONEPROC:
			case tagTokenType::ttDONE:
				if (!ParseDone()) {
					return false;
				}
				else if (IsDone() && !HasMore() && m_buffer.GetSize()) {
#ifndef NDEBUG
					std::cout << "CSqlBatch::ParseStream/Remaining bytes: " << m_buffer.GetSize() << "\n";
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

    void CSqlBatch::ParseTransChange(tagEnvchangeType type, TransChange& tc) {
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

    int CSqlBatch::SendMessage(tagRequestType rt, tagIsolationLevel il, SPA::UINT64 trans_decriptor) {
        Reset();
        TransactionDescriptor td(trans_decriptor);
        PacketHeader ph(tagPacketType::ptTransaction, 1);
        ph.Length = (Packet_Length)(sizeof(ph) + sizeof(td) + sizeof(rt) + sizeof(il));
        ph.Length = ChangeEndian(ph.Length);
        SPA::CScopeUQueue sb;
        sb << ph << td << rt << il;
        return m_channel.Send(sb->GetBuffer(), sb->GetSize(), this);
    }

    bool CSqlBatch::ParseDone() {
		if (CReqBase::ParseDone()) {
			if (m_Done.Status == tagDoneStatus::dsFinal || (m_Done.Status & tagDoneStatus::dsMore) == tagDoneStatus::dsMore || (m_Done.Status & tagDoneStatus::dsCount) == tagDoneStatus::dsCount) {
				m_posCol = INVALID_COL;
				memset(&m_collation, 0, sizeof(m_collation));
				m_cols = 0;
				m_vCol.clear();
				m_vDT.clear();
				m_vNull.clear();
				m_out.SetSize(0);
			}
			return true;
		}
        return false;
    }

    bool CSqlBatch::ParseEventChange() {
        if (m_buffer.GetSize() > 2) {
			unsigned short len = *(unsigned short *) m_buffer.GetBuffer();
            if (len + sizeof(unsigned short) <= m_buffer.GetSize()) {
                tagEnvchangeType type;
                m_buffer >> len >> type;
                switch (type) {
                    case tagEnvchangeType::packet_size:
                    case tagEnvchangeType::language:
                    case tagEnvchangeType::database:
                    {
                        StringEventChange sec;
						m_vEventChange.push_back(sec);
						ParseStringChange(type, m_vEventChange.back());
                    }
                        break;
					case tagEnvchangeType::begin_trans:
					case tagEnvchangeType::commit_trans:
					case tagEnvchangeType::rollback_trans:
					{
						TransChange tc;
						ParseTransChange(type, tc);
						m_vTransChange.push_back(tc);
					}
						break;
                    case tagEnvchangeType::collation:
                        if (!ParseCollation(m_CollationChange)) {
                            return false;
                        }
                        break;
                    default:
                        assert(false);
                        break;
                }
                m_tt = tagTokenType::ttZero;
                return true;
            }
        }
        return false;
    }
}
