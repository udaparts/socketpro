
#include "stdafx.h"
#include <cctype>
#include "phpbuffer.h"
#include <memory>
#include "phpmanager.h"

namespace SPA {
	namespace ClientSide {
		using namespace PA;
		Php::Value CAsyncServiceHandler::SendRequest(Php::Parameters &params) {
			bool canceled = false;
			bool exception = false;
			bool closed = false;
			unsigned int timeout = GetAttachedClientSocket()->GetRecvTimeout();
			bool sync = false;
			int64_t id = params[0].numericValue();
			if (id <= SPA::sidReserved || id > 0xffff) {
				throw Php::Exception("Bad request id");
			}
			unsigned short reqId = (unsigned short)id;
			ResultHandler rh;
			DDiscarded discarded;
			DServerException se;
			const unsigned char *pBuffer = nullptr;
			unsigned int bytes = 0;
			Php::Value phpRh, phpCanceled, phpEx;
			size_t args = params.size();
			if (args > 2) {
				phpRh = params[2];
				if (phpRh.isNumeric()) {
					sync = true;
					unsigned int t = (unsigned int)phpRh.numericValue();
					if (t < timeout) {
						timeout = t;
					}
				}
				else if (phpRh.isBool()) {
					sync = phpRh.boolValue();
				}
				else if (!phpRh.isCallable()) {
					throw Php::Exception("A callback required for returning result");
				}
			}
			rh = [phpRh, sync, this](CAsyncResult & ar) {
				PAsyncServiceHandler ash = ar.AsyncServiceHandler;
				Php::Object q(PHP_BUFFER, new CPhpBuffer(&ar.UQueue));
				if (sync) {
					std::unique_lock<std::mutex> lk(this->m_mPhp);
					this->m_cvPhp.notify_all();
				}
				else if (phpRh.isCallable()) {
					phpRh(q, ar.RequestId);
				}
			};
			if (args > 4) {
				phpEx = params[4];
				if (!phpEx.isCallable()) {
					throw Php::Exception("A callback required for server exception");
				}
			}
			se = [phpEx, sync, &exception, this](CAsyncServiceHandler *ash, unsigned short reqId, const wchar_t *errMsg, const char *errWhere, unsigned int errCode) {
				if (phpEx.isCallable()) {
					phpEx(Utilities::ToUTF8(errMsg).c_str(), (int64_t)errCode, errWhere, reqId);
				}
				if (sync) {
					exception = true;
					std::unique_lock<std::mutex> lk(this->m_mPhp);
					this->m_cvPhp.notify_all();
				}
			};
			if (args > 3) {
				phpCanceled = params[3];
				if (!phpCanceled.isCallable()) {
					throw Php::Exception("A callback required for request aborting event");
				}
			}
			discarded = [phpCanceled, reqId, sync, this, &canceled, &closed](CAsyncServiceHandler *ash, bool aborted) {
				if (phpCanceled.isCallable()) {
					phpCanceled(aborted, reqId);
				}
				if (sync) {
					canceled = aborted;
					if (!canceled) {
						closed = true;
					}
					std::unique_lock<std::mutex> lk(this->m_mPhp);
					this->m_cvPhp.notify_all();
				}
			};
			Php::Value v;
			if (args > 1) {
				Php::Value &q = params[1];
				if (q.instanceOf(PHP_BUFFER)) {
					v = q.call("PopBytes");
					pBuffer = (const unsigned char*)v.rawValue();
					bytes = (unsigned int)v.length();
				}
				else if (!q.isNull()) {
					throw Php::Exception("An instance of CUQueue or null required for request sending data");
				}
			}
			if (sync) {
				if (!SendRequest(reqId, pBuffer, bytes, rh, discarded, se)) {
					return rrsClosed;
				}
				std::unique_lock<std::mutex> lk(m_mPhp);
				auto status = m_cvPhp.wait_for(lk, std::chrono::milliseconds(timeout));
				if (status == std::cv_status::timeout) {
					return rrsTimeout;
				}
				else if (closed) {
					return rrsClosed;
				}
				else if (exception) {
					return PA::rrsServerException;
				}
				return rrsOk;
			}
			return SendRequest(reqId, pBuffer, bytes, rh, discarded, se) ? rrsOk : rrsClosed;
		}
	}
}

