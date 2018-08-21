#include "stdafx.h"
#include "njobjects.h"
#include "../../../include/async_sqlite.h"
#include "../../../include/mysql/umysql.h"

namespace NJA {
	using v8::Context;

	Persistent<Function> NJSocketPool::constructor;

	NJSocketPool::NJSocketPool(unsigned int id, bool autoConn, unsigned int recvTimeout, unsigned int connTimeout) 
	: SvsId(id), m_isolate(nullptr) {
		switch (id) {
		case SPA::Sqlite::sidSqlite:
		case SPA::Mysql::sidMysql:
			Db = new CSocketPool<CAsyncDBHandler<0>>(autoConn, recvTimeout, connTimeout, id);
			break;
		case SPA::Odbc::sidOdbc:
			Odbc = new CSocketPool<COdbc>(autoConn, recvTimeout, connTimeout, id);
			break;
		case SPA::Queue::sidQueue:
			Queue = new CSocketPool<CAsyncQueue>(autoConn, recvTimeout, connTimeout, id);
			break;
		case SPA::SFile::sidFile:
			File = new CSocketPool<CStreamingFile>(autoConn, recvTimeout, connTimeout, id);
			break;
		default:
			Handler = new CSocketPool<CAsyncHandler>(autoConn, recvTimeout, connTimeout, id);
			break;
		}
		::memset(&m_asyncType, 0, sizeof(m_asyncType));
		m_asyncType.data = this;
	}

	NJSocketPool::~NJSocketPool() {
		Release();
	}

