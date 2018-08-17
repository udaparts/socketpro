
#include "stdafx.h"
#include "njobjects.h"
#include "spincludes.h"
#include "njqueue.h"

namespace NJA {

	void CreatePool(const FunctionCallbackInfo<Value>& args) {

	}

	void NewBuffer(const FunctionCallbackInfo<Value>& args) {
		auto &d = args[0];
		char *data = node::Buffer::Data(d);
		size_t len = node::Buffer::Length(d);
		len = 0;
	}

	void Destroy(const FunctionCallbackInfo<Value>& args) {

	}

	void InitAll(Local<Object> exports) {
		NODE_SET_METHOD(exports, "destroy", Destroy);
		NODE_SET_METHOD(exports, "newbuffer", NewBuffer);
		NODE_SET_METHOD(exports, "newPool", CreatePool);
		NJQueue::Init(exports);
		NJAObjects::Init(exports);
		AtExit(NJAObjects::At_Exit, exports->GetIsolate());
	}
	NODE_MODULE(NODE_GYP_MODULE_NAME, InitAll)
}
