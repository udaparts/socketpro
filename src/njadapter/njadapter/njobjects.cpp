#include "stdafx.h"
#include "njobjects.h"

namespace NJA {
	using v8::Context;

	Persistent<Function> NJSocketPool::constructor;

	NJSocketPool::NJSocketPool() : m_pPool(nullptr) {
		uv_loop_t *main_loop = uv_default_loop();
		/*
		uv_async_t context;
		memset(&context, 0, sizeof(uv_async_t));

		int err = uv_async_init(main_loop, &context, nullptr);
		*/
		main_loop = nullptr;
	}

	NJSocketPool::~NJSocketPool() {
		
	}

	void NJSocketPool::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();

		// Prepare constructor template
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(String::NewFromUtf8(isolate, "CSocketPool"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		//Prototype
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

		constructor.Reset(isolate, tpl->GetFunction());
		exports->Set(String::NewFromUtf8(isolate, "CSocketPool"), tpl->GetFunction());
	}

	void NJSocketPool::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		if (args.IsConstructCall()) {
			// Invoked as constructor: `new NJSocketPool(...)`
			double value = args[0]->IsNumber() ? args[0]->NumberValue() : 0;
			NJSocketPool* obj = new NJSocketPool();
			obj->Wrap(args.This());
			/*
			Local<Number> num = Number::New(isolate, 1.24);
			args.This()->Set(String::NewFromUtf8(isolate, "msg"), num);
			*/
			args.GetReturnValue().Set(args.This());
		}
		else {
			// Invoked as plain function `NJSocketPool(...)`, turn into construct call.
			Local<Value> argv[] = { args[0] };
			Local<Context> context = isolate->GetCurrentContext();
			Local<Function> cons = Local<Function>::New(isolate, constructor);
			Local<Object> result = cons->NewInstance(context, 1, argv).ToLocalChecked();
			args.GetReturnValue().Set(result);
		}
	}

	void NJSocketPool::DisconnectAll(const FunctionCallbackInfo<Value>& args) {

	}

	void NJSocketPool::getAsyncHandlers(const FunctionCallbackInfo<Value>& args) {

	}

	void NJSocketPool::getAvg(const FunctionCallbackInfo<Value>& args) {

	}

	void NJSocketPool::getConenctedSockets(const FunctionCallbackInfo<Value>& args) {

	}

	void NJSocketPool::getDisconnectedSockets(const FunctionCallbackInfo<Value>& args) {

	}

	void NJSocketPool::getIdleSockets(const FunctionCallbackInfo<Value>& args) {

	}

	void NJSocketPool::getLockedSockets(const FunctionCallbackInfo<Value>& args) {

	}

	void NJSocketPool::getPoolId(const FunctionCallbackInfo<Value>& args) {

	}

	void NJSocketPool::getQueueAutoMerge(const FunctionCallbackInfo<Value>& args) {

	}

	void NJSocketPool::setQueueAutoMerge(const FunctionCallbackInfo<Value>& args) {

	}
	void NJSocketPool::getQueueName(const FunctionCallbackInfo<Value>& args) {

	}
	void NJSocketPool::setQueueName(const FunctionCallbackInfo<Value>& args) {

	}
	void NJSocketPool::getQueues(const FunctionCallbackInfo<Value>& args) {

	}
	void NJSocketPool::getSockets(const FunctionCallbackInfo<Value>& args) {
	}
	void NJSocketPool::getSocketsPerThread(const FunctionCallbackInfo<Value>& args) {

	}

	void NJSocketPool::getStarted(const FunctionCallbackInfo<Value>& args) {

	}

	void NJSocketPool::getThreadsCreated(const FunctionCallbackInfo<Value>& args) {

	}

	void NJSocketPool::Lock(const FunctionCallbackInfo<Value>& args) {

	}
	void NJSocketPool::Seek(const FunctionCallbackInfo<Value>& args) {

	}
	void NJSocketPool::SeekByQueue(const FunctionCallbackInfo<Value>& args) {

	}
	void NJSocketPool::ShutdownPool(const FunctionCallbackInfo<Value>& args) {

	}
	void NJSocketPool::StartSocketPool(const FunctionCallbackInfo<Value>& args) {

	}
	void NJSocketPool::Unlock(const FunctionCallbackInfo<Value>& args) {

	}
}
