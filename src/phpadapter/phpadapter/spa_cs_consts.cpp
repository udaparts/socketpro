#include "stdafx.h"
#include "spa_cs_consts.h"

namespace PA {

	void tagConnectionState::RegisterInto(Php::Namespace &cs) {
		Php::Class<tagConnectionState> reg("ConnState");
		reg.property("csClosed", SPA::ClientSide::csClosed, Php::Const);
		reg.property("csConnecting", SPA::ClientSide::csConnecting, Php::Const);
		reg.property("csSslShaking", SPA::ClientSide::csSslShaking, Php::Const);
		reg.property("csClosing", SPA::ClientSide::csClosing, Php::Const);
		reg.property("csConnected", SPA::ClientSide::csConnected, Php::Const);
		reg.property("csSwitched", SPA::ClientSide::csSwitched, Php::Const);
		cs.add(reg);
	}

	void RegisterSpaClientConstsInto(Php::Namespace &cs) {
		tagConnectionState::RegisterInto(cs);
	}
} //namespace PA
