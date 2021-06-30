#include "sqlbatch.h"
#include "tdschannel.h"
#include <iostream>
#include "../include/odbc/uodbc.h"

namespace tds
{
    SPA::CDBString CSqlBatch::LibraryName(u"udaparts_sql_server_client");

    SPA::CDBString CSqlBatch::SQL_SERVER_DEFAULT_SCHEMA(u"dbo");

    CSqlBatch::CSqlBatch(CTdsChannel& channel, bool meta)
            : CReqBase(channel), m_out(*m_sbOut), m_timeout(0), m_meta(meta), m_cols(0), m_posCol(INVALID_COL), m_lenLarge(0),
            m_endLarge(0), m_rs(0), m_inputs(0), m_outputs(0), m_affects(0), m_bConnected(false) {
    }

    inline VARTYPE CSqlBatch::GetVarType(tagDataType dt, unsigned char money_bytes) {
        switch (dt) {
            case tagDataType::IMAGE:
                return VT_ARRAY | VT_UI1;
            case tagDataType::TEXT:
                return VT_ARRAY | VT_I1;
            case tagDataType::UNIQUEIDENTIFIER:
                return VT_CLSID;
            case tagDataType::INTN:
            case tagDataType::TINYINT:
                return VT_UI1;
            case tagDataType::BIT:
                return VT_BOOL;
            case tagDataType::SMALLINT:
                return VT_I2;
            case tagDataType::INT:
                return VT_I4;
            case tagDataType::REAL:
                return VT_R4;
            case tagDataType::MONEYN:
                return (money_bytes > 4) ? VT_I8 : VT_I4;
            case tagDataType::MONEY:
                return VT_I8;
            case tagDataType::DATETIMEOFFSETN:
                return VT_ARRAY | VT_I1;
            case tagDataType::TIMEN:
            case tagDataType::DATEN:
            case tagDataType::DATETIMN: //smalldatetime
            case tagDataType::DATETIME:
            case tagDataType::DATETIME2N: //datetime2
                return VT_DATE;
            case tagDataType::FLOAT:
                return VT_R8;
            case tagDataType::SQL_VARIANT:
                return VT_VARIANT;
            case tagDataType::NTEXT:
                return VT_BSTR;
            case tagDataType::DECIMAL:
            case tagDataType::NUMERIC:
                return VT_DECIMAL;
            case tagDataType::SMALLMONEY:
                return VT_I4;
            case tagDataType::BIGINT:
                return VT_I8;
            case tagDataType::CHAR:
            case tagDataType::VARCHAR:
                return (VT_ARRAY | VT_I1);
            case tagDataType::VARBINARY:
            case tagDataType::BINARY:
                return (VT_ARRAY | VT_UI1);
            case tagDataType::NVARCHAR:
            case tagDataType::NCHAR:
                return VT_BSTR;
            case tagDataType::UDT:
                return (VT_ARRAY | VT_UI1);
            case tagDataType::XML:
                return VT_BSTR;
            default:
                assert(false);
                break;
        }
        return VT_VARIANT;
    }

    inline SPA::CDBString CSqlBatch::GetSqlDeclaredType(tagDataType dt, unsigned char money_bytes) {
        switch (dt) {
            case tagDataType::IMAGE:
                return u"image";
            case tagDataType::TEXT:
                return u"text";
            case tagDataType::UNIQUEIDENTIFIER:
                return u"uniqueidentifier";
            case tagDataType::INTN:
            case tagDataType::TINYINT:
                return u"tinyint";
            case tagDataType::BIT:
                return u"bit";
            case tagDataType::SMALLINT:
                return u"smallint";
            case tagDataType::INT:
                return u"int";
            case tagDataType::REAL:
                return u"real";
            case tagDataType::MONEYN:
                return ((money_bytes > 4) ? u"money" : u"smallmoney");
            case tagDataType::MONEY:
                return u"money";
            case tagDataType::DATETIMEOFFSETN:
                return u"datetimeoffset";
            case tagDataType::TIMEN:
                return u"time";
            case tagDataType::DATEN:
                return u"date";
            case tagDataType::DATETIMN:
                return ((money_bytes > 4) ? u"datetime" : u"smalldatetime");
            case tagDataType::DATETIME:
                return u"datetime";
            case tagDataType::DATETIME2N: //datetime2
                return u"datetime2";
            case tagDataType::FLOAT:
                return u"float";
            case tagDataType::SQL_VARIANT:
                return u"sql_variant";
            case tagDataType::NTEXT:
                return u"ntext";
            case tagDataType::DECIMAL:
                return u"decimal";
            case tagDataType::NUMERIC:
                return u"numeric";
            case tagDataType::SMALLMONEY:
                return u"smallmoney";
            case tagDataType::BIGINT:
                return u"bigint";
            case tagDataType::CHAR:
                return u"char";
            case tagDataType::VARCHAR:
                return u"varchar";
            case tagDataType::VARBINARY:
                return u"varbinary";
            case tagDataType::BINARY:
                return u"binary";
            case tagDataType::NVARCHAR:
                return u"nvarchar";
            case tagDataType::NCHAR:
                return u"nchar";
            case tagDataType::UDT:
                return u"udt";
            case tagDataType::XML:
                return u"xml";
            default:
                assert(false);
                break;
        }
        return u"sql_variant";
    }

    // Obfuscate password to be sent to SQL Server
    // Blurb from the TDS spec at https://msdn.microsoft.com/en-us/library/dd304523.aspx
    // "Before submitting a password from the client to the server, for every byte in the password buffer 
    // starting with the position pointed to by IbPassword, the client SHOULD first swap the four high bits 
    // with the four low bits and then do a bit-XOR with 0xA5 (10100101). After reading a submitted password, 
    // for every byte in the password buffer starting with the position pointed to by IbPassword, the server SHOULD 
    // first do a bit-XOR with 0xA5 (10100101) and then swap the four high bits with the four low bits."
    // The password exchange during Login phase happens over a secure channel i.e. SSL/TLS 
    // Note: The same logic is used in SNIPacketSetData (SniManagedWrapper) to encrypt passwords stored in SecureString
    // If this logic changed, SNIPacketSetData needs to be changed as well

    std::vector<unsigned char> CSqlBatch::ObfuscatePassword(const SPA::CDBString & password) {
#if 1
        unsigned char bLo, bHi;
        std::vector<unsigned char> v(password.size() << 1);
        unsigned char* bObfuscated = &v.front();
        for (size_t n = 0, len = password.size(); n < len; ++n) {
            char16_t c16 = password[n];
            bLo = (unsigned char) (c16 & 0xff);
            bHi = (unsigned char) ((c16 >> 8) & 0xff);
            bObfuscated[n << 1] = (unsigned char) ((((bLo & 0x0f) << 4) | (bLo >> 4)) ^ 0xa5);
            bObfuscated[(n << 1) + 1] = (unsigned char) ((((bHi & 0x0f) << 4) | (bHi >> 4)) ^ 0xa5);
        }
#else
        const unsigned char* start = (const unsigned char*) password.data();
        std::vector<unsigned char> v(start, start + (password.size() << 1));
#endif
        return v;
    }

    unsigned char* CSqlBatch::ObfuscatePassword(unsigned char* password, unsigned int bytes) {
        unsigned char bLo, bHi;
        for (unsigned int i = 0; i < bytes; i++) {
            bLo = (password[i] & 0x0f);
            bHi = (password[i] & 0xf0);
            password[i] = (((bHi >> 4) | (bLo << 4)) ^ 0xa5);
        }
        return password;
    }

    //https://en.wikipedia.org/wiki/Julian_day#Gregorian_calendar_from_Julian_day_number

    int CSqlBatch::ToTdsJDN(int year, int month, int month_day) {
        return (1461 * (year + 4800 + (month - 14) / 12)) / 4 + (367 * (month - 2 - 12 * ((month - 14) / 12))) / 12 - (3 * ((year + 4900 + (month - 14) / 12) / 100)) / 4 + month_day - 32075 - TDS_JDN_OFFSET_1_1_1;
    }

    void CSqlBatch::ToDate(Date date, int& year, int& month, int& month_day) {
        int J = date.High;
        J <<= 16;
        J += date.Low;
        if (!J) {
            year = 0;
            month = 0;
            month_day = 0;
            return;
        }

        J += TDS_JDN_OFFSET_1_1_1; //offset 01/01/01

        int f = J + 1401 + (((4 * J + 274277) / 146097) * 3) / 4 - 38;
        int e = 4 * f + 3;
        int g = (e % 1461) / 4;
        int h = 5 * g + 2;
        month_day = (h % 153) / 5 + 1;
        month = ((h / 153 + 2) % 12) + 1;
        year = (e / 1461) - 4716 + (14 - month) / 12;
    }

    void CSqlBatch::ToDateTime(DateTime dt, int& year, int& month, int& month_day, int& hour, int& minute, int& second, unsigned int& us) {
        int dmy = dt.Day + TDS_JDN_OFFSET_1900_1_1;
        Date date;
        date.Low = (dmy & 0xffff);
        date.High = (dmy >> 16) & 0xff;
        ToDate(date, year, month, month_day);

        hour = dt.SecCount / DATETIME_HOUR_TICKET;
        unsigned int remain = (dt.SecCount % DATETIME_HOUR_TICKET);

        minute = remain / DATETIME_MINUTE_TICKET;
        remain = (remain % DATETIME_MINUTE_TICKET);

        second = remain / DATETIME_SECOND_TICKET;
        us = (remain % DATETIME_SECOND_TICKET);
        us *= 10;
        double d = us;
        d = d / 3 + 0.5;
        us = (unsigned int) d;
        us *= 1000;
    }

    void CSqlBatch::ToTime(SPA::UINT64 time, unsigned char scale, int& hour, int& minute, int& second, unsigned int& us) {
        assert(scale <= 7);
        unsigned int p = (unsigned int) pow(10, scale);
        unsigned int fraction = (unsigned int) (time % p);
        unsigned int day_seconds = (unsigned int) (time / p);
        hour = (int) (day_seconds / 3600);
        day_seconds = (day_seconds % 3600);
        minute = (int) (day_seconds / 60);
        second = (day_seconds % 60);
        double d = fraction / pow(10, (char) scale - 6) + 0.5;
        us = (unsigned int) d;
    }

