
#include "stdafx.h"
#include "njobjects.h"

namespace NJA {
	void InitAll(Local<Object> exports) {
		MyObject::Init(exports);
		AtExit(MyObject::At_Exit, exports->GetIsolate());
	}
	NODE_MODULE(NODE_GYP_MODULE_NAME, InitAll)
}
