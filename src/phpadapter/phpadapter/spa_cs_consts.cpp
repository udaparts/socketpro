#include "stdafx.h"
#include "spa_cs_consts.h"

namespace PA
{

    void tagConnectionState::__construct(Php::Parameters & params) {
    }

    void tagConnectionState::RegisterInto(Php::Namespace & cs) {
        Php::Class<tagConnectionState> reg("ConnState");
        reg.method<&tagConnectionState::__construct>(PHP_CONSTRUCT, Php::Private);
        reg.property("Closed", (int) SPA::ClientSide::tagConnectionState::csClosed, Php::Const);
        reg.property("Connecting", (int) SPA::ClientSide::tagConnectionState::csConnecting, Php::Const);
        reg.property("SslShaking", (int) SPA::ClientSide::tagConnectionState::csSslShaking, Php::Const);
        reg.property("Closing", (int) SPA::ClientSide::tagConnectionState::csClosing, Php::Const);
        reg.property("Connected", (int) SPA::ClientSide::tagConnectionState::csConnected, Php::Const);
        reg.property("Switched", (int) SPA::ClientSide::tagConnectionState::csSwitched, Php::Const);
        cs.add(reg);
    }

    void RegisterSpaClientConstsInto(Php::Namespace & cs) {
        tagConnectionState::RegisterInto(cs);
    }
} //namespace PA