    void CSqlBatch::ToDateTime(Date dt, SPA::UINT64 time, unsigned char scale, int& year, int& month, int& month_day, int& hour, int& minute, int& second, unsigned int& us) {
        ToDate(dt, year, month, month_day);
        ToTime(time, scale, hour, minute, second, us);
    }

    unsigned int CSqlBatch::ToUDBFlags(ColFlag cf) {
        unsigned int flags = 0;
        if (cf.CaseSensitivity) {
            flags |= SPA::UDB::CDBColumnInfo::FLAG_CASE_SENSITIVE;
        }
        if (!cf.Nullable) {
            flags |= SPA::UDB::CDBColumnInfo::FLAG_NOT_NULL;
        }
        if (cf.Identity) {
            flags |= SPA::UDB::CDBColumnInfo::FLAG_NOT_NULL;
            flags |= SPA::UDB::CDBColumnInfo::FLAG_AUTOINCREMENT;
            flags |= SPA::UDB::CDBColumnInfo::FLAG_PRIMARY_KEY;
            flags |= SPA::UDB::CDBColumnInfo::FLAG_UNIQUE;
        }
        if (cf.Updateable == 0) {
            flags |= SPA::UDB::CDBColumnInfo::FLAG_NOT_WRITABLE;
        }
        return flags;
    }

    void CSqlBatch::Reset() {
        m_vInfo.clear();
        memset(&m_CollationChange, 0, sizeof (m_CollationChange));
        m_vEventChange.clear();
        m_vCol.clear();
        m_cols = 0;
        m_vDT.clear();
        m_posCol = INVALID_COL;
        m_dip.Status = tagDoneStatus::dsInitial;
        m_rs = 0;
        CReqBase::Reset();
    }

    SPA::CDBString CSqlBatch::GetSQLProc(const char16_t* procName, const char16_t* schema, const char16_t* dbName) {
        SPA::CDBString sql_proc;
        bool switched = false;
        if (!schema || !SPA::GetLen(schema)) {
            schema = SQL_SERVER_DEFAULT_SCHEMA.c_str();
        }
        if (dbName && SPA::GetLen(dbName)) {
            SPA::CDBString dname = dbName;
            SPA::Trim(dname);
            SPA::ToLower(dname);
            if (dname != m_dbNameChange.NewValue) {
                sql_proc = u"use ";
                sql_proc += dname;
                sql_proc.push_back(';');
                switched = true;
            }
        }
        sql_proc += u"select sysp.name,parameter_id as porder,is_output,type_name(user_type_id)as type_name,max_length from sys.objects obj left join sys.parameters sysp on sysp.object_id=obj.object_id where obj.type in('P','PC')and obj.name='";
        sql_proc += procName;
        sql_proc += u"' and schema_name(obj.schema_id)='";
        sql_proc += schema;
        sql_proc += u"' order by porder";
        if (switched) {
            sql_proc.push_back(';');
            sql_proc += u"use ";
            sql_proc += m_dbNameChange.NewValue;
        }
        return std::move(sql_proc);
    }