	bool NJSocketPool::IsValid(Isolate* isolate) {
		if (!Handler) {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "SocketPool object already disposed")));
			return false;
		}
		return true;
	}

	void NJSocketPool::Release() {
		if (Handler) {
			switch (SvsId) {
			case SPA::Sqlite::sidSqlite:
			case SPA::Mysql::sidMysql:
				delete Db;
				break;
			case SPA::Odbc::sidOdbc:
				delete Odbc;
				break;
			case SPA::Queue::sidQueue:
				delete Queue;
				break;
			case SPA::SFile::sidFile:
				delete File;
				break;
			default:
				delete Handler;
				break;
			}
			Handler = nullptr;
		}
	}

	void NJSocketPool::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();

		// Prepare constructor template
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(String::NewFromUtf8(isolate, "CSocketPool"));
		tpl->InstanceTemplate()->SetInternalFieldCount(2);

		//Prototype
		NODE_SET_PROTOTYPE_METHOD(tpl, "Dispose", Dispose);
		NODE_SET_PROTOTYPE_METHOD(tpl, "DisconnectAll", DisconnectAll);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getAsyncHandlers", getAsyncHandlers);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getAvg", getAvg);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getConenctedSockets", getConenctedSockets);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getDisconnectedSockets", getDisconnectedSockets);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getIdleSockets", getIdleSockets);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getLockedSockets", getLockedSockets);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getPoolId", getPoolId);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getQueueAutoMerge", getQueueAutoMerge);
		NODE_SET_PROTOTYPE_METHOD(tpl, "setQueueAutoMerge", setQueueAutoMerge);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getQueueName", getQueueName);
		NODE_SET_PROTOTYPE_METHOD(tpl, "setQueueName", setQueueName);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getQueues", getQueues);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getSockets", getSockets);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getSocketsPerThread", getSocketsPerThread);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getStarted", getStarted);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getThreadsCreated", getThreadsCreated);
		NODE_SET_PROTOTYPE_METHOD(tpl, "Lock", Lock);
		NODE_SET_PROTOTYPE_METHOD(tpl, "Seek", Seek);
		NODE_SET_PROTOTYPE_METHOD(tpl, "SeekByQueue", SeekByQueue);
		NODE_SET_PROTOTYPE_METHOD(tpl, "ShutdownPool", ShutdownPool);
		NODE_SET_PROTOTYPE_METHOD(tpl, "StartSocketPool", StartSocketPool);
		NODE_SET_PROTOTYPE_METHOD(tpl, "Unlock", Unlock);
		NODE_SET_PROTOTYPE_METHOD(tpl, "setPoolEvent", setPoolEvent);
		NODE_SET_PROTOTYPE_METHOD(tpl, "setSSLAuthentication", setSSLAuthentication);

		constructor.Reset(isolate, tpl->GetFunction());
		exports->Set(String::NewFromUtf8(isolate, "CSocketPool"), tpl->GetFunction());
	}

	void NJSocketPool::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		if (args.IsConstructCall()) {
			bool autoConn = true;
			unsigned int svsId = 0, recvTimeout = SPA::ClientSide::DEFAULT_RECV_TIMEOUT, connTimeout = SPA::ClientSide::DEFAULT_CONN_TIMEOUT;
			if (args[0]->IsUint32()) {
				svsId = args[0]->Uint32Value();
			}
			if (!svsId) {
				isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "A non-zero unsigned int service id required")));
				return;
			}
			if (args[2]->IsBoolean())
				autoConn = args[2]->BooleanValue();
			if (args[3]->IsUint32())
				recvTimeout = args[3]->Uint32Value();
			if (args[5]->IsUint32())
				connTimeout = args[5]->Uint32Value();

			// Invoked as constructor: `new NJSocketPool(...)`
			NJSocketPool* obj = new NJSocketPool(svsId, autoConn, recvTimeout, connTimeout);
			obj->m_isolate = isolate;
			if (args[1]->IsFunction())
				obj->m_rr = Local<Function>::Cast(args[1]);
			if (args[4]->IsFunction())
				obj->m_mt = Local<Function>::Cast(args[4]);
			if (args[6]->IsFunction())
				obj->m_brp = Local<Function>::Cast(args[6]);
			if (args[7]->IsFunction())
				obj->m_se = Local<Function>::Cast(args[7]);
			if (args[8]->IsFunction())
				obj->m_ap = Local<Function>::Cast(args[8]);
			obj->Wrap(args.This());
			obj->Handler->DoSslServerAuthentication = [obj](CSocketPool<CAsyncHandler> *pool, SPA::ClientSide::CClientSocket *cs)->bool {
				if (!obj->m_ssl->IsFunction())
					return false;
				return false;
			};
			obj->Handler->SocketPoolEvent = [obj](CSocketPool<CAsyncHandler> *pool, tagSocketPoolEvent spe, CAsyncHandler *handler) {
				if (obj->m_evPool->IsFunction())
					return;
				PoolEvent pe;
				pe.Spe = spe;
				pe.Handler = handler;
				SPA::CAutoLock al(obj->m_cs);
				obj->m_deqPoolEvent.push_back(pe);
				int fail = uv_async_send(&obj->m_asyncType);
				assert(!fail);
			};
			int fail = uv_async_init(g_mainloop, &obj->m_asyncType, async_cb);
			assert(!fail);
			args.GetReturnValue().Set(args.This());
		}
		else {
			// Invoked as plain function `NJSocketPool(...)`, turn into construct call.
			Local<Value> argv[] = {args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8]};
			Local<Context> context = isolate->GetCurrentContext();
			Local<Function> cons = Local<Function>::New(isolate, constructor);
			Local<Object> result = cons->NewInstance(context, 9, argv).ToLocalChecked();
			args.GetReturnValue().Set(result);
		}
	}

	void NJSocketPool::async_cb(uv_async_t* handle) {
		NJSocketPool* obj = (NJSocketPool*)handle->data;
		SPA::CAutoLock al(obj->m_cs);
		while (obj->m_deqPoolEvent.size()) {
			const PoolEvent &pe = obj->m_deqPoolEvent.front();
			if (obj->m_evPool->IsFunction()) {
				Local<Value> argv[] = {Int32::New(obj->m_isolate, pe.Spe)};
				Local<Function> cb = Local<Function>::Cast(obj->m_evPool);
				cb->Call(Null(obj->m_isolate), 1, argv);
			}
			obj->m_deqPoolEvent.pop_front();
		}
		if (!obj->Handler) {
			uv_close((uv_handle_t*)handle, nullptr);
		}
	}

	void NJSocketPool::Dispose(const FunctionCallbackInfo<Value>& args) {
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		obj->Release();
	}

	void NJSocketPool::DisconnectAll(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->Handler->DisconnectAll();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJSocketPool::getAsyncHandlers(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {

		}
	}

	void NJSocketPool::getAvg(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->Handler->IsAvg();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJSocketPool::getConenctedSockets(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {
			unsigned int data = obj->Handler->GetConnectedSockets();
			args.GetReturnValue().Set(Uint32::New(isolate, data));
		}
	}

	void NJSocketPool::getDisconnectedSockets(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {
			unsigned int data = obj->Handler->GetDisconnectedSockets();
			args.GetReturnValue().Set(Uint32::New(isolate, data));
		}
	}

	void NJSocketPool::getIdleSockets(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {
			unsigned int data = obj->Handler->GetIdleSockets();
			args.GetReturnValue().Set(Uint32::New(isolate, data));
		}
	}

	void NJSocketPool::getLockedSockets(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {
			unsigned int data = obj->Handler->GetLockedSockets();
			args.GetReturnValue().Set(Uint32::New(isolate, data));
		}
	}

	void NJSocketPool::getPoolId(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {
			unsigned int data = obj->Handler->GetPoolId();
			args.GetReturnValue().Set(Uint32::New(isolate, data));
		}
	}

	void NJSocketPool::getQueueAutoMerge(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->Handler->GetQueueAutoMerge();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJSocketPool::setQueueAutoMerge(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {
			auto p = args[0];
			if (p->IsBoolean()) {
				bool b = p->BooleanValue();
				obj->Handler->SetQueueAutoMerge(b);
			}
			else {
				isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "A boolean value expected")));
			}
		}
	}

	void NJSocketPool::getQueueName(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {
			std::string queueName = obj->Handler->GetQueueName();
			args.GetReturnValue().Set(String::NewFromUtf8(isolate, queueName.c_str()));
		}
	}
	void NJSocketPool::setQueueName(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {
			auto p = args[0];
			if (p->IsString()) {
				String::Utf8Value str(p);
				obj->Handler->SetQueueName(*str);
			}
			else {
				isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "A boolean value expected")));
			}
		}
	}
	void NJSocketPool::getQueues(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {
			unsigned int data = obj->Handler->GetQueues();
			args.GetReturnValue().Set(Uint32::New(isolate, data));
		}
	}
	void NJSocketPool::getSockets(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {
			
		}
	}
	void NJSocketPool::getSocketsPerThread(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {
			unsigned int data = obj->Handler->GetSocketsPerThread();
			args.GetReturnValue().Set(Uint32::New(isolate, data));
		}
	}

	void NJSocketPool::getStarted(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->Handler->IsStarted();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJSocketPool::getThreadsCreated(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {
			unsigned int data = obj->Handler->GetThreadsCreated();
			args.GetReturnValue().Set(Uint32::New(isolate, data));
		}
	}

	void NJSocketPool::Lock(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {

		}
	}
	void NJSocketPool::Seek(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {

		}
	}
	void NJSocketPool::SeekByQueue(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {

		}
	}
	void NJSocketPool::ShutdownPool(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {
			obj->Handler->ShutdownPool();
		}
	}
	void NJSocketPool::StartSocketPool(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {

		}
	}
	void NJSocketPool::Unlock(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {

		}
	}

	void NJSocketPool::setSSLAuthentication(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {
			auto p = args[0];
			if (p->IsFunction()) {
				obj->m_ssl = Local<Function>::Cast(p);
			}
			else {
				isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "A callback expected for authenticating server certificate")));
			}
		}
	}

	void NJSocketPool::setPoolEvent(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
		if (obj->IsValid(isolate)) {
			auto p = args[0];
			if (p->IsFunction()) {
				SPA::CAutoLock al(obj->m_cs);
				obj->m_ssl = Local<Function>::Cast(p);
			}
			else {
				isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "A callback expected for tracking pool event")));
			}
		}
	}
}
