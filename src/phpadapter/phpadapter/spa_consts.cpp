#include "stdafx.h"
#include "spa_consts.h"

namespace PA {

	void tagOperationSystem::__construct(Php::Parameters &params) {
	}
	void tagOperationSystem::RegisterInto(Php::Namespace &spa) {
		Php::Class<tagOperationSystem> reg("OperationSystem");
		reg.method<&tagOperationSystem::__construct>(PHP_CONSTRUCT, Php::Private);
		reg.property("Win", SPA::osWin, Php::Const);
		reg.property("Apple", SPA::osApple, Php::Const);
		reg.property("Mac", SPA::osMac, Php::Const);
		reg.property("Unix", SPA::osUnix, Php::Const);
		reg.property("Linux", SPA::osLinux, Php::Const);
		reg.property("BSD", SPA::osBSD, Php::Const);
		reg.property("Android", SPA::osAndroid, Php::Const);
		reg.property("WinCE", SPA::osWinCE, Php::Const);
		reg.property("WinPhone", SPA::osWinPhone, Php::Const);
		spa.add(reg);
	}

	void tagBaseRequestID::__construct(Php::Parameters &params) {
	}
	void tagBaseRequestID::RegisterInto(Php::Namespace &spa) {
		Php::Class<tagBaseRequestID> reg("BaseID");
		reg.method<&tagBaseRequestID::__construct>(PHP_CONSTRUCT, Php::Private);
		reg.property("idUnknown", SPA::idUnknown, Php::Const);
		reg.property("idSwitchTo", SPA::idSwitchTo, Php::Const);
		reg.property("idRouteeChanged", SPA::idRouteeChanged, Php::Const);
		reg.property("idEncrypted", SPA::idEncrypted, Php::Const);
		reg.property("idBatchZipped", SPA::idBatchZipped, Php::Const);
		reg.property("idCancel", SPA::idCancel, Php::Const);
		reg.property("idGetSockOptAtSvr", SPA::idGetSockOptAtSvr, Php::Const);
		reg.property("idSetSockOptAtSvr", SPA::idSetSockOptAtSvr, Php::Const);
		reg.property("idDoEcho", SPA::idDoEcho, Php::Const);
		reg.property("idTurnOnZipAtSvr", SPA::idTurnOnZipAtSvr, Php::Const);
		reg.property("idStartBatching", SPA::idStartBatching, Php::Const);
		reg.property("idCommitBatching", SPA::idCommitBatching, Php::Const);
		reg.property("idStartMerge", SPA::idStartMerge, Php::Const);
		reg.property("idEndMerge", SPA::idEndMerge, Php::Const);
		reg.property("idPing", SPA::idPing, Php::Const);
		reg.property("idEnableClientDequeue", SPA::idEnableClientDequeue, Php::Const);
		reg.property("idServerException", SPA::idServerException, Php::Const);
		reg.property("idAllMessagesDequeued", SPA::idAllMessagesDequeued, Php::Const);
		reg.property("idHttpClose", SPA::idHttpClose, Php::Const);
		reg.property("idSetZipLevelAtSvr", SPA::idSetZipLevelAtSvr, Php::Const);
		reg.property("idStartJob", SPA::idStartJob, Php::Const);
		reg.property("idEndJob", SPA::idEndJob, Php::Const);
		reg.property("idRoutingData", SPA::idRoutingData, Php::Const);
		reg.property("idDequeueConfirmed", SPA::idDequeueConfirmed, Php::Const);
		reg.property("idMessageQueued", SPA::idMessageQueued, Php::Const);
		reg.property("idStartQueue", SPA::idStartQueue, Php::Const);
		reg.property("idStopQueue", SPA::idStopQueue, Php::Const);
		reg.property("idRoutePeerUnavailable", SPA::idRoutePeerUnavailable, Php::Const);
		reg.property("idReservedOne", SPA::idReservedOne, Php::Const);
		reg.property("idReservedTwo", SPA::idReservedTwo, Php::Const);
		
		//chat
		reg.property("idEnter", SPA::idEnter, Php::Const);
		reg.property("idSpeak", SPA::idSpeak, Php::Const);
		reg.property("idSpeakEx", SPA::idSpeakEx, Php::Const);
		reg.property("idExit", SPA::idExit, Php::Const);
		reg.property("idSendUserMessage", SPA::idSendUserMessage, Php::Const);
		reg.property("idSendUserMessageEx", SPA::idSendUserMessageEx, Php::Const);

		spa.add(reg);
	}

	void BaseServiceID::__construct(Php::Parameters &params) {
	}
	void BaseServiceID::RegisterInto(Php::Namespace &spa) {
		Php::Class<BaseServiceID> reg("SID");
		reg.method<&BaseServiceID::__construct>(PHP_CONSTRUCT, Php::Private);
		reg.property("sidReserved1", SPA::sidReserved1, Php::Const);
		reg.property("sidStartup", SPA::sidStartup, Php::Const);
		reg.property("sidChat", SPA::sidChat, Php::Const);
		reg.property("sidHTTP", SPA::sidHTTP, Php::Const);
		reg.property("sidFile", SPA::sidFile, Php::Const);
		reg.property("sidODBC", SPA::sidODBC, Php::Const);
		reg.property("sidQueue", SPA::sidChat, Php::Const);
		reg.property("sidReserved", SPA::sidReserved, Php::Const);
		reg.property("sidSqlite", (int32_t)SPA::Sqlite::sidSqlite, Php::Const);
		reg.property("sidMysql", (int32_t)SPA::Mysql::sidMysql, Php::Const);
		spa.add(reg);
	}

	void tagEncryptionMethod::__construct(Php::Parameters &params) {
	}
	void tagEncryptionMethod::RegisterInto(Php::Namespace &spa) {
		Php::Class<tagEncryptionMethod> reg("EM");
		reg.method<&tagEncryptionMethod::__construct>(PHP_CONSTRUCT, Php::Private);
		reg.property("NoEncryption", SPA::NoEncryption, Php::Const);
		reg.property("TLSv1", SPA::TLSv1, Php::Const);
		spa.add(reg);
	}

	void RegisterSpaConstsInto(Php::Namespace &spa) {
		tagOperationSystem::RegisterInto(spa);
		tagBaseRequestID::RegisterInto(spa);
		BaseServiceID::RegisterInto(spa);
		tagEncryptionMethod::RegisterInto(spa);
	}

} //namespace PA
