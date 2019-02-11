#include "stdafx.h"
#include "spa_cs_consts.h"

namespace PA {

	void tagConnectionState::__construct(Php::Parameters &params) {
	}
	void tagConnectionState::RegisterInto(Php::Namespace &cs) {
		Php::Class<tagConnectionState> reg("ConnState");
		reg.method(PHP_CONSTRUCT, &tagConnectionState::__construct, Php::Private);
		reg.property("Closed", SPA::ClientSide::csClosed, Php::Const);
		reg.property("Connecting", SPA::ClientSide::csConnecting, Php::Const);
		reg.property("SslShaking", SPA::ClientSide::csSslShaking, Php::Const);
		reg.property("Closing", SPA::ClientSide::csClosing, Php::Const);
		reg.property("Connected", SPA::ClientSide::csConnected, Php::Const);
		reg.property("Switched", SPA::ClientSide::csSwitched, Php::Const);
		cs.add(reg);
	}

	void RegisterSpaClientConstsInto(Php::Namespace &cs) {
		tagConnectionState::RegisterInto(cs);
	}
} //namespace PA