namespace PA {
	const char *PHP_BUFFER = "CUQueue";
	const char *PHP_CONN_CONTEXT = "CConnectionContext";
	const char *PHP_FILE_HANDLER = "CAsyncFile";
	const char *PHP_DB_HANDLER = "CAsyncDb";
	const char *PHP_QUEUE_HANDLER = "CAsyncQueue";
	const char *PHP_ASYNC_HANDLER = "CAsyncHandler";
	const char *PHP_SOCKET_POOL = "CSocketPool";
	const char *PHP_CERT = "CUCert";
	const char *PHP_SOCKET = "CClientSocket";
	const char *PHP_CONSTRUCT = "__construct";
	const char *PHP_MANAGER = "CManager";

	//SendRequest
	const char *PHP_SENDREQUEST = "SendRequest";
	const char *PHP_SENDREQUEST_REQID = "reqId";
	const char *PHP_SENDREQUEST_BUFF = "buff";
	const char *PHP_SENDREQUEST_RH = "rh";
	const char *PHP_SENDREQUEST_CH = "ch";
	const char *PHP_SENDREQUEST_EX = "ex";

	const std::string SPA_NS("SPA\\");
	const std::string SPA_CS_NS("SPA\\ClientSide\\");

	std::string SP_CONFIG = "sp_config.json";

#ifdef WIN32_64
	const char SYS_DIR = '\\';
#else
	const char SYS_DIR = '/';
#endif

	void Trim(std::string &str) {
		while (str.size() && std::isspace(str.back())) {
			str.pop_back();
		}
		while (str.size() && std::isspace(str.front())) {
			str.erase(0, 1);
		}
	}

