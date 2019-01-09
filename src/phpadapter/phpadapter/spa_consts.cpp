#include "stdafx.h"
#include "spa_consts.h"

void tagZipLevel::RegisterInto(Php::Namespace &spa) {
	Php::Class<tagZipLevel> reg("tagZipLevel");
	reg.property("zlDefault", SPA::zlDefault, Php::Const);
	reg.property("zlBestSpeed", SPA::zlBestSpeed, Php::Const);
	reg.property("zlBestCompression", SPA::zlBestCompression, Php::Const);
	spa.add(reg);
}

void tagSocketOption::RegisterInto(Php::Namespace &spa) {
	Php::Class<tagSocketOption> reg("tagSocketOption");
	reg.property("soTcpNoDelay", SPA::soTcpNoDelay, Php::Const);
	reg.property("soReuseAddr", SPA::soReuseAddr, Php::Const);
	reg.property("soKeepAlive", SPA::soKeepAlive, Php::Const);
	reg.property("soSndBuf", SPA::soSndBuf, Php::Const);
	reg.property("soRcvBuf", SPA::soRcvBuf, Php::Const);
	spa.add(reg);
}

void tagSocketLevel::RegisterInto(Php::Namespace &spa) {
	Php::Class<tagSocketLevel> reg("tagSocketLevel");
	reg.property("slTcp", SPA::slTcp, Php::Const);
	reg.property("slSocket", SPA::slSocket, Php::Const);
	spa.add(reg);
}

void tagOperationSystem::RegisterInto(Php::Namespace &spa) {
	Php::Class<tagOperationSystem> reg("tagOperationSystem");
	reg.property("osWin", SPA::osWin, Php::Const);
	reg.property("osApple", SPA::osApple, Php::Const);
	reg.property("osMac", SPA::osMac, Php::Const);
	reg.property("osUnix", SPA::osUnix, Php::Const);
	reg.property("osLinux", SPA::osLinux, Php::Const);
	reg.property("osBSD", SPA::osBSD, Php::Const);
	reg.property("osAndroid", SPA::osAndroid, Php::Const);
	reg.property("osWinCE", SPA::osWinCE, Php::Const);
	reg.property("osWinPhone", SPA::osWinPhone, Php::Const);
	spa.add(reg);
}

void tagThreadApartment::RegisterInto(Php::Namespace &spa) {
	Php::Class<tagThreadApartment> reg("tagThreadApartment");
	reg.property("taNone", SPA::taNone, Php::Const);
	reg.property("taApartment", SPA::taApartment, Php::Const);
	reg.property("taFree", SPA::taFree, Php::Const);
	spa.add(reg);
}


void tagBaseRequestID::RegisterInto(Php::Namespace &spa) {

}

void tagChatRequestID::RegisterInto(Php::Namespace &spa) {

}

void BaseServiceID::RegisterInto(Php::Namespace &spa) {

}

void BaseExceptionCode::RegisterInto(Php::Namespace &spa) {

}

void tagEncryptionMethod::RegisterInto(Php::Namespace &spa) {

}

void tagSType::RegisterInto(Php::Namespace &spa) {

}

void tagQueueStatus::RegisterInto(Php::Namespace &spa) {

}

void tagOptimistic::RegisterInto(Php::Namespace &spa) {

}

void RegisterSpaConstsInto(Php::Namespace &spa) {
	tagZipLevel::RegisterInto(spa);
	tagSocketOption::RegisterInto(spa);
	tagSocketLevel::RegisterInto(spa);
	tagOperationSystem::RegisterInto(spa);
	tagThreadApartment::RegisterInto(spa);
	tagBaseRequestID::RegisterInto(spa);
	tagChatRequestID::RegisterInto(spa);
	BaseServiceID::RegisterInto(spa);
	BaseExceptionCode::RegisterInto(spa);
	tagEncryptionMethod::RegisterInto(spa);
	tagSType::RegisterInto(spa);
	tagQueueStatus::RegisterInto(spa);
	tagOptimistic::RegisterInto(spa);
}
