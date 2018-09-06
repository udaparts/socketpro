
#pragma once

#include "../../../include/udb_client.h"

namespace SPA {
	namespace ClientSide {

		Local<v8::Object> CreateDb(Isolate* isolate, CAsyncServiceHandler *ash);

		template<unsigned int serviceId>
		void CAsyncDBHandler<serviceId>::req_cb(uv_async_t* handle) {
			CAsyncDBHandler<serviceId>* obj = (CAsyncDBHandler<serviceId>*)handle->data; //sender
			assert(obj);
			if (!obj)
				return;
			Isolate* isolate = Isolate::GetCurrent();
			HandleScope handleScope(isolate); //required for Node 4.x
			{
				SPA::CAutoLock al(obj->m_csDB);
				while (obj->m_deqDBCb.size()) {
					DBCb &cb = obj->m_deqDBCb.front();
					PAsyncDBHandler processor;
					*cb.Buffer >> processor;
					assert(!processor);
					Local<v8::Object> njDB = CreateDb(isolate, processor);
					Local<Function> func;
					assert(cb.Func);
					if (cb.Func)
						func = Local<Function>::New(isolate, *cb.Func);
					switch (cb.Type) {
					case eResult:
					{
						int res;
						std::wstring errMsg;
						*cb.Buffer >> res >> errMsg;
						std::string strMsg = SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size());
						assert(!cb.Buffer->GetSize());
						CScopeUQueue::Unlock(cb.Buffer);
						auto njRes = Int32::New(isolate, res);
						auto njMsg = String::NewFromUtf8(isolate, strMsg.c_str());
						if (!func.IsEmpty()) {
							Local<Value> argv[] = {njRes, njMsg, njDB };
							func->Call(isolate->GetCurrentContext(), Null(isolate), 3, argv);
						}
					}
					break;
					case eRowsetHeader:
					{
						assert(!cb.Buffer->GetSize());
						CScopeUQueue::Unlock(cb.Buffer);
						Local<Object> meta = Object::New(isolate);
						meta->Set(String::NewFromUtf8(isolate, "DBPath"), String::NewFromUtf8(isolate, ""));

						if (!func.IsEmpty()) {
							Local<Value> argv[] = { meta, njDB };
							func->Call(isolate->GetCurrentContext(), Null(isolate), 2, argv);
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
							Local<Value> argv[] = { b, njDB};
							func->Call(isolate->GetCurrentContext(), Null(isolate), 2, argv);
						}
					}
					break;
					default:
						assert(false);
						break;
					}
					


					obj->m_deqDBCb.pop_front();
				}
			}
			isolate->RunMicrotasks();
		}
	}
}