    SPA::CDBString CSqlBatch::Prepare(const char16_t* sql, unsigned int& parameters) {
        assert(sql);
        assert(SPA::GetLen(sql));
        bool called = false;
        parameters = 0;
        SPA::CDBString s = sql ? sql : u"";
        SPA::Trim(s);
        if (!s.size()) {
            return s;
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
                    str = "p" + str;
                    if (m_vParamInfo.size() > parameters && m_vParamInfo[parameters].Direction != SPA::UDB::tagParameterDirection::pdInput) {
                        str += " OUT";
                    }
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

    int CSqlBatch::Prepare(const char16_t* sql, SPA::UDB::CParameterInfoArray& params, unsigned int& parameters) {
        m_inputs = 0;
        m_outputs = 0;
        m_vParamInfo = std::move(params);
        m_sqlPrepare = Prepare(sql, parameters);
        if (!parameters || !m_sqlPrepare.size()) {
            return SPA::Odbc::ER_NO_PARAMETER_SPECIFIED;
        }
        if (m_vParamInfo.size() && m_vParamInfo.size() != parameters) {
            return SPA::Odbc::ER_BAD_PARAMETER_COLUMN_SIZE;
        }
        unsigned short outputs = 0;
        for (auto it = m_vParamInfo.cbegin(), end = m_vParamInfo.cend(); it != end; ++it) {
            switch (it->Direction) {
                case SPA::UDB::tagParameterDirection::pdInput:
                    break;
                case SPA::UDB::tagParameterDirection::pdInputOutput:
                case SPA::UDB::tagParameterDirection::pdOutput:
                    ++outputs;
                    break;
                default:
                    return SPA::Odbc::ER_BAD_PARAMETER_DIRECTION_TYPE;
            }
            switch (it->DataType) {
                case VT_UI1:
                case VT_I1:
                case VT_I2:
                case VT_UI2:
                case VT_I4:
                case VT_UI4:
                case VT_INT:
                case VT_UINT:
                case VT_I8:
                case VT_UI8:
                case VT_R4:
                case VT_R8:
                case VT_DATE:
                case VT_CY:
                case VT_CLSID:
                case VT_BOOL:
                case SPA::VT_XML:
                    break;
                case (VT_UI1 | VT_ARRAY):
                case (VT_I1 | VT_ARRAY):
                    if (!it->ColumnSize || (it->ColumnSize > 8000 && it->ColumnSize != MAX_IMAGE_TEXT_LEN)) {
                        return ER_BAD_PARAMETER_INFO_COLUMN_SIZE;
                    }
                    break;
                case VT_BSTR:
                    if (!it->ColumnSize || (it->ColumnSize > 4000 && it->ColumnSize != MAX_IMAGE_TEXT_LEN)) {
                        return ER_BAD_PARAMETER_INFO_COLUMN_SIZE;
                    }
                    break;
                case VT_DECIMAL:
                    if (!it->Precision || it->Precision > 38 || it->Precision < it->Scale) {
                        return ER_BAD_DECIMAL_PRECSION_PROVIDED;
                    }
                    break;
                default:
                    return SPA::Odbc::ER_DATA_TYPE_NOT_SUPPORTED;
            }
        }
        m_outputs = outputs;
        m_inputs = (unsigned short) (parameters - m_outputs);
        parameters = m_outputs;
        parameters <<= 16;
        parameters += m_inputs;
        return 0;
    }

    int CSqlBatch::ToString(const SPA::UDB::CDBVariant* pVt, unsigned int count, SPA::CDBString & s) const {
        char16_t param[16];
        s.clear();
        unsigned int ps = (unsigned int) m_vParamInfo.size();
        for (unsigned int n = 0; n < count; ++n) {
            if (s.size()) {
                s.push_back(',');
            }
            const SPA::UDB::CParameterInfo* pi = nullptr;
            if (ps) {
                pi = m_vParamInfo.data() + (n % ps);
            }
            const SPA::UDB::CDBVariant& v = pVt[n];
            s.append(u"@p", 2);
            unsigned char chars = sizeof (param) / sizeof (char16_t);
            const char16_t* ret = SPA::ToString(n, param, chars);
            s.append(ret, chars);
            s.push_back(' ');
            VARTYPE vt = v.vt;
            if (vt <= VT_NULL && pi) {
                vt = pi->DataType;
            }
            switch (vt) {
                case VT_CLSID:
                    s += u"uniqueidentifier";
                    break;
                case VT_I1:
                case VT_UI1:
                    s += u"tinyint";
                    break;
                case VT_I2:
                case VT_UI2:
                    s += u"smallint";
                    break;
                case VT_I4:
                case VT_UI4:
                case VT_INT:
                case VT_UINT:
                    s += u"int";
                    break;
                case VT_I8:
                case VT_UI8:
                    s += u"bigint";
                    break;
                case VT_R4:
                    s += u"real";
                    break;
                case VT_R8:
                    s += u"float";
                    break;
                case VT_BOOL:
                    s += u"bit";
                    break;
                case VT_DATE:
                {
                    SPA::UDateTime udt(v.ullVal);
                    std::tm tm = udt.GetCTime();
                    if (tm.tm_mday) {
                        s += u"datetime2(6)";
                    } else {
                        s += u"time(6)";
                    }
                }
                    break;
                case VT_CY:
                    s += u"money";
                    break;
                case VT_DECIMAL:
                    if (v.decVal.Hi32) {
                        s += u"decimal(28,";
                    } else {
                        s += u"decimal(19,";
                    }
                    chars = sizeof (param) / sizeof (char16_t);
                    ret = SPA::ToString((unsigned int) v.decVal.scale, param, chars);
                    s.append(ret, chars);
                    s.push_back(')');
                    break;
                case (VT_ARRAY | VT_I1):
                    s += u"varchar(";
                    if (pi && pi->Direction != SPA::UDB::tagParameterDirection::pdInput) {
                        if (pi->ColumnSize > 8000) {
                            s += u"max";
                        } else {
                            chars = sizeof (param) / sizeof (char16_t);
                            ret = SPA::ToString(pi->ColumnSize, param, chars);
                            s.append(ret, chars);
                        }
                    } else {
                        unsigned int len = v.parray->rgsabound[0].cElements;
                        if (len > 8000) {
                            s += u"max";
                        } else {
                            if (!len) len = 1;
                            chars = sizeof (param) / sizeof (char16_t);
                            ret = SPA::ToString(len, param, chars);
                            s.append(ret, chars);
                        }
                    }
                    s.push_back(')');
                    break;
                case VT_BSTR:
                    if (pi && pi->DataType == SPA::VT_XML) {
                        s += u"xml";
                    } else {
                        s += u"nvarchar(";
                        if (pi && pi->Direction != SPA::UDB::tagParameterDirection::pdInput) {
                            if (pi->ColumnSize > 4000) {
                                s += u"max";
                            } else {
                                chars = sizeof (param) / sizeof (char16_t);
                                ret = SPA::ToString(pi->ColumnSize, param, chars);
                                s.append(ret, chars);
                            }
                        } else {
                            unsigned int len = ::SysStringLen(v.bstrVal);
                            if (len > 4000) {
                                s += u"max";
                            } else {
                                if (!len) len = 1;
                                chars = sizeof (param) / sizeof (char16_t);
                                ret = SPA::ToString(len, param, chars);
                                s.append(ret, chars);
                            }
                        }
                        s.push_back(')');
                    }
                    break;
                case SPA::VT_XML:
                    s += u"xml";
                    break;
                case (VT_ARRAY | VT_UI1):
                    if (v.VtExt == SPA::UDB::tagVTExt::vteGuid) {
                        s += u"uniqueidentifier";
                    } else {
                        s += u"varbinary(";
                        if (pi && pi->Direction != SPA::UDB::tagParameterDirection::pdInput) {
                            if (pi->ColumnSize > 8000) {
                                s += u"max";
                            } else {
                                chars = sizeof (param) / sizeof (char16_t);
                                ret = SPA::ToString(pi->ColumnSize, param, chars);
                                s.append(ret, chars);
                            }
                        } else {
                            unsigned int len = v.parray->rgsabound[0].cElements;
                            if (len > 8000) {
                                s += u"max";
                            } else {
                                if (!len) len = 1;
                                chars = sizeof (param) / sizeof (char16_t);
                                ret = SPA::ToString(len, param, chars);
                                s.append(ret, chars);
                            }
                        }
                        s.push_back(')');
                    }
                    break;
                case VT_NULL:
                case VT_EMPTY:
                    if (pi && pi->DataType == SPA::VT_XML) {
                        s += u"xml";
                    } else {
                        s += u"varchar(16)";
                    }
                    break;
                default:
                    return SPA::Odbc::ER_DATA_TYPE_NOT_SUPPORTED;
            }
            if (pi) {
                switch (pi->Direction) {
                    case SPA::UDB::tagParameterDirection::pdOutput:
                    case SPA::UDB::tagParameterDirection::pdInputOutput:
                        s += u" OUT";
                        break;
                    default:
                        break;
                }
            }
        }
        return 0;
    }

    bool CSqlBatch::ParseReturnValue() {
        unsigned char status;
        unsigned int user_type;
        unsigned char flags;
        unsigned char type_info;
        unsigned short ordinal = *(unsigned short*) m_buffer.GetBuffer();
        unsigned char b_len = *m_buffer.GetBuffer(sizeof (ordinal));
        unsigned int len = sizeof (ordinal) + sizeof (b_len);
        if (m_buffer.GetSize() <= len) {
            return false;
        }
        len += (((unsigned int) b_len) << 1) + sizeof (status) + sizeof (user_type) + sizeof (flags) + sizeof (type_info);
        if (m_buffer.GetSize() <= len) {
            return false;
        }
        tagDataType dt;
        len += sizeof (dt);
        if (m_buffer.GetSize() <= len + sizeof (TokenDone)) {
            return false;
        }
        m_buffer.Pop((unsigned int) 3);
        const char16_t* pname = (const char16_t*) m_buffer.GetBuffer();
        SPA::CDBString pName(pname, pname + b_len);
        m_buffer.Pop((unsigned int) (b_len << 1));
        m_buffer >> status >> user_type >> flags >> type_info >> dt;
        switch (dt) {
#if 0
            case tagDataType::TIMEN:
            {
                unsigned char p_len; //scale
                unsigned char b_len; //length;
                m_buffer >> p_len >> b_len;
                assert(p_len == 6);
                if (b_len) {
                    assert(b_len == 5);
                    unsigned int low, us;
                    unsigned char high;
                    m_buffer >> low >> high;
                    SPA::UINT64 time = high;
                    time <<= 32;
                    time += low;
                    std::tm tm;
                    ::memset(&tm, 0, sizeof (tm));
                    tm.tm_mday = 1;
                    ToTime(time, p_len, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
                    SPA::UDateTime udt(tm, us);
                    m_out << (VARTYPE) VT_DATE << udt.time;
                } else {
                    m_out << (VARTYPE) VT_NULL;
                }
            }
                break;
#endif
            case tagDataType::MONEYN:
            {
                unsigned char bytes0, bytes1;
                m_buffer >> bytes0 >> bytes1;
                assert(bytes0 == sizeof (CY));
                if (bytes1) {
                    assert(bytes0 == bytes1);
                    CY cy;
                    m_buffer >> cy.Hi >> cy.Lo;
                    m_out << (VARTYPE) VT_CY << cy;
                } else {
                    m_out << (VARTYPE) VT_NULL;
                }
            }
                break;
            case tagDataType::UNIQUEIDENTIFIER:
            {
                unsigned char bytes0, bytes1;
                m_buffer >> bytes0 >> bytes1;
                assert(bytes0 == sizeof (CLSID));
                if (m_buffer.GetSize() < sizeof (CLSID)) {
                    return false;
                }
                if (bytes1) {
                    assert(bytes0 == bytes1);
                    m_out << (VARTYPE) VT_CLSID;
                    m_out.Push(m_buffer.GetBuffer(), sizeof (CLSID));
                    m_buffer.Pop(sizeof (CLSID));
                } else {
                    m_out << (VARTYPE) VT_NULL;
                }
            }
                break;
            case tagDataType::DATETIME2N:
            {
                unsigned char p_len; //scale
                unsigned char b_len; //length;
                m_buffer >> p_len >> b_len;
                assert(p_len == 6);
                if (b_len) {
                    assert(b_len == 8);
                    unsigned int low, us;
                    unsigned char high;
                    Date date;
                    m_buffer >> low >> high >> date;
                    SPA::UINT64 time = high;
                    time <<= 32;
                    time += low;
                    std::tm tm;
                    ToDateTime(date, time, p_len, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
                    tm.tm_year -= 1900;
                    tm.tm_mon -= 1;
                    SPA::UDateTime udt(tm, us);
                    m_out << (VARTYPE) VT_DATE << udt.time;
                } else {
                    m_out << (VARTYPE) VT_NULL;
                }
            }
                break;
            case tagDataType::FLTN:
            {
                unsigned char bytes0, bytes1;
                m_buffer >> bytes0 >> bytes1;
                switch (bytes1) {
                    case 0:
                        m_out << (VARTYPE) VT_NULL;
                        break;
                    case sizeof (float):
                        assert(bytes0 == bytes1);
                        m_out << (VARTYPE) VT_R4;
                        m_out.Push(m_buffer.GetBuffer(), sizeof (float));
                        m_buffer.Pop(sizeof (float));
                        break;
                    default:
                        assert(bytes0 == bytes1);
                        m_out << (VARTYPE) VT_R8;
                        m_out.Push(m_buffer.GetBuffer(), sizeof (double));
                        m_buffer.Pop(sizeof (double));
                        break;
                }
            }
                break;
            case tagDataType::BITN:
            {
                unsigned char bytes0, bytes1;
                m_buffer >> bytes0 >> bytes1;
                assert(bytes0 == sizeof (bool));
                switch (bytes1) {
                    case 0:
                        m_out << (VARTYPE) VT_NULL;
                        break;
                    default:
                        assert(bytes0 == bytes1);
                        m_buffer >> bytes1;
                        m_out << (VARTYPE) VT_BOOL << (VARIANT_BOOL) (bytes1 ? VARIANT_TRUE : VARIANT_FALSE);
                        break;
                }
            }
                break;
            case tagDataType::INTN:
            {
                unsigned char bytes0, bytes1;
                m_buffer >> bytes0 >> bytes1;
                switch (bytes1) {
                    case 0:
                        m_out << (VARTYPE) VT_NULL;
                        break;
                    case sizeof (unsigned char):
                        assert(bytes0 == bytes1);
                        m_out << (VARTYPE) VT_UI1;
                        m_out.Push(m_buffer.GetBuffer(), sizeof (unsigned char));
                        m_buffer.Pop(sizeof (unsigned char));
                        break;
                    case sizeof (short):
                        assert(bytes0 == bytes1);
                        m_out << (VARTYPE) VT_I2;
                        m_out.Push(m_buffer.GetBuffer(), sizeof (short));
                        m_buffer.Pop(sizeof (short));
                        break;
                    case sizeof (int):
                        assert(bytes0 == bytes1);
                        m_out << (VARTYPE) VT_I4;
                        m_out.Push(m_buffer.GetBuffer(), sizeof (int));
                        m_buffer.Pop(sizeof (int));
                        break;
                    default:
                        assert(bytes0 == bytes1);
                        m_out << (VARTYPE) VT_I8;
                        m_out.Push(m_buffer.GetBuffer(), sizeof (SPA::INT64));
                        m_buffer.Pop(sizeof (SPA::INT64));
                        break;
                }
            }
                break;
            case tagDataType::DECIMAL:
            case tagDataType::NUMERIC:
            {
                unsigned char bytes, precison, scale, sign;
                m_buffer >> bytes >> precison >> scale >> b_len >> sign;
                m_out << (VARTYPE) VT_DECIMAL;
                DECIMAL dec;
                ::memset(&dec, 0, sizeof (dec));
                if (!sign) {
                    dec.sign = 0x80;
                }
                switch (b_len) {
                    case 5:
                        m_buffer >> dec.Lo32;
                        break;
                    case 9:
                        m_buffer >> dec.Lo64;
                        break;
                    case 13:
                        m_buffer >> dec.Lo64 >> dec.Hi32;
                        break;
                    case 17:
                        m_buffer >> dec.Lo64 >> dec.Hi32;
                        m_buffer.Pop(sizeof (unsigned int));
                        break;
                    default:
                        assert(false);
                        break;
                }
                m_out << dec;
            }
                break;
            case tagDataType::VARCHAR:
                if (!ConvertTo(pName)) {
                    return false;
                }
                break;
            case tagDataType::VARBINARY:
            {
                unsigned short max_len;
                m_buffer >> max_len;
                if (max_len == VAR_MAX) {
                    if (!PopPLP(VT_ARRAY | VT_UI1)) {
                        return false;
                    }
                } else {
                    unsigned short str_len;
                    m_buffer >> str_len;
                    if (str_len == VAR_MAX) {
                        m_out << (VARTYPE) VT_NULL;
                        m_tt = tagTokenType::ttZero;
                        return true;
                    } else if (str_len > m_buffer.GetSize()) {
                        return false;
                    }
                    m_out << (VARTYPE) (VT_UI1 | VT_ARRAY);
                    m_out << (unsigned int) str_len;
                    m_out.Push(m_buffer.GetBuffer(), str_len);
                    m_buffer.Pop(str_len);
                }
            }
                break;
            case tagDataType::NVARCHAR:
            {
                unsigned short max_len;
                Collation collation;
                m_buffer >> max_len >> collation;
                if (max_len == VAR_MAX) {
                    if (!PopPLP(VT_BSTR)) {
                        return false;
                    }
                } else {
                    unsigned short str_len;
                    m_buffer >> str_len;
                    if (str_len == VAR_MAX) {
                        m_out << (VARTYPE) VT_NULL;
                        m_tt = tagTokenType::ttZero;
                        return true;
                    } else if (str_len > m_buffer.GetSize()) {
                        return false;
                    }
#ifndef NDEBUG          
                    const char16_t* head = (const char16_t*) m_buffer.GetBuffer();
#endif
                    m_out << (VARTYPE) VT_BSTR;
                    m_out << (unsigned int) str_len;
                    m_out.Push(m_buffer.GetBuffer(), str_len);
                    m_buffer.Pop(str_len);
                }
            }
                break;
            case tagDataType::XML:
            {
                unsigned char p_byte;
                SPA::UINT64 plp_len;
                if (m_buffer.GetSize() <= sizeof (unsigned char) + sizeof (SPA::UINT64)) {
                    return false;
                }
                m_buffer >> p_byte >> plp_len;
                assert(!p_byte);
                if (plp_len != BLOB_NULL_LEN) {
                    assert(plp_len == UNKNOWN_XML_LEN);
                    unsigned int sub_len;
                    if (m_buffer.GetSize() <= sizeof (sub_len)) {
                        return false;
                    }
                    m_buffer >> sub_len;
                    if (sub_len > m_buffer.GetSize()) {
                        return false;
                    }
                    m_out << (VARTYPE) VT_BSTR;
                    if (m_buffer.GetSize() >= sub_len + sizeof (unsigned int)) {
                        m_out << sub_len;
                        m_out.Push(m_buffer.GetBuffer(), sub_len);
                        unsigned int plp_terminator = *(unsigned int*) m_buffer.GetBuffer(sub_len);
                        if (plp_terminator == 0) {
                            m_buffer.Pop(sub_len + sizeof (unsigned int));
                        } else {
                            m_buffer.Pop(sub_len);
                            m_lenLarge = UNKNOWN_XML_LEN;
                        }
                    } else if (m_buffer.GetSize() >= sub_len) {
                        m_out << (unsigned int) plp_len;
                        m_out.Push(m_buffer.GetBuffer(), sub_len);
                        m_buffer.Pop(sub_len);
                        m_lenLarge = UNKNOWN_XML_LEN;
                    } else {
                        return false;
                    }
                } else {
                    m_out << (VARTYPE) VT_NULL;
                }
            }
                break;
            default:
                assert(false);
                return false;

        }
        m_tt = tagTokenType::ttZero;
        return true;
    }

    bool CSqlBatch::PopPLP(VARTYPE vt) {
        constexpr unsigned int HEADER_SIZE = sizeof (PLPHeader);
        if (m_buffer.GetSize() <= HEADER_SIZE + sizeof (PLP_TERMINATOR)) {
            return false;
        }
        PLPHeader *ph = (PLPHeader *) m_buffer.GetBuffer();
        if (ph->HEADER == BLOB_NULL_LEN) {
            m_buffer.Pop(sizeof (SPA::UINT64));
            m_out << (VARTYPE) VT_NULL;
            return true;
        } else if (ph->SUB_LEN + HEADER_SIZE > m_buffer.GetSize()) {
            return false;
        }
        PLPHeader plp_header;
        m_buffer >> plp_header;
        m_out << vt << (unsigned int) plp_header.HEADER;
        m_out.Push(m_buffer.GetBuffer(), plp_header.SUB_LEN);
        m_buffer.Pop(plp_header.SUB_LEN);
        plp_header.HEADER -= plp_header.SUB_LEN;
        if (plp_header.HEADER) {
            m_lenLarge = UNKNOWN_XML_LEN;
        } else if (m_buffer.GetSize() >= sizeof (PLP_TERMINATOR)) {
            m_buffer >> plp_header.SUB_LEN;
            assert(plp_header.SUB_LEN == PLP_TERMINATOR);
        }
        return true;
    }

    bool CSqlBatch::ConvertTo(const SPA::CDBString & pn) {
        unsigned short max_len;
        Collation collation;
        m_buffer >> max_len >> collation;
        if (max_len == VAR_MAX) {
            if (!PopPLP(VT_ARRAY | VT_I1)) {
                return false;
            }
        } else {
            unsigned short str_len;
            m_buffer >> str_len;
            if (str_len == VAR_MAX) {
                m_out << (VARTYPE) VT_NULL;
                return true;
            } else if (str_len > m_buffer.GetSize()) {
                return false;
            }
            auto pi = FindParameterInfo(pn);
            assert(pi);
            const char* head = (const char*) m_buffer.GetBuffer();
            m_out << pi->DataType;
            switch (pi->DataType) {
                case VT_I1:
                    break;
                case (VT_I1 | VT_ARRAY):
                    m_out << (unsigned int) str_len;
                    m_out.Push(head, str_len);
                    break;
                case VT_DATE:
                {
                    SPA::UDateTime dt;
                    dt.ParseFromDBString(head);
                    m_out << dt.time;
                }
                    break;
                default:
                    assert(false);
                    break;
            }
            m_buffer.Pop(str_len);
        }
        return true;
    }

    const SPA::UDB::CParameterInfo * CSqlBatch::FindParameterInfo(const SPA::CDBString & pn) const {
        for (auto it = m_vParamInfo.cbegin(), end = m_vParamInfo.cend(); it != end; ++it) {
            if (it->ParameterName == pn) {
                return &(*it);
            }
        }
        return nullptr;
    }

    int CSqlBatch::SaveParameter(unsigned char& packet_id, const SPA::UDB::CDBVariant& v, SPA::CUQueue& buffer, const SPA::UDB::CParameterInfo * pi) {
        tagDataType dt;
        unsigned char b_len = 0;
        unsigned char p_len = 0;
        buffer << p_len;
        unsigned char p_status = 0;
        int fail = 0;
        if (pi) {
            switch (pi->Direction) {
                case SPA::UDB::tagParameterDirection::pdOutput:
                case SPA::UDB::tagParameterDirection::pdInputOutput:
                    p_status = 1;
                    break;
                case SPA::UDB::tagParameterDirection::pdReturnValue:
                    assert(false); //not implemented yet
                default:
                    break;
            }
        }
        buffer << p_status;
        switch (v.vt) {
            case VT_CY:
                dt = tagDataType::MONEYN;
                b_len = sizeof (CY); //length;
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
                    int dt = ToTdsJDN(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
                    Date d;
                    d.Low = (unsigned short) (dt & USHORT_NULL_LEN);
                    d.High = (char) ((dt >> 16) & 0xff);
                    SPA::UINT64 time = tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;
                    time *= 1000000;
                    time += us;
                    unsigned int low = (unsigned int) (time & UINT_NULL_LEN);
                    time >>= 32;
                    unsigned char high = (unsigned char) (time & 0xff);
                    buffer << low << high << d;
                } else {
                    dt = tagDataType::TIMEN;
                    p_len = 6; //scale
                    b_len = 5; //length;
                    buffer << dt << p_len << b_len;
                    SPA::UINT64 time = tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;
                    time *= 1000000;
                    time += us;
                    unsigned int low = (unsigned int) (time & UINT_NULL_LEN);
                    time >>= 32;
                    unsigned char high = (unsigned char) (time & 0xff);
                    buffer << low << high;
                }
            }
                break;
            case VT_DECIMAL:
                dt = tagDataType::DECIMAL;
                if (pi && pi->Direction != SPA::UDB::tagParameterDirection::pdInput) {
                    p_len = pi->Precision;
                    if (p_len <= 19) {
                        b_len = 9;
                    } else {
                        b_len = 13;
                    }
                    buffer << dt << b_len << p_len << pi->Scale << b_len;
                } else {
                    b_len = v.decVal.Hi32 ? 13 : 9; //max length;
                    p_len = v.decVal.Hi32 ? 28 : 19; //precision;
                    buffer << dt << b_len << p_len << v.decVal.scale << b_len;
                }
                p_len = v.decVal.sign ? 0 : 1; //sign
                buffer << p_len << v.decVal.Lo64;
                if (b_len > 9) {
                    buffer << v.decVal.Hi32;
                }
                break;
            case VT_R4:
                dt = tagDataType::FLTN;
                b_len = sizeof (float);
                buffer << dt << b_len << b_len << v.fltVal;
                break;
            case VT_R8:
                dt = tagDataType::FLTN;
                b_len = sizeof (double);
                buffer << dt << b_len << b_len << v.dblVal;
                break;
            case VT_I4:
            case VT_UI4:
            case VT_INT:
            case VT_UINT:
                dt = tagDataType::INTN;
                b_len = sizeof (int);
                buffer << dt << b_len << b_len << v.intVal;
                break;
            case VT_I8:
            case VT_UI8:
                dt = tagDataType::INTN;
                b_len = sizeof (SPA::INT64);
                buffer << dt << b_len << b_len << v.llVal;
                break;
            case VT_I2:
            case VT_UI2:
                dt = tagDataType::INTN;
                b_len = sizeof (short);
                buffer << dt << b_len << b_len << v.iVal;
                break;
            case VT_BOOL:
                dt = tagDataType::BITN;
                b_len = sizeof (unsigned char);
            {
                unsigned char c = v.boolVal ? 1 : 0;
                buffer << dt << b_len << b_len << c;
            }
                break;
            case VT_I1:
            case VT_UI1:
                dt = tagDataType::INTN;
                b_len = sizeof (unsigned char);
                buffer << dt << b_len << b_len << v.bVal;
                break;
            case VT_NULL:
            case VT_EMPTY:
                if (pi && pi->DataType == VT_CLSID) {
                    dt = tagDataType::UNIQUEIDENTIFIER;
                    b_len = sizeof (CLSID);
                    p_len = 0;
                    buffer << dt << b_len << p_len;
                } else if (pi && pi->DataType == SPA::VT_XML) {
                    dt = tagDataType::XML;
                    p_len = 0;
                    buffer << dt << p_len << BLOB_NULL_LEN;
                } else {
                    unsigned short len;
                    if (pi) {
                        if (pi->DataType == VT_BSTR) {
                            dt = tagDataType::NVARCHAR;
                            if (pi->ColumnSize <= 4000) {
                                len = pi->ColumnSize;
                                len <<= 1;
                                buffer << dt << len << m_collation;
                            } else {
                                len = VAR_MAX;
                                buffer << dt << len << m_collation << BLOB_NULL_LEN;
                                return true;
                            }
                        } else if (pi->DataType == (VT_ARRAY | VT_I1)) {
                            dt = tagDataType::VARCHAR;
                            if (pi->ColumnSize <= 8000) {
                                len = pi->ColumnSize;
                                buffer << dt << len << m_collation;
                            } else {
                                len = VAR_MAX;
                                buffer << dt << len << m_collation << BLOB_NULL_LEN;
                                return true;
                            }
                        } else if (pi->DataType == (VT_ARRAY | VT_UI1)) {
                            dt = tagDataType::VARBINARY;
                            if (pi->ColumnSize <= 8000) {
                                len = pi->ColumnSize;
                                buffer << dt << len;
                            } else {
                                len = VAR_MAX;
                                buffer << dt << len << BLOB_NULL_LEN;
                                return true;
                            }
                        } else {
                            dt = tagDataType::VARCHAR;
                            len = 64;
                            buffer << dt << len << m_collation;
                        }
                    } else {
                        dt = tagDataType::VARCHAR;
                        len = 2;
                        buffer << dt << len << m_collation;
                    }
                    buffer << USHORT_NULL_LEN;
                }
                break;
            case VT_BSTR:
                if (pi && pi->DataType == SPA::VT_XML) {
                    dt = tagDataType::XML;
                    p_len = 0;
                    buffer << dt << p_len;
                    unsigned int len = SysStringLen(v.bstrVal);
                    len <<= 1;
                    fail = SavePLP((const unsigned char*) v.bstrVal, len, buffer, packet_id);
                } else {
                    dt = tagDataType::NVARCHAR;
                    unsigned short max = VAR_MAX;
                    unsigned int len = SysStringLen(v.bstrVal);
                    len <<= 1;
                    if (pi && pi->Direction != SPA::UDB::tagParameterDirection::pdInput && pi->ColumnSize <= 4000) {
                        max = (unsigned short) pi->ColumnSize;
                        max <<= 1;
                    }
                    if (len > 4000) {
                        max = VAR_MAX;
                    } else if (len > max) {
                        max = len;
                    }
                    buffer << dt << max << m_collation;
                    if (max == VAR_MAX) {
                        fail = SavePLP((const unsigned char*) v.bstrVal, len, buffer, packet_id);
                    } else {
                        buffer << (unsigned short) len;
                        buffer.Push(v.bstrVal, len >> 1);
                    }
                }
                break;
            case (VT_I1 | VT_ARRAY):
                dt = tagDataType::VARCHAR;
            {
                const char* s;
                unsigned short max = VAR_MAX;
                unsigned int len = v.parray->rgsabound[0].cElements;
                if (pi && pi->Direction != SPA::UDB::tagParameterDirection::pdInput && pi->ColumnSize <= 8000) {
                    max = (unsigned short) pi->ColumnSize;
                }
                if (len > 8000) {
                    max = VAR_MAX;
                } else if (len > max) {
                    max = len;
                }
                buffer << dt << max << m_collation;
                SafeArrayAccessData(v.parray, (void**) &s);
                if (max == VAR_MAX) {
                    fail = SavePLP((const unsigned char*) s, len, buffer, packet_id);
                } else {
                    buffer << (unsigned short) len;
                    buffer.Push(s, len);
                }
                SafeArrayUnaccessData(v.parray);
            }
                break;
            case VT_CLSID:
            case (VT_UI1 | VT_ARRAY):
            {
                const unsigned char* s;
                unsigned int len = v.parray->rgsabound[0].cElements;
                SafeArrayAccessData(v.parray, (void**) &s);
                if (v.VtExt == SPA::UDB::tagVTExt::vteGuid) {
                    dt = tagDataType::UNIQUEIDENTIFIER;
                    b_len = sizeof (GUID);
                    buffer << dt << b_len << b_len;
                    buffer.Push(s, b_len);
                } else {
                    dt = tagDataType::VARBINARY;
                    unsigned short max = VAR_MAX;
                    if (pi && pi->Direction != SPA::UDB::tagParameterDirection::pdInput && pi->ColumnSize <= 8000) {
                        max = (unsigned short) pi->ColumnSize;
                    }
                    if (len > 8000) {
                        max = VAR_MAX;
                    } else if (len > max) {
                        max = len;
                    }
                    buffer << dt << max;
                    if (max == VAR_MAX) {
                        fail = SavePLP(s, len, buffer, packet_id);
                    } else {
                        buffer << (unsigned short) len;
                        buffer.Push(s, len);
                    }
                }
                SafeArrayUnaccessData(v.parray);
            }
                break;
            default:
                assert(false);
                return SPA::Odbc::ER_DATA_TYPE_NOT_SUPPORTED;
        }
        return fail;
    }

    int CSqlBatch::SendTDSMessage(const SqlLogin& rec, FeatureExtension requestedFeatures, bool sync) {
        m_affects = 0;
        SPA::CDBString userName;
        std::vector<unsigned char> encryptedPassword;
        unsigned short encryptedPasswordLengthInBytes = 0;
        if (rec.credential.UserId.size() || rec.credential.Password.size()) {
            userName = rec.credential.UserId;
            encryptedPasswordLengthInBytes = (unsigned short) (rec.credential.Password.size() << 1);
        } else {
            userName = rec.userName;
            if (rec.password.size()) {
                encryptedPassword = ObfuscatePassword(rec.password);
                encryptedPasswordLengthInBytes = (unsigned short) encryptedPassword.size();
            }
        }
        unsigned short encryptedChangePasswordLengthInBytes = 0;
        std::vector<unsigned char> encryptedChangePassword;
        if (rec.newSecurePassword.size()) {
            encryptedChangePasswordLengthInBytes = (unsigned short) (rec.newSecurePassword.size() << 1);
        } else if (rec.newPassword.size()) {
            encryptedChangePassword = ObfuscatePassword(rec.newPassword);
            encryptedChangePasswordLengthInBytes = (unsigned short) encryptedChangePassword.size();
        }

        SPA::CScopeUQueue sbFeature, sbData;

        // length in bytes
        unsigned int length = YUKON_LOG_REC_FIXED_LEN;
        SPA::CDBString clientInterfaceName = ApplicationName;

        length += (unsigned int) ((rec.hostName.size() + rec.applicationName.size() +
                rec.serverName.size() + clientInterfaceName.size() +
                rec.language.size() + rec.database.size() +
                rec.attachDBFilename.size()) << 1);
        if (requestedFeatures.GetValue()) {
            length += 4;
        }
        std::vector<unsigned char> outSSPIBuff;
        unsigned int outSSPILength = 0;
#ifndef WIN32_64
        rec.useSSPI = false;
#endif
        m_bConnected = true;
        // only add lengths of password and username if not using SSPI or requesting federated authentication info
        if (!rec.useSSPI /*&& !(_connHandler._federatedAuthenticationInfoRequested || _connHandler._federatedAuthenticationRequested)*/) {
            length += (unsigned int) (userName.size() << 1) + encryptedPasswordLengthInBytes + encryptedChangePasswordLengthInBytes;
        } else {
#ifdef WIN32_64
            if (rec.useSSPI) {
                m_bConnected = false;
                m_sspi.reset(new CSspi);
                char serverName[256] = {0};
                m_channel.GetServerName(serverName, sizeof (serverName));
                SPA::CDBString tn = u"MSSQLSvc/" + SPA::CDBString(serverName, serverName + strlen(serverName));
                SPA::CScopeUQueue sbSspi;
                outSSPILength = sbSspi->GetMaxSize();
                SECURITY_STATUS ss = m_sspi->QuerySecurityContext(tn.c_str(), nullptr, 0, (unsigned char*) sbSspi->GetBuffer(), outSSPILength);
                if (ss < 0) {
                    return ER_NO_SSPI_CONTEXT_AVAILABLE;
                }
                const char* str = (const char*) sbSspi->GetBuffer();
                const char *found = strstr(str, "NTLMSSP");
                if (str == found) {
                    return ER_NO_SSPI_CONTEXT_AVAILABLE;
                }
                const unsigned char* start = sbSspi->GetBuffer();
                outSSPIBuff.assign(start, start + outSSPILength);
                length += outSSPILength;
            }
#endif
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
        Option1.fSetLang = 0;
        OptionalFlags2 Option2;
        Option2.fLanguage = 0;
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

        unsigned short str_len, offset = (unsigned short) YUKON_LOG_REC_FIXED_LEN;
        str_len = (unsigned short) rec.hostName.size();
        sb << offset << str_len;
        offset += (str_len << 1);

        if (!rec.useSSPI /*&& !(_connHandler._federatedAuthenticationInfoRequested || _connHandler._federatedAuthenticationRequested)*/) {
            str_len = (unsigned short) userName.size();
            sb << offset << str_len;
            offset += (str_len << 1);

            str_len = (encryptedPasswordLengthInBytes >> 1);
            sb << offset << str_len;
            offset += (str_len << 1);
        } else {
            SPA::UINT64 not_used = 0;
            sb << not_used;
        }
        str_len = (unsigned short) rec.applicationName.size();
        sb << offset << str_len;
        offset += (str_len << 1);

        str_len = (unsigned short) rec.serverName.size();
        sb << offset << str_len;
        offset += (str_len << 1);

        sb << offset;
        if (requestedFeatures.GetValue()) {
            // length of ibFeatgureExtLong (which is a DWORD)
            str_len = 4;
            sb << str_len;
            offset += 4;
        } else {
            str_len = 0;
            // unused
            sb << str_len;
        }

        str_len = (unsigned short) clientInterfaceName.size();
        sb << offset << str_len;
        offset += (str_len << 1);

        str_len = (unsigned short) rec.language.size();
        sb << offset << str_len;
        offset += (str_len << 1);

        str_len = (unsigned short) rec.database.size();
        sb << offset << str_len;
        offset += (str_len << 1);

        //ClientID
        assert(TDS_NIC_ADDRESS.size() == 6);
        sb->Push(TDS_NIC_ADDRESS.data(), (unsigned int) TDS_NIC_ADDRESS.size());

        sb << offset;
        if (rec.useSSPI) {
            sb << (unsigned short) outSSPILength;
            offset += outSSPILength;
        } else {
            str_len = 0;
            sb << str_len;
        }

        str_len = (unsigned short) rec.attachDBFilename.size();
        sb << offset << str_len;
        offset += (str_len << 1);

        //reset password offset
        str_len = (unsigned short) (encryptedChangePasswordLengthInBytes >> 1);
        sb << offset << str_len;

        // reserved for chSSPI
        sb << (unsigned int) 0;

        // write variable length portion
        sb->Push((const unsigned char*) rec.hostName.c_str(), (unsigned int) (rec.hostName.size() << 1));

        // if we are using SSPI, do not send over username/password, since we will use SSPI instead
        // same behavior as Luxor
        if (!rec.useSSPI /*&& !(_connHandler._federatedAuthenticationInfoRequested || _connHandler._federatedAuthenticationRequested)*/) {
            sb->Push((const unsigned char*) userName.c_str(), (unsigned int) (userName.size() << 1));
            if (rec.credential.UserId.size() || rec.credential.Password.size()) {
                sb->Push((const unsigned char*) rec.credential.Password.c_str(), (unsigned int) (rec.credential.Password.size() << 1));
            } else {
                sb->Push(encryptedPassword.data(), encryptedPasswordLengthInBytes);
            }
        }
        sb->Push((const unsigned char*) rec.applicationName.c_str(), (unsigned int) (rec.applicationName.size() << 1));
        sb->Push((const unsigned char*) rec.serverName.c_str(), (unsigned int) (rec.serverName.size() << 1));
        if (requestedFeatures.GetValue()) {
            sb << feOffset;
        }
        sb->Push((const unsigned char*) clientInterfaceName.c_str(), (unsigned int) (clientInterfaceName.size() << 1));
        sb->Push((const unsigned char*) rec.language.c_str(), (unsigned int) (rec.language.size() << 1));
        sb->Push((const unsigned char*) rec.database.c_str(), (unsigned int) (rec.database.size() << 1));
        // send over SSPI data if we are using SSPI
        if (rec.useSSPI) {
            sb->Push(outSSPIBuff.data(), (unsigned int) outSSPIBuff.size());
        }
        sb->Push((const unsigned char*) rec.attachDBFilename.c_str(), (unsigned int) (rec.attachDBFilename.size() << 1));
        if (!rec.useSSPI/* && !(_connHandler._federatedAuthenticationInfoRequested || _connHandler._federatedAuthenticationRequested)*/) {
            if (rec.newSecurePassword.size()) {
                sb->Push((const unsigned char*) rec.newSecurePassword.c_str(), (unsigned int) (rec.newSecurePassword.size() << 1));
            } else {
                sb->Push(encryptedChangePassword.data(), (unsigned int) encryptedChangePasswordLengthInBytes);
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
        PacketHeader* pHeader = (PacketHeader*) sb->GetBuffer();
        pHeader->Length = ChangeEndian((Packet_Length) sb->GetSize());
        m_vInfo.clear();
        m_timeout = rec.timeout;
        int fail = Send(sb->GetBuffer(), sb->GetSize(), m_timeout, sync);
        if (fail) {
            m_bConnected = false;
        }
        return fail;
    }

    SPA::UINT64 CSqlBatch::GetAffected() const {
        return m_affects;
    }

    int CSqlBatch::Cancel(bool completed, bool sync) {
        PacketHeader ph(tagPacketType::ptAttention, 1);
        ph.Length = ChangeEndian((unsigned short) sizeof (ph));
        ph.Spid = ChangeEndian(GetResponseHeader().Spid);
        if (completed) {
            ph.Status = tagPacketStatus::psEOM;
        } else {
            ph.Status = (tagPacketStatus) 0x03; //(0x01 | 0x02)
        }
        return Send((const unsigned char*) &ph, sizeof (ph), m_timeout, sync);
    }

    int CSqlBatch::SendTDSMessage(const SPA::UDB::CDBVariant* pVt, unsigned int count, bool sync) {
        m_affects = 0;
        if (!pVt || !count) {
            return SPA::Odbc::ER_NO_PARAMETER_SPECIFIED;
        }
        unsigned int parameters = m_inputs + m_outputs;
        if (!m_sqlPrepare.size() || !parameters) {
            return SPA::Odbc::ER_NO_PARAMETER_SPECIFIED;
        }
        if ((count % parameters)) {
            return SPA::Odbc::ER_BAD_PARAMETER_COLUMN_SIZE;
        }
        if (m_vParamInfo.size() && m_vParamInfo.size() != parameters) {
            return SPA::Odbc::ER_BAD_PARAMETER_COLUMN_SIZE;
        }
        if (m_vParamInfo.size()) {
            for (unsigned int n = 0; n < count; ++n) {
                SPA::UDB::CParameterInfo* pi = m_vParamInfo.data() + (n % parameters);
                if (pi && pi->Direction != SPA::UDB::tagParameterDirection::pdInput && pVt[n].vt > VT_NULL) {
                    if (pi->DataType != pVt[n].vt) {
                        if (pi->DataType == SPA::VT_XML) {
                            if (pVt[n].vt != VT_BSTR) {
                                return ER_BAD_OUTPUT_PARAMETER_DATA_TYPE;
                            }
                        } else if (pi->DataType == VT_CLSID) {
                            if (pVt[n].vt != (VT_ARRAY | VT_UI1) || pVt[n].parray->rgsabound[0].cElements != sizeof (CLSID)) {
                                return ER_BAD_OUTPUT_PARAMETER_DATA_TYPE;
                            }
                        } else {
                            return ER_BAD_OUTPUT_PARAMETER_DATA_TYPE;
                        }
                    }
                }
            }
        }
        unsigned int cycles = count / parameters;
        SPA::CDBString str = m_sqlPrepare;
        SPA::CDBString p0, p1, s;
        p0.reserve(16);
        p1.reserve(16);
        s.reserve(str.size() + 10 * parameters);
        char16_t p_num[16];
        for (unsigned int n = 1; n < cycles; ++n) {
            str.push_back(';');
            size_t pos = 0;
            s = m_sqlPrepare;
            for (unsigned m = 0; m < parameters; ++m) {
                p0 = u"@p";
                unsigned char chars = sizeof (p_num) / sizeof (char16_t);
                const char16_t* ret = SPA::ToString(m, p_num, chars);
                p0.append(ret, chars);
                p1 = u"@p";
                chars = sizeof (p_num) / sizeof (char16_t);
                ret = SPA::ToString(m + n * parameters, p_num, chars);
                p1.append(ret, chars);
                pos = s.find(p0, pos);
                s.replace(pos, p0.size(), p1);
                pos += 4;
            }
            str += s;
        }
        SPA::CScopeUQueue sb;
        //Query packet
        TransactionDescriptor td(m_tc.NewValue);
        sb << td;
        constexpr unsigned short p_name_length = USHORT_NULL_LEN;
        sb << p_name_length;
        constexpr unsigned short s_proc_id = 10; //sp_executesql
        sb << s_proc_id;
        constexpr unsigned short optionFlags = 0; //no meta data
        sb << optionFlags;
        int fail = 0;
        unsigned char packet_id = 1;

        constexpr unsigned char name_len = 0, status = 0;
        constexpr tagDataType dt = tagDataType::NVARCHAR;
        constexpr unsigned short max_len = VAR_MAX;

        sb << name_len << status << dt << max_len << m_collation;
        fail = SavePLP((const unsigned char*) str.c_str(), (unsigned int) (str.size() << 1), *sb, packet_id);
        if (fail) {
            return fail;
        }
        //
        sb << name_len << status << dt << max_len << m_collation;
        str.clear();
        fail = ToString(pVt, count, str);
        if (fail) {
            return fail;
        }
        fail = SavePLP((const unsigned char*) str.c_str(), (unsigned int) (str.size() << 1), *sb, packet_id);
        if (fail) {
            return fail;
        }

        for (unsigned int n = 0; n < count; ++n) {
            const SPA::UDB::CParameterInfo* pi = nullptr;
            if (m_vParamInfo.size()) {
                pi = m_vParamInfo.data() + (n % parameters);
            }
            fail = SaveParameter(packet_id, pVt[n], *sb, pi);
            if (fail) {
                return fail;
            }
            while (sb->GetSize() >= PACKET_DATA_SIZE) {
                fail = SendARpcPacket(*sb, packet_id);
                if (fail) {
                    return fail;
                }
            }
        }

        PacketHeader ph(tagPacketType::ptRpc, packet_id);
        ph.Length = (Packet_Length) (sb->GetSize() + sizeof (ph));
        ph.Length = ChangeEndian(ph.Length);
        ph.Spid = ChangeEndian(GetResponseHeader().Spid);
        SPA::CScopeUQueue sbEnd;
        sbEnd << ph;
        sbEnd->Push(sb->GetBuffer(), sb->GetSize());
        return Send(sbEnd->GetBuffer(), sbEnd->GetSize(), m_timeout, sync);
    }

    int CSqlBatch::SendARpcPacket(SPA::CUQueue& buffer, unsigned char& packet_id) {
        assert(buffer.GetSize() >= PACKET_DATA_SIZE);
        PacketHeader ph(tagPacketType::ptRpc, packet_id++);
        ph.Length = ChangeEndian(DEFAULT_PACKET_SIZE);
        ph.Spid = ChangeEndian(GetResponseHeader().Spid);
        ph.Status = tagPacketStatus::psNormal;
        SPA::CScopeUQueue sb;
        sb << ph;
        sb->Push(buffer.GetBuffer(), PACKET_DATA_SIZE);
        buffer.Pop(PACKET_DATA_SIZE);
        assert(sb->GetSize() == DEFAULT_PACKET_SIZE);
        return Send(sb->GetBuffer(), DEFAULT_PACKET_SIZE, 0, false);
    }

    int CSqlBatch::SavePLP(const unsigned char* buffer, unsigned int bytes, SPA::CUQueue& q, unsigned char& packet_id) {
        int fail = 0;
        q << (SPA::UINT64) bytes;
        while (q.GetSize() >= PACKET_DATA_SIZE) {
            fail = SendARpcPacket(q, packet_id);
            if (fail) {
                return fail;
            }
        }
        constexpr unsigned int MAX_SUB_SIZE = PACKET_DATA_SIZE - sizeof (bytes);
        while (bytes + q.GetSize() >= MAX_SUB_SIZE) {
            unsigned int sub_bytes = (bytes > MAX_SUB_SIZE) ? MAX_SUB_SIZE : bytes;
            q << sub_bytes;
            q.Push(buffer, sub_bytes);
            buffer += sub_bytes;
            bytes -= sub_bytes;
            fail = SendARpcPacket(q, packet_id);
            if (fail) {
                return fail;
            }
        }
        if (bytes) {
            q << bytes;
            q.Push(buffer, bytes);
        }
        q << PLP_TERMINATOR;
        return 0;
    }

    int CSqlBatch::SendTDSMessage(const char16_t * sql, unsigned int chars, bool sync) {
        m_affects = 0;
        unsigned char packet_id = 1;
        SPA::CScopeUQueue sb;
        TransactionDescriptor td(m_tc.NewValue);
        sb << td;
        if (chars == SPA::UQUEUE_NULL_LENGTH) {
            chars = (unsigned int) SPA::GetLen(sql);
        }
        sb->Push((const unsigned char*) sql, chars << 1);
        SPA::CScopeUQueue sbEnd;
        while (sb->GetSize() >= PACKET_DATA_SIZE) {
            PacketHeader ph(tagPacketType::ptBatch, packet_id++);
            ph.Length = ChangeEndian(DEFAULT_PACKET_SIZE);
            ph.Spid = ChangeEndian(GetResponseHeader().Spid);
            ph.Status = tagPacketStatus::psNormal;
            sbEnd << ph;
            sbEnd->Push(sb->GetBuffer(), PACKET_DATA_SIZE);
            assert(sbEnd->GetSize() == DEFAULT_PACKET_SIZE);
            int fail = Send(sbEnd->GetBuffer(), DEFAULT_PACKET_SIZE, 0, false);
            if (fail) {
                return fail;
            }
            sb->Pop(PACKET_DATA_SIZE);
            sbEnd->SetSize(0);
        }
        PacketHeader ph(tagPacketType::ptBatch, packet_id);
        ph.Spid = ChangeEndian(GetResponseHeader().Spid);
        sbEnd << ph;
        sbEnd->Push(sb->GetBuffer(), sb->GetSize());
        PacketHeader* pHeader = (PacketHeader*) sbEnd->GetBuffer();
        pHeader->Length = ChangeEndian((Packet_Length) sbEnd->GetSize());
        return Send(sbEnd->GetBuffer(), sbEnd->GetSize(), m_timeout, sync);
    }

    bool CSqlBatch::ParseDoneInProc() {
        if (m_buffer.GetSize() >= sizeof (m_dip)) {
            m_buffer >> m_dip;
            if (m_vCol.size() == 0 && (m_dip.Status & tagDoneStatus::dsCount) == tagDoneStatus::dsCount) {
                m_affects += m_dip.RowCount;
            }
            m_tt = tagTokenType::ttZero;
            return true;
        }
        return false;
    }

    bool CSqlBatch::ParseReturnStatus() {
        if (m_buffer.GetSize() >= sizeof (m_rs)) {
            m_buffer >> m_rs;
            m_tt = tagTokenType::ttZero;
            return true;
        }
        return false;
    }

    bool CSqlBatch::ParseCollation(CollationChange & cc) {
        if (m_buffer.GetSize() >= sizeof (cc) + sizeof (unsigned char) + sizeof (unsigned char)) {
            unsigned char b;
            m_buffer >> b;
            assert(b == 5);
            m_buffer >> cc.NewValue;
            m_buffer >> b;
            if (b) {
                m_buffer >> cc.OldValue;
            } else {
                Collation initial;
                cc.OldValue = initial;
            }
            m_tt = tagTokenType::ttZero;
            return true;
        }
        return false;
    }

    void CSqlBatch::ParseStringChange(tagEnvchangeType type, StringEventChange & sec) {
        unsigned char b;
        m_buffer >> b;
        sec.Type = type;
        const char16_t* str = (const char16_t*) m_buffer.GetBuffer();
        sec.NewValue.assign(str, str + b);
        SPA::ToLower(sec.NewValue);
        m_buffer.Pop(((unsigned int) b) << 1);
        m_buffer >> b;
        if (b) {
            str = (const char16_t*) m_buffer.GetBuffer();
            sec.OldValue.assign(str, str + b);
            SPA::ToLower(sec.OldValue);
            m_buffer.Pop(((unsigned int) b) << 1);
        }
    }

    bool CSqlBatch::ParseInfo() {
        if (m_buffer.GetSize() > 2) {
            unsigned short len = *(unsigned short*) m_buffer.GetBuffer();
            if (len + sizeof (len) <= m_buffer.GetSize()) {
                TokenInfo ti;
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
            SPA::UDB::CDBColumnInfo ci;
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
                        ci.Collation = collation.GetString();
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
                        ci.Collation = collation.GetString();
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
                    SPA::CDBString schema, assembly;
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
                        SPA::CDBString ownerName, collection;
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
                ci.Flags |= SPA::UDB::CDBColumnInfo::FLAG_XML;
            }
            if (!m_meta) {
                m_vCol.push_back(std::move(ci));
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

    bool CSqlBatch::ParseVariant(SPA::UDB::CDBColumnInfo * cinfo) {
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
                return ParseData(dt, cinfo->ColumnSize == USHORT_NULL_LEN);
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
                return ParseData(dt, cinfo->ColumnSize == USHORT_NULL_LEN);
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
        SPA::UDB::CDBColumnInfo *cinfo = m_vCol.data();
        tagDataType *pdt = m_vDT.data();
        if (m_posCol == INVALID_COL) {
            m_posCol = 0;
        }
        cinfo += m_posCol;
        for (unsigned short n = m_posCol; n < cols; ++n, ++cinfo, ++m_posCol) {
            if (!done) {
                if (m_buffer.GetSize() <= sizeof (PacketHeader) + sizeof (m_Done) + sizeof (tagTokenType)) {
                    return false;
                }
            }
            tagDataType dt = pdt[n];
            unsigned char nullable = (!(cinfo->Flags & SPA::UDB::CDBColumnInfo::FLAG_NOT_NULL));
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
                    if (!ParseData(dt, false)) {
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
                    if (!ParseData(dt, false)) {
                        return false;
                    }
                    break;
                case tagDataType::UDT:
                case tagDataType::XML:
                {
                    unsigned int len = *(unsigned int*) m_buffer.GetBuffer();
                    if (len == UINT_NULL_LEN) {
                        m_out << (VARTYPE) VT_NULL;
                        m_buffer.Pop(4);
                        continue;
                    }
                    if (!ParseData(dt, false)) {
                        return false;
                    }
                }
                    break;
                case tagDataType::NVARCHAR:
                case tagDataType::NCHAR:
                case tagDataType::CHAR:
                case tagDataType::VARCHAR:
                case tagDataType::VARBINARY:
                case tagDataType::BINARY:
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
                    if (!ParseData(dt, cinfo->ColumnSize == VAR_MAX)) {
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
                } else if (scale == 3 || scale == 4) {
                    unsigned int time;
                    m_buffer >> time >> dt;
                    ToDateTime(dt, time, scale, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
                } else {
                    unsigned short low;
                    unsigned char high;
                    m_buffer >> low >> high >> dt;
                    unsigned int time = high;
                    time <<= 16;
                    time += low;
                    ToDateTime(dt, time, scale, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
                }
                char datetime[32] = {0};
                tm.tm_year -= 1900;
                tm.tm_mon -= 1;
                SPA::UDateTime udt(tm, us);
                udt.ToDBString(datetime, sizeof (datetime));
                m_buffer >> offset;
                bool neg = false;
                if (offset < 0) {
                    offset = (-offset);
                    neg = true;
                }
                char os[8] = {0};
#ifdef WIN32_64
                int len = ::sprintf_s(os, sizeof (os), " %s%02d:%02d", neg ? "-" : "+", offset / 60, (offset % 60));
#else
                int len::sprintf(os, " %s%02d:%02d", neg ? "-" : "+", offset / 60, (offset % 60));
#endif
                unsigned int len0 = (unsigned int) ::strlen(datetime);
                m_out << (VARTYPE) (VT_ARRAY | VT_I1);
                m_out << (len0 + (unsigned int) len);
                m_out.Push((const unsigned char*) datetime, len0);
                m_out.Push((const unsigned char*) os, (unsigned int) len);
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
                } else if (bytes == 8 && scale == 0) { //datetime
                    DateTime dt;
                    m_buffer >> dt;
                    ToDateTime(dt, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
                } else {
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
                    } else if (scale == 3 || scale == 4) {
                        unsigned int time;
                        m_buffer >> time >> dt;
                        ToDateTime(dt, time, scale, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
                    } else {
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
                m_out << (VARTYPE) VT_DATE << udt.time;
            }
                break;
            case tagDataType::TIMEN:
            {
                unsigned int us;
                assert(scale <= 7);
                std::tm tm;
                ::memset(&tm, 0, sizeof (tm));
                if (scale >= 5 && scale <= 7) {
                    unsigned int low;
                    unsigned char high;
                    m_buffer >> low >> high;
                    SPA::UINT64 time = high;
                    time <<= 32;
                    time += low;
                    ToTime(time, scale, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
                } else if (scale == 3 || scale == 4) {
                    unsigned int time;
                    m_buffer >> time;
                    ToTime(time, scale, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
                } else {
                    unsigned short low;
                    unsigned char high;
                    m_buffer >> low >> high;
                    unsigned int time = high;
                    time <<= 16;
                    time += low;
                    ToTime(time, scale, tm.tm_hour, tm.tm_min, tm.tm_sec, us);
                }
                SPA::UDateTime udt(tm, us);
                m_out << (VARTYPE) VT_DATE << udt.time;
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

    bool CSqlBatch::ParseData(tagDataType dt, bool max) {
        VARTYPE vt = GetVarType(dt, 0);
        switch (dt) {
            case tagDataType::XML:
            {
                unsigned int remain;
                m_buffer >> m_lenLarge >> remain;
                assert(m_lenLarge == UNKNOWN_XML_LEN);
                assert(remain > 0 && remain <= DEFAULT_PACKET_SIZE - sizeof (m_lenLarge) - sizeof (remain));
                m_out << vt;
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
                unsigned char skipped_bytes;
                m_buffer >> skipped_bytes;
                m_buffer.Pop(skipped_bytes);
                UINT64 timestamp;
                m_buffer >> timestamp;
                unsigned int image_bytes;
                m_buffer >> image_bytes;
                m_lenLarge = image_bytes;
                m_out << vt << (unsigned int) m_lenLarge;
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
                unsigned int remain;
                m_buffer >> m_lenLarge >> remain;
                assert(remain <= DEFAULT_PACKET_SIZE - sizeof (m_lenLarge) - sizeof (remain));
                m_out << vt << (unsigned int) m_lenLarge;
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
                if (max) {
                    unsigned int remain;
                    m_buffer >> m_lenLarge >> remain;
                    assert(remain > 0 && remain <= DEFAULT_PACKET_SIZE - sizeof (m_lenLarge) - sizeof (remain));
                    m_out << vt << (unsigned int) m_lenLarge;
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
                    m_out << vt << (unsigned int) len;
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
        SPA::UDB::CDBColumnInfo *cinfo = m_vCol.data();
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
        cinfo += m_posCol;
        bool done = IsDone();
        for (unsigned short n = m_posCol; n < cols; ++n, ++m_posCol, ++cinfo) {
            if (!m_buffer.GetSize()) {
                return false;
            }
            bool is_null = (nulls[n >> 3] & (1 << (n % 8)));
            if (is_null) {
                m_out << (VARTYPE) VT_NULL;
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
                    if (!ParseData(dt, cinfo->ColumnSize == VAR_MAX)) {
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
        unsigned short len = *(unsigned short*) m_buffer.GetBuffer();
        if (len + sizeof (len) > m_buffer.GetSize()) {
            return false;
        }
        unsigned char b;
        m_buffer >> len >> b >> m_LoginAck.Tds_Version;
        assert(b == 1); //Interface
        m_buffer >> b; //byte length
        const char16_t* str = (const char16_t*) m_buffer.GetBuffer();
        m_LoginAck.ServerName.assign(str, str + b);
        m_buffer.Pop((unsigned int) (m_LoginAck.ServerName.size() << 1));
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
            if (m_buffer.GetSize() >= sizeof (len)) {
                len = *(unsigned int*) m_buffer.GetBuffer();
                if (len == 0) {
                    m_buffer.Pop(sizeof (len));
                    m_lenLarge = 0;
                    if (m_vCol.size()) {
                        ++m_posCol;
                        if (m_posCol == m_vCol.size()) {
                            m_posCol = INVALID_COL;
                            m_tt = tagTokenType::ttZero;
                        }
                    } else {
                        m_tt = tagTokenType::ttZero;
                    }
                } else {
                    return false;
                }
            } else {
                return false;
            }
        } else {
            if (m_lenLarge && (m_endLarge == MAX_IMAGE_TEXT_LEN || m_endLarge == MAX_NTEXT_LEN)) {
                unsigned int len = *(unsigned int*) m_buffer.GetBuffer();
                len = m_buffer.GetSize();
                if (len > m_lenLarge) {
                    len = (unsigned int) m_lenLarge;
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
                } else {
                    assert(!m_buffer.GetSize());
                }
            } else {
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
                    } else {
                        assert(m_lenLarge < 0x7fffffff);
                    }
                }
                if (!m_lenLarge && m_endLarge == UINT_NULL_LEN) {
                    if (m_buffer.GetSize() >= sizeof (m_endLarge)) {
                        m_buffer >> m_endLarge;
                        assert(!m_endLarge);
                    } else {
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
#ifdef WIN32_64
                case tagTokenType::ttSSPI:
                    if (!ParseSSPI()) {
                        return false;
                    }
                    break;
#endif
                case tagTokenType::ttORDER:
                    m_vOrder.clear();
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
                    if (!ParseError()) {
                        return false;
                    }
                    break;
                case tagTokenType::ttINFO:
                    if (!ParseInfo()) {
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
                case tagTokenType::ttRETURNVALUE:
                {
                    unsigned int hp = m_buffer.GetHeadPosition();
                    unsigned int size = m_out.GetSize();
                    if (!ParseReturnValue()) {
                        m_buffer.SetHeadPosition(hp);
                        m_out.SetSize(size);
                        return false;
                    }
                }
                    break;
                case tagTokenType::ttDONEPROC:
                case tagTokenType::ttDONE:
                    if (!ParseDone()) {
                        return false;
                    } else if (IsDone() && !HasMore() && m_buffer.GetSize()) {
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
                return false;
            }
        }
        return true;
    }

    void CSqlBatch::ParseTransChange(tagEnvchangeType type, TransChange & tc) {
        tc.Type = type;
        unsigned char len;
        m_buffer >> len;
        if (len) {
            m_buffer >> tc.NewValue;
        } else {
            tc.NewValue = 0;
        }
        m_buffer >> len;
        if (len) {
            m_buffer >> tc.OldValue;
        } else {
            tc.OldValue = 0;
        }
    }

    int CSqlBatch::SendTDSMessage(tagRequestType rt, tagIsolationLevel il, bool sync) {
        m_affects = 0;
        switch (rt) {
            case tagRequestType::rtBeginTrans:
                if (m_tc.NewValue) {
                    return SPA::Odbc::ER_BAD_MANUAL_TRANSACTION_STATE;
                }
                break;
            case tagRequestType::rtCommit:
            case tagRequestType::rtRollback:
                if (!m_tc.NewValue) {
                    return SPA::Odbc::ER_BAD_MANUAL_TRANSACTION_STATE;
                }
                break;
            default:
                assert(false);
                break;
        }
        TransactionDescriptor td(m_tc.NewValue);
        PacketHeader ph(tagPacketType::ptTransaction, 1);
        ph.Spid = ChangeEndian(GetResponseHeader().Spid);
        ph.Length = (Packet_Length) (sizeof (ph) + sizeof (td) + sizeof (rt) + sizeof (il));
        ph.Length = ChangeEndian(ph.Length);
        SPA::CScopeUQueue sb;
        sb << ph << td << rt << il;
        return Send(sb->GetBuffer(), sb->GetSize(), m_timeout, sync);
    }

    bool CSqlBatch::ParseDone() {
        if (CReqBase::ParseDone()) {
            if (m_Done.Status == tagDoneStatus::dsFinal || (m_Done.Status & tagDoneStatus::dsMore) == tagDoneStatus::dsMore || (m_Done.Status & tagDoneStatus::dsCount) == tagDoneStatus::dsCount) {
                if (m_vCol.size() == 0 && (m_Done.Status & tagDoneStatus::dsCount) == tagDoneStatus::dsCount) {
                    m_affects += m_Done.RowCount;
                }
                m_posCol = INVALID_COL;
                m_cols = 0;
                m_vCol.clear();
                m_vDT.clear();
                m_vNull.clear();
                SPA::UDB::CDBVariant vt;
                while (m_out.GetSize()) {
                    m_out >> vt;
                }
            }
            return true;
        }
        return false;
    }

    bool CSqlBatch::ParseEventChange() {
        if (m_buffer.GetSize() > 2) {
            unsigned short len = *(unsigned short *) m_buffer.GetBuffer();
            if (len + sizeof (unsigned short) <= m_buffer.GetSize()) {
                tagEnvchangeType type;
                m_buffer >> len >> type;
                switch (type) {
                    case tagEnvchangeType::database:
                        m_dbNameChange.OldValue.clear();
                        ParseStringChange(type, m_dbNameChange);
                        break;
                    case tagEnvchangeType::packet_size:
                    case tagEnvchangeType::language:
                    {
                        StringEventChange sec;
                        m_vEventChange.push_back(sec);
                        ParseStringChange(type, m_vEventChange.back());
                    }
                        break;
                    case tagEnvchangeType::begin_trans:
                    case tagEnvchangeType::commit_trans:
                    case tagEnvchangeType::rollback_trans:
                        ParseTransChange(type, m_tc);
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

    bool CSqlBatch::IsTDSConnected() {
        return m_bConnected;
    }

#ifdef WIN32_64

    bool CSqlBatch::ParseSSPI() {
        unsigned short len;
        if (m_buffer.GetSize() <= sizeof (len)) {
            return false;
        }
        len = *(unsigned short*) m_buffer.GetBuffer();
        if (m_buffer.GetSize() < len + sizeof (len)) {
            return false;
        }
        char serverName[256] = {0};
        m_channel.GetServerName(serverName, sizeof (serverName));
        SPA::CDBString tn = u"MSSQLSvc/" + SPA::CDBString(serverName, serverName + strlen(serverName));
        SPA::CScopeUQueue sb;
        unsigned int cbOut = sb->GetMaxSize();
        SECURITY_STATUS ss = m_sspi->QuerySecurityContext(tn.c_str(), (unsigned char*) m_buffer.GetBuffer(sizeof (len)), len, (unsigned char*) sb->GetBuffer(), cbOut);
        if (ss < 0) {
            return false;
        }
        m_buffer.Pop(sizeof (len) + len);
        if (!m_sspi->IsDone() && cbOut) {
            PacketHeader ph(tagPacketType::ptSspi, 1);
            ph.Spid = ChangeEndian(GetResponseHeader().Spid);
            unsigned short length = (unsigned short) (sizeof (ph) + cbOut);
            ph.Length = ChangeEndian(length);
            SPA::CScopeUQueue sbEnd;
            sbEnd << ph;
            sbEnd->Push(sb->GetBuffer(), cbOut);
            m_channel.Send(this, sbEnd->GetBuffer(), sbEnd->GetSize());
        }
        if (ss == SEC_E_OK) {
            m_bConnected = true;
            m_sspi.reset();
        }
        m_tt = tagTokenType::ttZero;
        return true;
    }
#endif
}
