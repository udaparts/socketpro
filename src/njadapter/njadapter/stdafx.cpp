
#include "stdafx.h"
#include "njobjects.h"

namespace SPA {
	namespace ClientSide {

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
					isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "A callback expected for tracking returned results")));
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
						bool bigEndian;
						tagOperationSystem os = ash->GetAttachedClientSocket()->GetPeerOs(&bigEndian);
						cb.Buffer = CScopeUQueue::Lock(os, bigEndian);
						*cb.Buffer << h << canceled;
						CAutoLock al(this->m_cs);
						this->m_deqReqCb.push_back(cb);
						int fail = uv_async_send(&this->m_typeReq);
						assert(!fail);
					};
				}
				else if (!argv[1]->IsNullOrUndefined()) {
					isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "A callback expected for tracking socket closed or canceled events")));
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
						bool bigEndian;
						tagOperationSystem os = ash->GetAttachedClientSocket()->GetPeerOs(&bigEndian);
						cb.Buffer = CScopeUQueue::Lock(os, bigEndian);
						*cb.Buffer << h << errMsg << errWhere << errCode;
						CAutoLock al(this->m_cs);
						this->m_deqReqCb.push_back(cb);
						int fail = uv_async_send(&this->m_typeReq);
						assert(!fail);
					};
				}
				else if (!argv[2]->IsNullOrUndefined()) {
					isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "A callback expected for tracking exceptions from server")));
					return 0;
				}
			}
			return (SendRequest(reqId, pBuffer, size, rh, dd, se) ? callIndex : INVALID_NUMBER);
		}

		void CAsyncServiceHandler::req_cb(uv_async_t* handle) {
			CAsyncServiceHandler* obj = (CAsyncServiceHandler*)handle->data; //sender
			assert(obj);
			if (!obj)
				return;
			SPA::CAutoLock al(obj->m_cs);
			if (!obj->m_deqReqCb.size())
				return;
			Isolate* isolate = Isolate::GetCurrent();
			HandleScope handleScope(isolate); //required for Node 4.x
			while (obj->m_deqReqCb.size()) {
				ReqCb &cb = obj->m_deqReqCb.front();
				PAsyncServiceHandler processor;
				*cb.Buffer >> processor;
				assert(!processor);
				Local<v8::Object> njAsh;
				unsigned int sid = processor->GetSvsID();
				switch (sid) {
				case SPA::Sqlite::sidSqlite:
					njAsh = NJA::NJSqlite::New(isolate, (CSqlite*)processor, true);
					break;
				case SPA::Mysql::sidMysql:
					njAsh = NJA::NJMysql::New(isolate, (CMysql*)processor, true);
					break;
				case SPA::Odbc::sidOdbc:
					njAsh = NJA::NJOdbc::New(isolate, (COdbc*)processor, true);
					break;
				case SPA::Queue::sidQueue:
					njAsh = NJA::NJAsyncQueue::New(isolate, (NJA::CAQueue*)processor, true);
					break;
				case SPA::SFile::sidFile:
					njAsh = NJA::NJFile::New(isolate, (NJA::CSFile*)processor, true);
					break;
				default:
					njAsh = NJA::NJHandler::New(isolate, processor, true);
					break;
				}
				Local<Value> jsReqId = v8::Uint32::New(isolate, cb.ReqId);
				Local<Function> func;
				if (cb.Func)
					func = Local<Function>::New(isolate, *cb.Func);
				switch (cb.Type) {
				case eResult:
				{
					Local<Object> q = NJA::NJQueue::New(isolate, cb.Buffer);
					if (!func.IsEmpty()) {
						Local<Value> argv[] = { q, func, njAsh, jsReqId };
						func->Call(Null(isolate), 4, argv);
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
						Local<Value> argv[] = { Boolean::New(isolate, canceled), njAsh, jsReqId };
						func->Call(Null(isolate), 3, argv);
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
#ifdef WIN32_64
					Local<String> jsMsg = String::NewFromTwoByte(isolate, (const uint16_t*)errMsg.c_str(), String::kNormalString, (int)errMsg.size());
#else

#endif
					Local<String> jsWhere = String::NewFromUtf8(isolate, errWhere.c_str());
					Local<Value> jsCode = v8::Number::New(isolate, errCode);
					if (!func.IsEmpty()) {
						Local<Value> argv[] = { jsMsg, jsCode, jsWhere, func, njAsh, jsReqId };
						func->Call(Null(isolate), 6, argv);
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
	}
}
