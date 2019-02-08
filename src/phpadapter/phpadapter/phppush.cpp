#include "stdafx.h"
#include "phppush.h"

namespace PA {

	CPhpPush::CPhpPush(CPush &p) : Push(p) {
	}

	void CPhpPush::__construct(Php::Parameters &params) {
	}

	void CPhpPush::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpPush> push(PHP_PUSH);
		push.method(PHP_CONSTRUCT, &CPhpPush::__construct, Php::Private);

		cs.add(push);
	}
}
