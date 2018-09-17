
#include "stdafx.h"
#include "njobjects.h"
#include "../../../include/sqlite/usqlite.h"
#include "../../../include/mysql/umysql.h"
#include "../../../include/odbc/uodbc.h"
#include "njsqlite.h"


namespace SPA {
	namespace ClientSide {

		using namespace NJA;

		SPA::UINT64 CAsyncServiceHandler::SendRequest(Isolate* isolate, int args, Local<Value> *argv, unsigned short reqId, const unsigned char *pBuffer, unsigned int size) {
			if (!argv) args = 0;
			ResultHandler rh;
			DServerException se;
			DDiscarded dd;
			UINT64 callIndex = GetCallIndex();
			if (args > 0) {
				if (argv[0]->IsFunction()) {
					std::shared_ptr<CNJFunc> func(new CNJFunc);
					func->Reset(isolate, Local<Function>::Cast(argv[0]));
					rh = [this, func](CAsyncResult &ar) {
						ReqCb cb;
						cb.ReqId = ar.RequestId;
						cb.Type = eResult;
						cb.Func = func;
						PAsyncServiceHandler h = ar.AsyncServiceHandler;
						cb.Buffer = CScopeUQueue::Lock(ar.UQueue.GetOS(), ar.UQueue.GetEndian());
						*cb.Buffer << h;
						cb.Buffer->Push(ar.UQueue.GetBuffer(), ar.UQueue.GetSize());
						ar.UQueue.SetSize(0);
						CAutoLock al(this->m_cs);
						this->m_deqReqCb.push_back(cb);
						int fail = uv_async_send(&this->m_typeReq);
						assert(!fail);
					};
				}
				else if (!argv[0]->IsNullOrUndefined()) {
					ThrowException(isolate, "A callback expected for tracking returned results");
					return 0;
				}
			}
			if (args > 1) {
				if (argv[1]->IsFunction()) {
					std::shared_ptr<CNJFunc> func(new CNJFunc);
					func->Reset(isolate, Local<Function>::Cast(argv[1]));
					dd = [this, func, reqId](CAsyncServiceHandler *ash, bool canceled) {
						ReqCb cb;
						cb.ReqId = reqId;
						cb.Type = eDiscarded;
						cb.Func = func;
						PAsyncServiceHandler h = ash;
						cb.Buffer = CScopeUQueue::Lock();
						*cb.Buffer << h << canceled;
						CAutoLock al(this->m_cs);
						this->m_deqReqCb.push_back(cb);
						int fail = uv_async_send(&this->m_typeReq);
						assert(!fail);
					};
				}
				else if (!argv[1]->IsNullOrUndefined()) {
					ThrowException(isolate, "A callback expected for tracking socket closed or canceled events");
					return 0;
				}
			}
			if (args > 2) {
				if (argv[2]->IsFunction()) {
					std::shared_ptr<CNJFunc> func(new CNJFunc);
					func->Reset(isolate, Local<Function>::Cast(argv[2]));
					se = [this, func](CAsyncServiceHandler *ash, unsigned short reqId, const wchar_t *errMsg, const char *errWhere, unsigned int errCode) {
						ReqCb cb;
						cb.ReqId = reqId;
						cb.Type = eException;
						cb.Func = func;
						PAsyncServiceHandler h = ash;
						cb.Buffer = CScopeUQueue::Lock();
						*cb.Buffer << h << errMsg << errWhere << errCode;
						CAutoLock al(this->m_cs);
						this->m_deqReqCb.push_back(cb);
						int fail = uv_async_send(&this->m_typeReq);
						assert(!fail);
					};
				}
				else if (!argv[2]->IsNullOrUndefined()) {
					ThrowException(isolate, "A callback expected for tracking exceptions from server");
					return 0;
				}
			}
			if (!SendRequest(reqId, pBuffer, size, rh, dd, se)) {
				return INVALID_NUMBER;
			}
			return callIndex;
		}

