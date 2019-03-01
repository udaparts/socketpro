
#include "stdafx.h"
#include "phpmanager.h"

namespace PA
{

    const int64_t PHP_ADAPTER_SECRET = 0xf234567890ab;

    const char *PHP_BUFFER = "CUQueue";
    const char *PHP_CONN_CONTEXT = "CConnectionContext";
    const char *PHP_FILE_HANDLER = "CAsyncFile";
    const char *PHP_DB_HANDLER = "CAsyncDb";
    const char *PHP_QUEUE_HANDLER = "CAsyncQueue";
    const char *PHP_ASYNC_HANDLER = "CAsyncHandler";
    const char *PHP_SOCKET_POOL = "CSocketPool";
    const char *PHP_CERT = "CUCert";
    const char *PHP_TABLE = "CTable";
    const char *PHP_DATASET = "CDataSet";
    const char *PHP_SOCKET = "CClientSocket";
    const char *PHP_DB_COLUMN_INFO = "CDBColumnInfo";
    const char *PHP_DB_PARAMETER_INFO = "CParamInfo";
    const char *PHP_CONSTRUCT = "__construct";
    const char *PHP_MANAGER = "CManager";
    const char *PHP_PUSH = "CPush";
    const char *PHP_CLIENTQUEUE = "CClientQueue";
    const char *PHP_KEY = "key";
    const char *PHP_TIMEOUT = "timeout";
    const char *PHP_LEN = "len";
    const char *PHP_OBJ = "obj";
    const char *PHP_SIZE = "Size";
    const char *PHP_ERR_CODE = "ec";
    const char *PHP_ERR_MSG = "em";
    const char *PHP_DB_NAME = "dbName";
    const char *PHP_TABLE_NAME = "tableName";
    const char *PHP_COLUMN_NAME = "colName";
    const char *PHP_EMPTY = "Empty";
    const char *PHP_ORDINAL = "ordinal";
    const char *PHP_DATATYPE = "DataType";
    const char *PHP_COLUMN_SIZE = "ColumnSize";
    const char *PHP_COLUMN_FLAGS = "Flags";
    const char *PHP_COLUMN_PRECSISON = "Precision";
    const char *PHP_COLUMN_SCALE = "Scale";
    const char *PHP_COPYDATA = "copyData";
    const char *PHP_TABLE_OP = "op";
    const char *PHP_VARIANT_V0 = "v0";
    const char *PHP_VARIANT_V1 = "v1";
    const char *PHP_VARIANT_V = "v";
    const char *PHP_POINTER_ADDRESS = "TABLE_POINTER_ADDRESS";
    const char *PHP_WAITALL = "WaitAll";
    const char *PHP_STARTBATCHING = "StartBatching";
    const char *PHP_ABORTBATCHING = "AbortBatching";
    const char *PHP_COMMITBATCHING = "CommitBatching";
    const char *PHP_UNLOCK = "Unlock";
    const char *PHP_CLEAN_CALLBACKS = "CleanCallbacks";
    const char *PHP_POPBYTES = "PopBytes";

    //SendRequest
    const char *PHP_SENDREQUEST = "SendRequest";
    const char *PHP_SENDREQUEST_REQID = "reqId";
    const char *PHP_SENDREQUEST_BUFF = "buff";
    const char *PHP_SENDREQUEST_SYNC = "sync";
    const char *PHP_SENDREQUEST_CH = "ch";
    const char *PHP_SENDREQUEST_EX = "ex";

    const char *PHP_SOCKET_CLOSED = "Socket closed";
    const char *PHP_REQUEST_CANCELED = "Request canceled";
    const char *PHP_SERVER_EXCEPTION = "Remote server exception caught";
    const char *PHP_REQUEST_TIMEOUT = "Request timed out";

    const std::string SPA_NS("SPA\\");
    const std::string SPA_CS_NS("SPA\\ClientSide\\");

    std::string SP_CONFIG = "sp_config.json";

#ifdef WIN32_64
    const char SYS_DIR = '\\';
#else
    const char SYS_DIR = '/';
#endif

    void Trim(std::string & str) {
        while (str.size() && std::isspace(str.back())) {
            str.pop_back();
        }
        while (str.size() && std::isspace(str.front())) {
            str.erase(0, 1);
        }
    }

