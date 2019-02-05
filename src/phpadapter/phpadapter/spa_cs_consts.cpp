#include "stdafx.h"
#include "spa_cs_consts.h"

namespace PA {

	void tagConnectionState::__construct(Php::Parameters &params) {
	}
	void tagConnectionState::RegisterInto(Php::Namespace &cs) {
		Php::Class<tagConnectionState> reg("ConnState");
		reg.method(PHP_CONSTRUCT, &tagConnectionState::__construct, Php::Private);
		reg.property("csClosed", SPA::ClientSide::csClosed, Php::Const);
		reg.property("csConnecting", SPA::ClientSide::csConnecting, Php::Const);
		reg.property("csSslShaking", SPA::ClientSide::csSslShaking, Php::Const);
		reg.property("csClosing", SPA::ClientSide::csClosing, Php::Const);
		reg.property("csConnected", SPA::ClientSide::csConnected, Php::Const);
		reg.property("csSwitched", SPA::ClientSide::csSwitched, Php::Const);
		cs.add(reg);
	}

	void CPoolType::__construct(Php::Parameters &params) {
	}
	void CPoolType::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPoolType> reg(KEY_POOL_TYPE);
		reg.method(PHP_CONSTRUCT, &CPoolType::__construct, Php::Private);
		reg.property("Regular", NotMS, Php::Const);
		reg.property("Slave", Slave, Php::Const);
		reg.property("Master", Master, Php::Const);
		cs.add(reg);
	}

	void RegisterSpaClientConstsInto(Php::Namespace &cs) {
		tagConnectionState::RegisterInto(cs);
		CPoolType::RegisterInto(cs);
	}
} //namespace PA