		void CAsyncServiceHandler::req_cb(uv_async_t* handle) {
			CAsyncServiceHandler* obj = (CAsyncServiceHandler*)handle->data; //sender
			assert(obj);
			if (!obj)
				return;
			Isolate* isolate = Isolate::GetCurrent();
			HandleScope handleScope(isolate); //required for Node 4.x
			{
				SPA::CAutoLock al(obj->m_cs);
				while (obj->m_deqReqCb.size()) {
					ReqCb &cb = obj->m_deqReqCb.front();
					PAsyncServiceHandler processor;
					*cb.Buffer >> processor;
					assert(!processor);
					Local<v8::Object> njAsh;
					unsigned int sid = processor->GetSvsID();
					switch (sid) {
					case SPA::Odbc::sidOdbc:
					case SPA::Mysql::sidMysql:
					case SPA::Sqlite::sidSqlite:
						njAsh = NJSqlite::New(isolate, (CNjDb*)processor, true);
						break;
					case SPA::Queue::sidQueue:
						njAsh = NJAsyncQueue::New(isolate, (CAQueue*)processor, true);
						break;
					case SPA::SFile::sidFile:
						njAsh = NJFile::New(isolate, (CSFile*)processor, true);
						break;
					default:
						njAsh = NJHandler::New(isolate, processor, true);
						break;
					}
					Local<Value> jsReqId = Uint32::New(isolate, cb.ReqId);
					Local<Function> func;
					assert(cb.Func);
					if (cb.Func)
						func = Local<Function>::New(isolate, *cb.Func);
					switch (cb.Type) {
					case eResult:
					{
						Local<Object> q = NJQueue::New(isolate, cb.Buffer);
						if (!func.IsEmpty()) {
							Local<Value> argv[] = { q, func, njAsh, jsReqId };
							func->Call(isolate->GetCurrentContext(), Null(isolate), 4, argv);
						}
					}
					break;
					case eDiscarded:
					{
						bool canceled;
						*cb.Buffer >> canceled;
						assert(!cb.Buffer->GetSize());
						CScopeUQueue::Unlock(cb.Buffer);
						auto b = Boolean::New(isolate, canceled);
						if (!func.IsEmpty()) {
							Local<Value> argv[] = { b, njAsh, jsReqId };
							func->Call(isolate->GetCurrentContext(), Null(isolate), 3, argv);
						}
					}
					break;
					case eException:
					{
						std::wstring errMsg;
						std::string errWhere;
						unsigned int errCode;
						*cb.Buffer >> errMsg >> errWhere >> errCode;
						assert(!cb.Buffer->GetSize());
						CScopeUQueue::Unlock(cb.Buffer);
						Local<String> jsMsg = ToStr(isolate, errMsg.c_str());
						Local<String> jsWhere = ToStr(isolate, errWhere.c_str());
						Local<Value> jsCode = Number::New(isolate, errCode);
						if (!func.IsEmpty()) {
							Local<Value> argv[] = { jsMsg, jsCode, jsWhere, func, njAsh, jsReqId };
							func->Call(isolate->GetCurrentContext(), Null(isolate), 6, argv);
						}
					}
					break;
					default:
						assert(false); //shouldn't come here
						break;
					}
					obj->m_deqReqCb.pop_front();
				}
			}
			isolate->RunMicrotasks();
		}

		Local<Object> CreateDb(Isolate* isolate, CAsyncServiceHandler *ash) {
			Local<Object> njDB;
			unsigned int sid = ash->GetSvsID();
			switch (sid) {
			case SPA::Odbc::sidOdbc:
			case SPA::Mysql::sidMysql:
			case SPA::Sqlite::sidSqlite:
				njDB = NJSqlite::New(isolate, (CNjDb*)ash, true);
				break;
			default:
				assert(false);
				break;
			}
			return njDB;
		}
	}
}

namespace NJA {

	int time_offset()
	{
		time_t gmt, rawtime = time(nullptr);
		struct tm *ptm;

#ifndef WIN32_64
		struct tm gbuf;
		ptm = gmtime_r(&rawtime, &gbuf);
#else
		ptm = gmtime(&rawtime);
#endif
		// Request that mktime() looksup dst in timezone database
		ptm->tm_isdst = -1;
		gmt = mktime(ptm);

		return (int)difftime(rawtime, gmt);
	}

	int g_TimeOffset = time_offset();