    void ToVariant(const Php::Value &data, CComVariant &vt, const std::string & id) {
        auto type = data.type();
        switch (type) {
            case Php::Type::Undefined:
            case Php::Type::Null:
                vt.Clear();
                vt.vt = VT_NULL;
                break;
            case Php::Type::Bool:
            case Php::Type::False:
            case Php::Type::True:
                vt = data.boolValue();
                break;
            case Php::Type::Numeric:
                if (id == "i" || id == "int") {
                    vt = (int) data.numericValue();
                } else if (id == "ui" || id == "uint" || id == "unsigned" || id == "unsigned int") {
                    vt = (unsigned int) data.numericValue();
                } else if (id == "ul" || id == "ulong" || id == "unsigned long long" || id == "unsigned long int") {
                    vt = (SPA::UINT64)data.numericValue();
                } else if (id == "s" || id == "short" || id == "w") {
                    vt = (short) data.numericValue();
                } else if (id == "c" || id == "char" || id == "a") {
                    vt = (char) data.numericValue();
                } else if (id == "b" || id == "byte" || id == "unsigned char") {
                    vt = (unsigned char) data.numericValue();
                } else if (id == "us" || id == "ushort" || id == "unsigned short") {
                    vt = (unsigned short) data.numericValue();
                } else if (id == "dec" || id == "decimal") {
                    vt.vt = VT_DECIMAL;
                    SPA::ToDecimal(data.numericValue(), vt.decVal);
                } else {
                    vt = data.numericValue();
                }
                break;
            case Php::Type::Float:
                if (id == "f" || id == "float") {
                    vt = (float) data.floatValue();
                } else if (id == "dec" || id == "decimal") {
                    vt.vt = VT_DECIMAL;
                    SPA::ToDecimal(data.floatValue(), vt.decVal);
                } else {
                    vt = data.floatValue();
                }
                break;
            case Php::Type::String:
            {
                vt.Clear();
                const char *raw = data.rawValue();
                if (!raw) {
                    vt.vt = VT_NULL;
                } else if (id == "a" || id == "ascii") {
                    vt = raw;
                } else if (id == "bytes" || id == "binary" || id == "uuid" || id == "clsid" || id == "guid") {
                    unsigned char *pBuffer;
                    vt.vt = (VT_ARRAY | VT_UI1);
                    SAFEARRAYBOUND sab[1] = {(unsigned int) data.length(), 0};
                    SAFEARRAY *psa = SafeArrayCreate(VT_UI1, 1, sab);
                    SafeArrayAccessData(psa, (void**) &pBuffer);
                    memcpy(pBuffer, raw, (unsigned int) data.length());
                    SafeArrayUnaccessData(psa);
                    vt.parray = psa;
                } else if (id == "dec" || id == "decimal") {
                    vt.vt = VT_DECIMAL;
                    SPA::ParseDec_long(raw, vt.decVal);
                } else {
                    vt.vt = VT_BSTR;
                    vt.bstrVal = SPA::Utilities::ToBSTR(raw, (unsigned int) data.length());
                }
            }
                break;
            case Php::Type::Object:
                if (Php::is_a(data, "DateTime")) {
                    vt.Clear();
                    Php::Value dt = data.call("format", "Y-m-d H:i:s.u");
                    SPA::UDateTime udt(dt.rawValue());
                    vt.ullVal = udt.time; //high precision datetime
                    vt.vt = VT_DATE;
                    break;
                } else {
                    throw Php::Exception("Unsupported data type");
                }
            default: //Php::Type::Array
            {
                CPhpBuffer pb;
                pb.SaveObject(data, "");
                *pb.m_pBuffer >> vt;
            }
                break;
        }
    }

