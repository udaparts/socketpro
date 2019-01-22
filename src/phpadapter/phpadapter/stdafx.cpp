
#include "stdafx.h"
#include <cctype>
#include "phpbuffer.h"

namespace SPA {
	namespace ClientSide {

		using namespace PA;

		Php::Value CAsyncServiceHandler::SendRequest(Php::Parameters &params) {
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
			size_t args = params.size();
			if (args > 4) {
				Php::Value func = params[4];
				if (!func.isCallable()) {
					throw Php::Exception("A callback required for server exception");
				}
				se = [func](CAsyncServiceHandler *ash, unsigned short reqId, const wchar_t *errMsg, const char *errWhere, unsigned int errCode) {
					func(Utilities::ToUTF8(errMsg).c_str(), (int64_t)errCode, errWhere, reqId);
				};
			}
			if (args > 3) {
				Php::Value func = params[3];
				if (!func.isCallable()) {
					throw Php::Exception("A callback required for request aborting event");
				}
				discarded = [func, reqId](CAsyncServiceHandler *ash, bool canceled) {
					func(canceled, reqId);
				};
			}
			if (args > 2) {
				Php::Value func = params[2];
				if (!func.isCallable()) {
					throw Php::Exception("A callback required for returning result");
				}
				rh = [func](CAsyncResult & ar) {
					PAsyncServiceHandler ash = ar.AsyncServiceHandler;
					Php::Object q(PHP_BUFFER, new CPhpBuffer(&ar.UQueue));
					func(q, ar.RequestId);
				};
			}
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
			return SendRequest(reqId, pBuffer, bytes, rh, discarded, se);
		}
	}
}

namespace PA {
	const char *PHP_BUFFER = "CUQueue";
	const char *PHP_FILE_HANDLER = "CAsyncFile";
	const char *PHP_DB_HANDLER = "CAsyncDb";
	const char *PHP_QUEUE_HANDLER = "CAsyncQueue";
	const char *PHP_ASYNC_HANDLER = "CAsyncHandler";
	const char *PHP_SOCKET_POOL = "CSocketPool";
	const char *PHP_CONSTRUCT = "__construct";

	//SendRequest
	const char *PHP_SENDREQUEST = "SendRequest";
	const char *PHP_SENDREQUEST_REQID = "reqId";
	const char *PHP_SENDREQUEST_BUFF = "buff";
	const char *PHP_SENDREQUEST_RH = "rh";
	const char *PHP_SENDREQUEST_CH = "ch";
	const char *PHP_SENDREQUEST_EX = "ex";

	void Trim(std::string &str) {
		while (str.size() && std::isspace(str.back())) {
			str.pop_back();
		}
		while (str.size() && std::isspace(str.front())) {
			str.erase(0, 1);
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

} //namespace PA