	void ThrowException(Isolate* isolate, const char *str) {
		isolate->ThrowException(Exception::TypeError(ToStr(isolate, str)));
	}

	std::vector<unsigned int>ToGroups(const Local<Value>& p) {
		unsigned int *groups = (unsigned int *)node::Buffer::Data(p);
		Local<v8::Uint32Array> vInt = Local<v8::Uint32Array>::Cast(p);
		unsigned int count = (unsigned int)vInt->Length();
		std::vector<unsigned int> v(groups, groups + count);
		return v;
	}

	Local<String> ToStr(Isolate* isolate, const char *str, size_t len) {
		if (!str) {
			str = "";
			len = 0;
		}
		return String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal, (int)len).ToLocalChecked();
	}

	Local<String> ToStr(Isolate* isolate, const wchar_t *str, size_t len) {
		if (!str) {
			str = L"";
			len = 0;
		}
#ifdef WIN32_64
		return String::NewFromTwoByte(isolate, (const uint16_t *)str, v8::NewStringType::kInternalized, (int)len).ToLocalChecked(); //v8::NewStringType::kNormal will crash if length is large
#else
		SPA::CScopeUQueue sb;
		SPA::Utilities::ToUTF16(str, len, *sb);
		return String::NewFromTwoByte(isolate, (const uint16_t *)sb->GetBuffer(), v8::NewStringType::kInternalized, (int)sb->GetSize() / sizeof(uint16_t)).ToLocalChecked();
#endif
	}

	Local<String> ToStr(Isolate* isolate, const uint16_t *str, size_t len) {
		if (!str) {
			str = (const uint16_t *)L"";
			len = 0;
		}
		return String::NewFromTwoByte(isolate, str, v8::NewStringType::kNormal, (int)len).ToLocalChecked();
	}

	std::wstring ToStr(const Local<Value>& s) {
		assert(s->IsString());
		String::Value str(s);
#ifdef WIN32_64
		return (const wchar_t*)*str;
#else
		return SPA::Utilities::ToWide(*str, (unsigned int)str.length());
#endif
	}

	std::string ToAStr(const Local<Value> &s) {
		assert(s->IsString());
		String::Utf8Value str(s);
		return *str;
	}

	Local<Value> ToDate(Isolate* isolate, SPA::UINT64 datetime) {
		SPA::UDateTime dt(datetime);
		unsigned int us;
		std::tm tm = dt.GetCTime(&us);
		if (!tm.tm_mday) {
			//time only, convert it to js string
			return String::NewFromUtf8(isolate, dt.ToDBString().c_str());
		}
		double time = (double)std::mktime(&tm);
		g_cs.lock();
		time += g_TimeOffset;
		g_cs.unlock();
		time *= 1000;
		time += (us / 1000.0);
		if (tm.tm_isdst > 0)
			time -= 3600000;
		return Date::New(isolate, time);
	}

	SPA::UINT64 ToDate(const Local<Value>& d) {
		SPA::UINT64 millisSinceEpoch;
		if (d->IsDate()) {
			Date *dt = Date::Cast(*d);
			millisSinceEpoch = (SPA::UINT64) dt->ValueOf();
		}
		else if (d->IsNumber()) {
			millisSinceEpoch = (SPA::UINT64)(d->IntegerValue());
		}
		else {
			return INVALID_NUMBER;
		}
		std::time_t t = millisSinceEpoch / 1000;
		unsigned int ms = (unsigned int)(millisSinceEpoch % 1000);
		std::tm *ltime = std::localtime(&t);
		SPA::UDateTime dt(*ltime, ms * 1000);
		return dt.time;
	}

	bool From(const Local<Value>& v, const std::string &id, CComVariant &vt) {
		vt.Clear();
		if (v->IsNullOrUndefined())
			vt.vt = VT_NULL;
		else if (v->IsDate()) {
			vt.vt = VT_DATE;
			vt.ullVal = ToDate(v);
		}
		else if (v->IsBoolean()) {
			vt.vt = VT_BOOL;
			vt.boolVal = v->BooleanValue() ? VARIANT_TRUE : VARIANT_FALSE;
		}
		else if (v->IsString()) {
			if (id == "a" || id == "ascii") {
				char *p;
				vt.vt = (VT_ARRAY | VT_I1);
				String::Utf8Value str(v);
				unsigned int len = (unsigned int)str.length();
				SAFEARRAYBOUND sab[] = { len, 0 };
				vt.parray = SafeArrayCreate(VT_I1, 1, sab);
				SafeArrayAccessData(vt.parray, (void**)&p);
				memcpy(p, *str, len);
				SafeArrayUnaccessData(vt.parray);
			}
			else if (id == "dec" || id == "decimal") {
				String::Utf8Value str(v);
				vt.vt = VT_DECIMAL;
				SPA::ParseDec(*str, vt.decVal);
			}
			else {
				vt.vt = VT_BSTR;
				String::Value str(v);
#ifdef WIN32_64
				vt.bstrVal = SysAllocString((const wchar_t*)*str);
#else
				vt.bstrVal = SPA::Utilities::SysAllocString(*str, (unsigned int)str.length());
#endif
			}
		}
		else if (v->IsNumber()) {
			if (id == "f" || id == "float") {
				vt.vt = VT_R4;
				vt.fltVal = (float)v->NumberValue();
			}
			else if (id == "d" || id == "double") {
				vt.vt = VT_R8;
				vt.dblVal = v->NumberValue();
			}
			else if (id == "i" || id == "int") {
				vt.vt = VT_I4;
				vt.lVal = v->Int32Value();
			}
			else if (id == "ui" || id == "uint") {
				vt.vt = VT_UI4;
				vt.ulVal = v->Uint32Value();
			}
			else if (id == "l" || id == "long") {
				vt.vt = VT_I8;
				vt.llVal = (SPA::INT64)v->NumberValue();
			}
			else if (id == "ul" || id == "ulong") {
				vt.vt = VT_UI8;
				vt.ullVal = (SPA::UINT64)v->NumberValue();
			}
			else if (id == "s" || id == "short") {
				vt.vt = VT_I2;
				vt.iVal = (short)v->Int32Value();
			}
			else if (id == "us" || id == "ushort") {
				vt.vt = VT_UI2;
				vt.iVal = (unsigned short)v->Uint32Value();
			}
			else if (id == "dec" || id == "decimal") {
				vt.vt = VT_DECIMAL;
				String::Utf8Value str(v);
				ParseDec(*str, vt.decVal);
			}
			else if (id == "c" || id == "char") {
				vt.vt = VT_I1;
				vt.cVal = (char)v->Int32Value();
			}
			else if (id == "b" || id == "byte") {
				vt.vt = VT_UI1;
				vt.bVal = (unsigned char)v->Uint32Value();
			}
			else if (id == "date") {
				vt.vt = VT_DATE;
				vt.ullVal = (SPA::UINT64)v->NumberValue();
			}
			else {
				assert(false);
				return false;
			}
		}
		else if (v->IsInt8Array()) {
			char *p;
			vt.vt = (VT_ARRAY | VT_I1);
			char *bytes = node::Buffer::Data(v);
			unsigned int len = (unsigned int)node::Buffer::Length(v);
			SAFEARRAYBOUND sab[] = { len, 0 };
			vt.parray = SafeArrayCreate(VT_I1, 1, sab);
			SafeArrayAccessData(vt.parray, (void**)&p);
			memcpy(p, bytes, len);
			SafeArrayUnaccessData(vt.parray);
		}
		else if (v->IsInt16Array()) {
			short *p;
			vt.vt = (VT_ARRAY | VT_I2);
			char *bytes = node::Buffer::Data(v);
			Local<v8::Int16Array> vInt = Local<v8::Int16Array>::Cast(v);
			unsigned int len = (unsigned int)vInt->Length();
			SAFEARRAYBOUND sab[] = { len, 0 };
			vt.parray = SafeArrayCreate(VT_I2, 1, sab);
			SafeArrayAccessData(vt.parray, (void**)&p);
			memcpy(p, bytes, len * sizeof(short));
			SafeArrayUnaccessData(vt.parray);
		}
		else if (v->IsUint16Array()) {
			unsigned short *p;
			vt.vt = (VT_ARRAY | VT_UI2);
			char *bytes = node::Buffer::Data(v);
			Local<v8::Uint16Array> vInt = Local<v8::Uint16Array>::Cast(v);
			unsigned int len = (unsigned int)vInt->Length();
			SAFEARRAYBOUND sab[] = { len, 0 };
			vt.parray = SafeArrayCreate(VT_UI2, 1, sab);
			SafeArrayAccessData(vt.parray, (void**)&p);
			memcpy(p, bytes, len * sizeof(unsigned short));
			SafeArrayUnaccessData(vt.parray);
		}
		else if (v->IsInt32Array()) {
			int *p;
			vt.vt = (VT_ARRAY | VT_I4);
			char *bytes = node::Buffer::Data(v);
			Local<v8::Int32Array> vInt = Local<v8::Int32Array>::Cast(v);
			unsigned int len = (unsigned int)vInt->Length();
			SAFEARRAYBOUND sab[] = { len, 0 };
			vt.parray = SafeArrayCreate(VT_I4, 1, sab);
			SafeArrayAccessData(vt.parray, (void**)&p);
			memcpy(p, bytes, len * sizeof(int));
			SafeArrayUnaccessData(vt.parray);
		}
		else if (v->IsUint32Array()) {
			unsigned int *p;
			vt = (VT_ARRAY | VT_UI4);
			char *bytes = node::Buffer::Data(v);
			Local<v8::Uint32Array> vInt = Local<v8::Uint32Array>::Cast(v);
			unsigned int len = (unsigned int)vInt->Length();
			SAFEARRAYBOUND sab[] = { len, 0 };
			vt.parray = SafeArrayCreate(VT_UI4, 1, sab);
			SafeArrayAccessData(vt.parray, (void**)&p);
			memcpy(p, bytes, len * sizeof(unsigned int));
			SafeArrayUnaccessData(vt.parray);
		}
		else if (v->IsFloat32Array()) {
			float *p;
			vt.vt = (VT_ARRAY | VT_R4);
			char *bytes = node::Buffer::Data(v);
			Local<v8::Float32Array> vInt = Local<v8::Float32Array>::Cast(v);
			unsigned int len = (unsigned int)vInt->Length();
			SAFEARRAYBOUND sab[] = { len, 0 };
			vt.parray = SafeArrayCreate(VT_R4, 1, sab);
			SafeArrayAccessData(vt.parray, (void**)&p);
			memcpy(p, bytes, len * sizeof(float));
			SafeArrayUnaccessData(vt.parray);
		}
		else if (v->IsFloat64Array()) {
			double *p;
			vt.vt = (VT_ARRAY | VT_R8);
			char *bytes = node::Buffer::Data(v);
			Local<v8::Float64Array> vInt = Local<v8::Float64Array>::Cast(v);
			unsigned int len = (unsigned int)vInt->Length();
			SAFEARRAYBOUND sab[] = { len, 0 };
			vt.parray = SafeArrayCreate(VT_R8, 1, sab);
			SafeArrayAccessData(vt.parray, (void**)&p);
			memcpy(p, bytes, len * sizeof(double));
			SafeArrayUnaccessData(vt.parray);
		}
		else if (node::Buffer::HasInstance(v)) {
			char *p;
			char *bytes = node::Buffer::Data(v);
			unsigned int len = (unsigned int)node::Buffer::Length(v);
			vt.vt = (VT_ARRAY | VT_UI1);
			SAFEARRAYBOUND sab[] = { len, 0 };
			vt.parray = SafeArrayCreate(VT_UI1, 1, sab);
			SafeArrayAccessData(vt.parray, (void**)&p);
			memcpy(p, bytes, len);
			SafeArrayUnaccessData(vt.parray);
			if (len == sizeof(GUID) && (id == "u" || id == "uuid")) {
				vt.vt = VT_CLSID;
			}
		}
		else if (v->IsArray()) {
			tagDataType dt = dtUnknown;
			Local<Array> jsArr = Local<Array>::Cast(v);
			unsigned int count = jsArr->Length();
			for (unsigned int n = 0; n < count; ++n) {
				auto d = jsArr->Get(n);
				if (d->IsBoolean()) {
					if (dt && dt != dtBool) {
						return false;
					}
					else
						dt = dtBool;
				}
				else if (d->IsDate()) {
					if (dt && dt != dtDate) {
						return false;
					}
					else
						dt = dtDate;
				}
				else if (d->IsString()) {
					if (dt && dt != dtString) {
						return false;
					}
					else
						dt = dtString;
				}
				else {
					return false;
				}
			}
			VARTYPE vtType;
			switch (dt) {
			case dtString:
				vtType = VT_BSTR;
				break;
			case dtBool:
				vtType = VT_BOOL;
				break;
			case dtDate:
				vtType = VT_DATE;
				break;
			default:
				assert(false); //shouldn't come here
				break;
			}
			void *p;
			vt.vt = (VT_ARRAY | vtType);
			SAFEARRAYBOUND sab[] = { count, 0 };
			vt.parray = SafeArrayCreate(vtType, 1, sab);
			SafeArrayAccessData(vt.parray, &p);
			for (unsigned int n = 0; n < count; ++n) {
				auto d = jsArr->Get(n);
				if (d->IsBoolean()) {
					VARIANT_BOOL *pb = (VARIANT_BOOL *)p;
					pb[n] = d->BooleanValue() ? VARIANT_TRUE : VARIANT_FALSE;
				}
				else if (d->IsDate()) {
					SPA::UINT64 *pd = (SPA::UINT64*)p;
					pd[n] = ToDate(d);
				}
				else if (d->IsString()) {
					BSTR *pbstr = (BSTR*)p;
					String::Value str(d);
#ifdef WIN32_64
					pbstr[n] = ::SysAllocString((const wchar_t *)*str);
#else
					pbstr[n] = SPA::Utilities::SysAllocString(*str, (unsigned int)str.length());
#endif
				}
				else {
					assert(false);
				}
			}
			SafeArrayUnaccessData(vt.parray);
		}
		else {
			return false; //not supported
		}
		return true;
	}

	Local<Value> From(Isolate* isolate, const VARIANT &vt, bool strForDec) {
		VARTYPE type = vt.vt;
		switch (type) {
		case VT_NULL:
		case VT_EMPTY:
			return Null(isolate);
		case VT_BOOL:
			return Boolean::New(isolate, vt.boolVal ? true : false);
		case VT_I1:
		case VT_I2:
		case VT_INT:
		case VT_I4:
			return Int32::New(isolate, vt.lVal);
		case VT_I8:
			return Number::New(isolate, (double)vt.llVal);
		case VT_UI1:
		case VT_UI2:
		case VT_UINT:
		case VT_UI4:
			return Uint32::New(isolate, vt.ulVal);
		case VT_UI8:
			return Number::New(isolate, (double)vt.ullVal);
		case VT_R4:
			return Number::New(isolate, vt.fltVal);
		case VT_R8:
			return Number::New(isolate, vt.dblVal);
		case VT_CY:
		{
			double d = (double)vt.llVal;
			d /= 10000;
			return Number::New(isolate, d);
		}
		case VT_DECIMAL:
			if (strForDec)
				return ToStr(isolate, SPA::ToString(vt.decVal).c_str());
			return Number::New(isolate, ToDouble(vt.decVal));
		case VT_DATE:
			return ToDate(isolate, vt.ullVal);
		case (VT_I1 | VT_ARRAY):
		{
			const char *str = nullptr;
			unsigned int len = vt.parray->rgsabound->cElements;
			::SafeArrayAccessData(vt.parray, (void**)&str);
			auto s = ToStr(isolate, str, len);
			::SafeArrayUnaccessData(vt.parray);
			return s;
		}
		case VT_CLSID:
		case (VT_UI1 | VT_ARRAY):
		{
			char *str = nullptr;
			unsigned int len = vt.parray->rgsabound->cElements;
			::SafeArrayAccessData(vt.parray, (void**)&str);
			auto bytes = node::Buffer::New(isolate, str, len).ToLocalChecked();
			::SafeArrayUnaccessData(vt.parray);
			return bytes;
		}
		case VT_BSTR:
			return ToStr(isolate, vt.bstrVal, SysStringLen(vt.bstrVal));
		default:
		{
			bool is_array = ((type & VT_ARRAY) == VT_ARRAY);
			if (is_array) {
				void *pvt;
				bool ok = true;
				unsigned int count = vt.parray->rgsabound->cElements;
				::SafeArrayAccessData(vt.parray, &pvt);
				type = (type & (~VT_ARRAY));
				switch (type) {
				case VT_BOOL:
				case VT_BSTR:
				case VT_DATE:
				case VT_I8:
				case VT_UI8:
				case VT_CY:
				case VT_DECIMAL:
				case VT_VARIANT:
				{
					Local<Array> v = Array::New(isolate);
					for (unsigned int n = 0; n < count; ++n) {
						switch (type) {
						case VT_BOOL:
						{
							VARIANT_BOOL *p = (VARIANT_BOOL *)pvt;
							v->Set(n, Boolean::New(isolate, (p[n] == VARIANT_FALSE) ? false : true));
						}
						break;
						case VT_UI8:
						{
							SPA::UINT64 *p = (SPA::UINT64 *)pvt;
							v->Set(n, Number::New(isolate, (double)(p[n])));
						}
						break;
						case VT_I8:
						{
							SPA::INT64 *p = (SPA::INT64 *)pvt;
							v->Set(n, Number::New(isolate, (double)(p[n])));
						}
						break;
						case VT_CY:
						{
							SPA::INT64 *p = (SPA::INT64 *)pvt;
							v->Set(n, Number::New(isolate, ((double)p[n]) / 10000));
						}
						break;
						case VT_DECIMAL:
						{
							DECIMAL *p = (DECIMAL *)pvt;
							if (strForDec)
								v->Set(n, ToStr(isolate, SPA::ToString(p[n]).c_str()));
							else
								v->Set(n, Number::New(isolate, SPA::ToDouble(p[n])));
						}
						break;
						case VT_BSTR:
						{
							BSTR *p = (BSTR *)pvt;
							if (p[n]) {
								auto s = ToStr(isolate, p[n]);
								v->Set(n, s);
							}
							else
								v->Set(n, Null(isolate));
						}
						break;
						case VT_DATE:
						{
							SPA::UINT64 *p = (SPA::UINT64 *)pvt;
							v->Set(n, ToDate(isolate, p[n]));
						}
						break;
						case VT_VARIANT:
						{
							Local<Array> v = Array::New(isolate);
							for (unsigned int n = 0; n < count; ++n) {
								VARIANT *p = (VARIANT *)pvt;
								v->Set(n, From(isolate, p[n], strForDec));
							}
							::SafeArrayUnaccessData(vt.parray);
							return v;
						}
						default:
							assert(false); //shouldn't come here
							break;
						}
					}
					::SafeArrayUnaccessData(vt.parray);
					return v;
				}
				case VT_I4:
				case VT_INT:
				{
					Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof(int));
					Local<v8::Int32Array> v = v8::Int32Array::New(buf, 0, count);
					Local<Value> p = v;
					char *bytes = node::Buffer::Data(p);
					memcpy(bytes, pvt, count * sizeof(int));
					::SafeArrayUnaccessData(vt.parray);
					return v;
				}
				break;
				case VT_UI4:
				case VT_UINT:
				{
					Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof(unsigned int));
					Local<v8::Uint32Array> v = v8::Uint32Array::New(buf, 0, count);
					Local<Value> p = v;
					char *bytes = node::Buffer::Data(p);
					memcpy(bytes, pvt, count * sizeof(unsigned int));
					::SafeArrayUnaccessData(vt.parray);
					return v;
				}
				break;
				case VT_I2:
				{
					Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof(short));
					Local<v8::Int16Array> v = v8::Int16Array::New(buf, 0, count);
					Local<Value> p = v;
					char *bytes = node::Buffer::Data(p);
					memcpy(bytes, pvt, count * sizeof(short));
					::SafeArrayUnaccessData(vt.parray);
					return v;
				}
				break;
				case VT_UI2:
				{
					Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof(unsigned short));
					Local<v8::Uint16Array> v = v8::Uint16Array::New(buf, 0, count);
					Local<Value> p = v;
					char *bytes = node::Buffer::Data(p);
					memcpy(bytes, pvt, count * sizeof(unsigned short));
					::SafeArrayUnaccessData(vt.parray);
					return v;
				}
				break;
				case VT_R4:
				{
					Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof(float));
					Local<v8::Float32Array> v = v8::Float32Array::New(buf, 0, count);
					Local<Value> p = v;
					char *bytes = node::Buffer::Data(p);
					memcpy(bytes, pvt, count * sizeof(float));
					::SafeArrayUnaccessData(vt.parray);
					return v;
				}
				break;
				case VT_R8:
				{
					Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof(double));
					Local<v8::Float64Array> v = v8::Float64Array::New(buf, 0, count);
					Local<Value> p = v;
					char *bytes = node::Buffer::Data(p);
					memcpy(bytes, pvt, count * sizeof(double));
					::SafeArrayUnaccessData(vt.parray);
					return v;
				}
				break;
				default:
					break;
				}
				::SafeArrayUnaccessData(vt.parray);
			}
		}
		break;
		}
		return v8::Undefined(isolate);
	}

	bool ToPInfoArray(Isolate* isolate, const Local<Value> &p0, CParameterInfoArray &vInfo) {
		vInfo.clear();
		if (p0->IsArray()) {
			Local<Array> jsArr = Local<Array>::Cast(p0);
			unsigned int count = jsArr->Length();
			for (unsigned int n = 0; n < count; ++n) {
				auto jsP = jsArr->Get(n);
				if (!jsP->IsObject()) {
					ThrowException(isolate, "Invalid parameter meta found");
					return false;
				}
				auto pi = jsP->ToObject();
				CParameterInfo pInfo;
				auto v = pi->Get(ToStr(isolate, "Direction"));
				if (v->IsInt32()) {
					int d = v->Int32Value();
					if (d < pdInput || d > pdReturnValue) {
						ThrowException(isolate, "Bad parameter direction value found");
						return false;
					}
					pInfo.Direction = (tagParameterDirection)d;
				}
				else if (!v->IsNullOrUndefined()) {
					ThrowException(isolate, "An integer value expected for parameter direction");
					return false;
				}
				v = pi->Get(ToStr(isolate, "DataType"));
				if (v->IsInt32()) {
					int d = v->Int32Value();
					pInfo.DataType = (VARTYPE)d;
				}
				else if (!v->IsNullOrUndefined()) {
					ThrowException(isolate, "An integer value expected for parameter data type");
					return false;
				}
				v = pi->Get(ToStr(isolate, "ColumnSize"));
				if (v->IsInt32()) {
					int d = v->Int32Value();
					if (d < -1) {
						ThrowException(isolate, "Bad parameter column size value found");
						return false;
					}
					pInfo.ColumnSize = (unsigned int)d;
				}
				else if (!v->IsNullOrUndefined()) {
					ThrowException(isolate, "An integer value expected for parameter column size");
					return false;
				}
				v = pi->Get(ToStr(isolate, "Precision"));
				if (v->IsInt32()) {
					int d = v->Int32Value();
					if (d < 0 || d > 64) {
						ThrowException(isolate, "Bad parameter precision value found");
						return false;
					}
					pInfo.Precision = (unsigned char)d;
				}
				else if (!v->IsNullOrUndefined()) {
					ThrowException(isolate, "An integer value expected for parameter data precision");
					return false;
				}
				v = pi->Get(ToStr(isolate, "Scale"));
				if (v->IsInt32()) {
					int d = v->Int32Value();
					if (d < 0 || d > 64) {
						ThrowException(isolate, "Bad parameter scale value found");
						return false;
					}
					pInfo.Precision = (unsigned char)d;
				}
				else if (!v->IsNullOrUndefined()) {
					ThrowException(isolate, "An integer value expected for parameter data scale");
					return false;
				}
				v = pi->Get(ToStr(isolate, "ParameterName"));
				if (v->IsString()) {
					pInfo.ParameterName = ToStr(v);
				}
				else if (!v->IsNullOrUndefined()) {
					ThrowException(isolate, "An integer value expected for parameter data scale");
					return false;
				}
			}
		}
		else if (!p0->IsNullOrUndefined()) {
			ThrowException(isolate, "An array of parameter meta data expected");
			return false;
		}
		return true;
	}

	SPA::CUCriticalSection g_cs;
	SPA::CUQueue g_KeyAllowed;
}
