
#include "stdafx.h"
#include "njobjects.h"
#include "njclientqueue.h"
#include "njpush.h"
#include "njcert.h"

namespace NJA {
	void GetVersion(const FunctionCallbackInfo<Value>& args) {
		auto isolate = args.GetIsolate();
		auto v = ToStr(isolate, ClientCoreLoader.GetUClientSocketVersion());
		args.GetReturnValue().Set(v);
	}

	void GetPools(const FunctionCallbackInfo<Value>& args) {
		auto isolate = args.GetIsolate();
		auto v = Uint32::New(isolate, ClientCoreLoader.GetNumberOfSocketPools());
		args.GetReturnValue().Set(v);
	}

	void SetVerifyLocation(const FunctionCallbackInfo<Value>& args) {
		auto isolate = args.GetIsolate();
		auto p0 = args[0];
		if (!p0->IsString()) {
			ThrowException(isolate, "A CA store location string required");
			return;
		}
		String::Utf8Value str(p0);
		unsigned int len = (unsigned int)str.length();
		if (!len) {
			ThrowException(isolate, "A CA store location string cannot be empty");
			return;
		}
		bool ok = ClientCoreLoader.SetVerifyLocation(*str);
		args.GetReturnValue().Set(Boolean::New(isolate, ok));
	}

	void GetWorkingDir(const FunctionCallbackInfo<Value>& args) {
		auto isolate = args.GetIsolate();
		auto v = ToStr(isolate, ClientCoreLoader.GetClientWorkDirectory());
		args.GetReturnValue().Set(v);
	}

	void SetWorkingDir(const FunctionCallbackInfo<Value>& args) {
		auto isolate = args.GetIsolate();
		auto p0 = args[0];
		if (!p0->IsString()) {
			ThrowException(isolate, "A working directory string required");
			return;
		}
		String::Utf8Value str(p0);
		unsigned int len = (unsigned int)str.length();
		if (!len) {
			ThrowException(isolate, "A working directory string cannot be empty");
			return;
		}
		ClientCoreLoader.SetClientWorkDirectory(*str);
	}

	void SetMessageQueuePassword(const FunctionCallbackInfo<Value>& args) {
		auto isolate = args.GetIsolate();
		auto p0 = args[0];
		if (!p0->IsString()) {
			ThrowException(isolate, "A password string required");
			return;
		}
		String::Utf8Value str(p0);
		ClientCoreLoader.SetMessageQueuePassword(*str);
	}

	void EnableSelfSigned(const FunctionCallbackInfo<Value>& args) {
		auto isolate = args.GetIsolate();
		auto p0 = args[0];
		if (p0->IsBoolean()) {
			g_bSelfSigned = p0->BooleanValue();
			args.GetReturnValue().Set(p0);
		}
		else if (p0->IsNullOrUndefined()) {
			g_bSelfSigned = false;
			args.GetReturnValue().Set(Boolean::New(isolate, false));
		}
		else {
			ThrowException(isolate, "A boolean value required");
		}
	}

	void InitAll(Local<Object> exports) {
		NODE_SET_METHOD(exports, "getVersion", GetVersion);
		NODE_SET_METHOD(exports, "getPools", GetPools);
		NODE_SET_METHOD(exports, "setCA", SetVerifyLocation);
		NODE_SET_METHOD(exports, "getWorkingDir", GetWorkingDir);
		NODE_SET_METHOD(exports, "setWorkingDir", SetWorkingDir);
		NODE_SET_METHOD(exports, "setPassword", SetMessageQueuePassword);
		NODE_SET_METHOD(exports, "EnableSelfSigned", EnableSelfSigned);
		NJQueue::Init(exports);
		NJSocketPool::Init(exports);
		NJHandler::Init(exports);
		NJFile::Init(exports);
		NJAsyncQueue::Init(exports);
		NJSqlite::Init(exports);
		NJSocket::Init(exports);
		NJClientQueue::Init(exports);
		NJPush::Init(exports);
		NJCert::Init(exports);
	}
	NODE_MODULE(NODE_GYP_MODULE_NAME, InitAll)
}
