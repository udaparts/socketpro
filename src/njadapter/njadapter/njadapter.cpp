
#include "stdafx.h"
#include "njobjects.h"
#include "spincludes.h"
#include "njqueue.h"
#include "njhandler.h"

namespace NJA {

	void PrepareLoop();

	void Destroy(const FunctionCallbackInfo<Value>& args) {

	}

	void InitAll(Local<Object> exports) {
		PrepareLoop();
		NODE_SET_METHOD(exports, "destroy", Destroy);
		NJQueue::Init(exports);
		NJSocketPool::Init(exports);
		NJHandler::Init(exports);
	}
	NODE_MODULE(NODE_GYP_MODULE_NAME, InitAll)
}
