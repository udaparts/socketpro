#include "stdafx.h"
#include "spa_cs_consts.h"

namespace PA {

	void tagConnectionState::RegisterInto(Php::Namespace &cs) {
		Php::Class<tagConnectionState> reg("tagConnectionState");
		reg.property("csClosed", SPA::ClientSide::csClosed, Php::Const);
		reg.property("csConnecting", SPA::ClientSide::csConnecting, Php::Const);
		reg.property("csSslShaking", SPA::ClientSide::csSslShaking, Php::Const);
		reg.property("csClosing", SPA::ClientSide::csClosing, Php::Const);
		reg.property("csConnected", SPA::ClientSide::csConnected, Php::Const);
		reg.property("csSwitched", SPA::ClientSide::csSwitched, Php::Const);
		cs.add(reg);
	}

	void enumSocketPoolEvent::RegisterInto(Php::Namespace &cs) {
		Php::Class<enumSocketPoolEvent> reg("tagSocketPoolEvent");
		reg.property("speUnknown", SPA::ClientSide::speUnknown, Php::Const);
		reg.property("speStarted", SPA::ClientSide::speStarted, Php::Const);
		reg.property("speCreatingThread", SPA::ClientSide::speCreatingThread, Php::Const);
		reg.property("speThreadCreated", SPA::ClientSide::speThreadCreated, Php::Const);
		reg.property("speConnecting", SPA::ClientSide::speConnecting, Php::Const);
		reg.property("speConnected", SPA::ClientSide::speConnected, Php::Const);
		reg.property("speKillingThread", SPA::ClientSide::speKillingThread, Php::Const);
		reg.property("speShutdown", SPA::ClientSide::speShutdown, Php::Const);
		reg.property("speUSocketCreated", SPA::ClientSide::speUSocketCreated, Php::Const);
		reg.property("speHandShakeCompleted", SPA::ClientSide::speHandShakeCompleted, Php::Const);
		reg.property("speLocked", SPA::ClientSide::speLocked, Php::Const);
		reg.property("speUnlocked", SPA::ClientSide::speUnlocked, Php::Const);
		reg.property("speThreadKilled", SPA::ClientSide::speThreadKilled, Php::Const);
		reg.property("speClosingSocket", SPA::ClientSide::speClosingSocket, Php::Const);
		reg.property("speSocketClosed", SPA::ClientSide::speSocketClosed, Php::Const);
		reg.property("speUSocketKilled", SPA::ClientSide::speUSocketKilled, Php::Const);
		reg.property("speTimer", SPA::ClientSide::speTimer, Php::Const);
		reg.property("speQueueMergedFrom", SPA::ClientSide::speQueueMergedFrom, Php::Const);
		reg.property("speQueueMergedTo", SPA::ClientSide::speQueueMergedTo, Php::Const);

		cs.add(reg);
	}

	void RegisterSpaClientConstsInto(Php::Namespace &cs) {
		tagConnectionState::RegisterInto(cs);
		enumSocketPoolEvent::RegisterInto(cs);
	}
} //namespace PA