	void ToVariant(const Php::Value &data, CComVariant &vt, const std::string &id) {
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
				vt = (int)data.numericValue();
			}
			else if (id == "ui" || id == "uint" || id == "unsigned" || id == "unsigned int") {
				vt = (unsigned int)data.numericValue();
			}
			else if (id == "ul" || id == "ulong" || id == "unsigned long long" || id == "unsigned long int") {
				vt = (SPA::UINT64)data.numericValue();
			}
			else if (id == "s" || id == "short" || id == "w") {
				vt = (short)data.numericValue();
			}
			else if (id == "c" || id == "char" || id == "a") {
				vt = (char)data.numericValue();
			}
			else if (id == "b" || id == "byte" || id == "unsigned char") {
				vt = (unsigned char)data.numericValue();
			}
			else if (id == "us" || id == "ushort" || id == "unsigned short") {
				vt = (unsigned short)data.numericValue();
			}
			else if (id == "dec" || id == "decimal") {
				vt.vt = VT_DECIMAL;
				SPA::ToDecimal(data.numericValue(), vt.decVal);
			}
			else {
				vt = data.numericValue();
			}
			break;
		case Php::Type::Float:
			if (id == "f" || id == "float") {
				vt = (float)data.floatValue();
			}
			else if (id == "dec" || id == "decimal") {
				vt.vt = VT_DECIMAL;
				SPA::ToDecimal(data.floatValue(), vt.decVal);
			}
			else {
				vt = data.floatValue();
			}
			break;
		case Php::Type::String:
		{
			vt.Clear();
			const char *raw = data.rawValue();
			if (!raw) {
				vt.vt = VT_NULL;
			}
			else if (id == "a" || id == "ascii") {
				vt = raw;
			}
			else if (id == "bytes" || id == "binary" || id == "uuid" || id == "clsid" || id == "guid") {
				unsigned char *pBuffer;
				vt.vt = (VT_ARRAY | VT_UI1);
				SAFEARRAYBOUND sab[1] = { (unsigned int)data.length(), 0 };
				SAFEARRAY *psa = SafeArrayCreate(VT_UI1, 1, sab);
				SafeArrayAccessData(psa, (void**)&pBuffer);
				memcpy(pBuffer, raw, (unsigned int)data.length());
				SafeArrayUnaccessData(psa);
				vt.parray = psa;
			}
			else if (id == "dec" || id == "decimal") {
				vt.vt = VT_DECIMAL;
				SPA::ParseDec_long(raw, vt.decVal);
			}
			else {
				vt.vt = VT_BSTR;
				vt.bstrVal = SPA::Utilities::ToBSTR(raw, (unsigned int)data.length());
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
			}
			else {
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

	void ToVariant(const Php::Value &data, SPA::UDB::CDBVariant &vt, const std::string &id) {
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
				vt = (int)data.numericValue();
			}
			else if (id == "ui" || id == "uint" || id == "unsigned" || id == "unsigned int") {
				vt = (unsigned int)data.numericValue();
			}
			else if (id == "ul" || id == "ulong" || id == "unsigned long long" || id == "unsigned long int") {
				vt = (SPA::UINT64)data.numericValue();
			}
			else if (id == "s" || id == "short" || id == "w") {
				vt = (short)data.numericValue();
			}
			else if (id == "c" || id == "char" || id == "a") {
				vt = (char)data.numericValue();
			}
			else if (id == "b" || id == "byte" || id == "unsigned char") {
				vt = (unsigned char)data.numericValue();
			}
			else if (id == "us" || id == "ushort" || id == "unsigned short") {
				vt = (unsigned short)data.numericValue();
			}
			else if (id == "dec" || id == "decimal") {
				vt.vt = VT_DECIMAL;
				SPA::ToDecimal(data.numericValue(), vt.decVal);
			}
			else {
				vt = data.numericValue();
			}
			break;
		case Php::Type::Float:
			if (id == "f" || id == "float") {
				vt = (float)data.floatValue();
			}
			else if (id == "dec" || id == "decimal") {
				vt.vt = VT_DECIMAL;
				SPA::ToDecimal(data.floatValue(), vt.decVal);
			}
			else {
				vt = data.floatValue();
			}
			break;
		case Php::Type::String:
		{
			vt.Clear();
			const char *raw = data.rawValue();
			if (!raw) {
				vt.vt = VT_NULL;
			}
			else if (id == "a" || id == "ascii") {
				vt = raw;
			}
			else if (id == "bytes" || id == "binary") {
				unsigned char *pBuffer;
				vt.vt = (VT_ARRAY | VT_UI1);
				SAFEARRAYBOUND sab[1] = { (unsigned int)data.length(), 0 };
				SAFEARRAY *psa = SafeArrayCreate(VT_UI1, 1, sab);
				SafeArrayAccessData(psa, (void**)&pBuffer);
				memcpy(pBuffer, raw, (unsigned int)data.length());
				SafeArrayUnaccessData(psa);
				vt.parray = psa;
			}
			else if ((id == "uuid" || id == "clsid" || id == "guid") && data.length() == sizeof(GUID)) {
				vt = *((GUID*)raw);
			}
			else if (id == "dec" || id == "decimal") {
				vt.vt = VT_DECIMAL;
				SPA::ParseDec_long(raw, vt.decVal);
			}
			else {
				vt.vt = VT_BSTR;
				vt.bstrVal = SPA::Utilities::ToBSTR(raw, (unsigned int)data.length());
			}
		}
		break;
		case Php::Type::Object:
			if (Php::is_a(data, "DateTime")) {
				Php::Value dt = data.call("format", "Y-m-d H:i:s.u");
				SPA::UDateTime udt(dt.rawValue());
				vt = udt;
				break;
			}
			else {
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

	Php::Value GetManager() {
		return CPhpManager::Parse();
	}

} //namespace PA