    void ToVariant(const Php::Value &data, SPA::UDB::CDBVariant &vt, const std::string & id) {
        auto type = data.type();
        switch (type) {
            case Php::Type::Undefined:
            case Php::Type::Null:
                vt.Clear();
                vt.vt = VT_NULL;
                break;
            case Php::Type::Bool:
            case Php::Type::False:
            case Php::Type::True:
                vt = data.boolValue();
                break;
            case Php::Type::Numeric:
                if (id == "i" || id == "int") {
                    vt = (int) data.numericValue();
                } else if (id == "ui" || id == "uint" || id == "unsigned" || id == "unsigned int") {
                    vt = (unsigned int) data.numericValue();
                } else if (id == "ul" || id == "ulong" || id == "unsigned long long" || id == "unsigned long int") {
                    vt = (SPA::UINT64)data.numericValue();
                } else if (id == "s" || id == "short" || id == "w") {
                    vt = (short) data.numericValue();
                } else if (id == "c" || id == "char" || id == "a") {
                    vt = (char) data.numericValue();
                } else if (id == "b" || id == "byte" || id == "unsigned char") {
                    vt = (unsigned char) data.numericValue();
                } else if (id == "us" || id == "ushort" || id == "unsigned short") {
                    vt = (unsigned short) data.numericValue();
                } else if (id == "dec" || id == "decimal") {
                    vt.vt = VT_DECIMAL;
                    SPA::ToDecimal(data.numericValue(), vt.decVal);
                } else {
                    vt = data.numericValue();
                }
                break;
            case Php::Type::Float:
                if (id == "f" || id == "float") {
                    vt = (float) data.floatValue();
                } else if (id == "dec" || id == "decimal") {
                    vt.vt = VT_DECIMAL;
                    SPA::ToDecimal(data.floatValue(), vt.decVal);
                } else {
                    vt = data.floatValue();
                }
                break;
            case Php::Type::String:
            {
                vt.Clear();
                const char *raw = data.rawValue();
                if (!raw) {
                    vt.vt = VT_NULL;
                } else if (id == "a" || id == "ascii") {
                    vt = raw;
                } else if (id == "bytes" || id == "binary") {
                    unsigned char *pBuffer;
                    vt.vt = (VT_ARRAY | VT_UI1);
                    SAFEARRAYBOUND sab[1] = {(unsigned int) data.length(), 0};
                    SAFEARRAY *psa = SafeArrayCreate(VT_UI1, 1, sab);
                    SafeArrayAccessData(psa, (void**) &pBuffer);
                    memcpy(pBuffer, raw, (unsigned int) data.length());
                    SafeArrayUnaccessData(psa);
                    vt.parray = psa;
                } else if ((id == "uuid" || id == "clsid" || id == "guid") && data.length() == sizeof (GUID)) {
                    vt = *((GUID*) raw);
                } else if (id == "dec" || id == "decimal") {
                    vt.vt = VT_DECIMAL;
                    SPA::ParseDec_long(raw, vt.decVal);
                } else {
                    vt.vt = VT_BSTR;
                    vt.bstrVal = SPA::Utilities::ToBSTR(raw, (unsigned int) data.length());
                }
            }
                break;
            case Php::Type::Object:
                if (Php::is_a(data, "DateTime")) {
                    Php::Value dt = data.call("format", "Y-m-d H:i:s.u");
                    SPA::UDateTime udt(dt.rawValue());
                    vt = udt;
                    break;
                } else {
                    throw Php::Exception("Unsupported data type");
                }
            default: //Php::Type::Array
            {
                CPhpBuffer pb;
                pb.SaveObject(data, "");
                *pb.m_pBuffer >> vt;
            }
                break;
        }
    }

    Php::Value GetManager(Php::Parameters & params) {
        return CPhpManager::Parse();
    }

    Php::Value GetSpPool(Php::Parameters & params) {
        CPhpManager::Parse();
        Php::Value pool = CPhpManager::Manager.GetPool(params[0], PHP_ADAPTER_SECRET);
        if (pool.type() == Php::Type::Null) {
            throw Php::Exception(CPhpManager::Manager.GetErrorMsg());
        }
        return pool;
    }

    Php::Value GetSpHandler(Php::Parameters & params) {
        Php::Value pool = GetSpPool(params);
        return pool.call("Seek");
    }

    Php::Value LockSpHandler(Php::Parameters & params) {
        Php::Value pool = GetSpPool(params);
        if (pool.type() == Php::Type::Null) {
            throw Php::Exception(CPhpManager::Manager.GetErrorMsg());
        }
        if (params.size() > 1) {
            if (!params[1].isNumeric()) {
                throw Php::Exception("An integer number expected");
            }
            return pool.call("Lock", params[1]);
        }
        return pool.call("Lock", -1); //default to infinite time
    }

    Php::Value SpBuff(Php::Parameters & params) {
        return Php::Object((SPA_NS + PHP_BUFFER).c_str(), new CPhpBuffer);
    }

} //namespace PA
