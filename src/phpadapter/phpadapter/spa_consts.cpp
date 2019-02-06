#include "stdafx.h"
#include "spa_consts.h"

namespace PA {

	void tagZipLevel::__construct(Php::Parameters &params) {
	}

	void tagZipLevel::RegisterInto(Php::Namespace &spa) {
		Php::Class<tagZipLevel> reg("ZipLevel");
		reg.method(PHP_CONSTRUCT, &tagZipLevel::__construct, Php::Private);
		reg.property("Default", SPA::zlDefault, Php::Const);
		reg.property("BestSpeed", SPA::zlBestSpeed, Php::Const);
		reg.property("BestCompression", SPA::zlBestCompression, Php::Const);
		spa.add(reg);
	}

	void tagOperationSystem::__construct(Php::Parameters &params) {
	}
	void tagOperationSystem::RegisterInto(Php::Namespace &spa) {
		Php::Class<tagOperationSystem> reg("OperationSystem");
		reg.method(PHP_CONSTRUCT, &tagOperationSystem::__construct, Php::Private);
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
		reg.method(PHP_CONSTRUCT, &tagBaseRequestID::__construct, Php::Private);
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
		spa.add(reg);
	}

	void BaseServiceID::__construct(Php::Parameters &params) {
	}
	void BaseServiceID::RegisterInto(Php::Namespace &spa) {
		Php::Class<BaseServiceID> reg("SID");
		reg.method(PHP_CONSTRUCT, &BaseServiceID::__construct, Php::Private);
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
		reg.method(PHP_CONSTRUCT, &tagEncryptionMethod::__construct, Php::Private);
		reg.property("NoEncryption", SPA::NoEncryption, Php::Const);
		reg.property("TLSv1", SPA::TLSv1, Php::Const);
		spa.add(reg);
	}

	void tagQueueStatus::__construct(Php::Parameters &params) {
	}
	void tagQueueStatus::RegisterInto(Php::Namespace &spa) {
		Php::Class<tagQueueStatus> reg("QueueStatus");
		reg.method(PHP_CONSTRUCT, &tagQueueStatus::__construct, Php::Private);
		reg.property("Normal", SPA::qsNormal, Php::Const);
		reg.property("MergeComplete", SPA::qsMergeComplete, Php::Const);
		reg.property("MergePushing", SPA::qsMergePushing, Php::Const);
		reg.property("MergeIncomplete", SPA::qsMergeIncomplete, Php::Const);
		reg.property("JobIncomplete", SPA::qsJobIncomplete, Php::Const);
		reg.property("Crash", SPA::qsCrash, Php::Const);
		reg.property("FileError", SPA::qsFileError, Php::Const);
		reg.property("BadPassword", SPA::qsBadPassword, Php::Const);
		reg.property("DuplicateName", SPA::qsDuplicateName, Php::Const);
		spa.add(reg);
	}

	void tagOptimistic::__construct(Php::Parameters &params) {
	}
	void tagOptimistic::RegisterInto(Php::Namespace &spa) {
		Php::Class<tagOptimistic> reg("Optimistic");
		reg.method(PHP_CONSTRUCT, &tagOptimistic::__construct, Php::Private);
		reg.property("MemoryCached", SPA::oMemoryCached, Php::Const);
		reg.property("SystemMemoryCached", SPA::oSystemMemoryCached, Php::Const);
		reg.property("DiskCommitted", SPA::oDiskCommitted, Php::Const);
		spa.add(reg);
	}

	void RegisterSpaConstsInto(Php::Namespace &spa) {
		tagZipLevel::RegisterInto(spa);
		tagOperationSystem::RegisterInto(spa);
		tagBaseRequestID::RegisterInto(spa);
		BaseServiceID::RegisterInto(spa);
		tagEncryptionMethod::RegisterInto(spa);
		tagQueueStatus::RegisterInto(spa);
		tagOptimistic::RegisterInto(spa);
	}

} //namespace PA
