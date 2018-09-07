
#pragma once

#include "../../../include/udb_client.h"

namespace SPA {
	namespace ClientSide {
		using namespace NJA;

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
						assert(!cb.Buffer->GetSize());
						CScopeUQueue::Unlock(cb.Buffer);
						auto njRes = Int32::New(isolate, res);
						auto njMsg = ToStr(isolate, errMsg.c_str());
						if (!func.IsEmpty()) {
							Local<Value> argv[] = {njRes, njMsg, njDB };
							func->Call(isolate->GetCurrentContext(), Null(isolate), 3, argv);
						}
					}
					break;
					case eRowsetHeader:
					{
						bool ok;
						CDBColumnInfoArray v;
						*cb.Buffer >> v;
						assert(!cb.Buffer->GetSize());
						CScopeUQueue::Unlock(cb.Buffer);
						Local<Array> jsMeta = Array::New(isolate);
						unsigned int index = 0;
						for (auto it = v.begin(), end = v.end(); it != end; ++it, ++index) {
							Local<Object> meta = Object::New(isolate);
							ok = meta->Set(ToStr(isolate, "DBPath"), ToStr(isolate, it->DBPath.c_str()));
							ok = meta->Set(ToStr(isolate, "TablePath"), ToStr(isolate, it->TablePath.c_str()));
							ok = meta->Set(ToStr(isolate, "DisplayName"), ToStr(isolate, it->DisplayName.c_str()));
							ok = meta->Set(ToStr(isolate, "OriginalName"), ToStr(isolate, it->OriginalName.c_str()));
							ok = meta->Set(ToStr(isolate, "DeclaredType"), ToStr(isolate, it->DeclaredType.c_str()));
							ok = meta->Set(ToStr(isolate, "Collation"), ToStr(isolate, it->Collation.c_str()));
							ok = meta->Set(ToStr(isolate, "ColumnSize"), Uint32::New(isolate, it->ColumnSize));
							ok = meta->Set(ToStr(isolate, "Flags"), Uint32::New(isolate, it->Flags));
							ok = meta->Set(ToStr(isolate, "DataType"), Uint32::New(isolate, it->DataType));
							ok = meta->Set(ToStr(isolate, "Precision"), Uint32::New(isolate, it->Precision));
							ok = meta->Set(ToStr(isolate, "Scale"), Uint32::New(isolate, it->Scale));
							ok = jsMeta->Set(index, meta);
						}
						if (!func.IsEmpty()) {
							Local<Value> argv[] = { jsMeta, njDB };
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
