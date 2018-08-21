
#include "stdafx.h"
#include "objecttype.h"

namespace NJA {
	uv_loop_t *g_mainloop = nullptr;
	void PrepareLoop() {
		g_mainloop = uv_default_loop();
	}
}