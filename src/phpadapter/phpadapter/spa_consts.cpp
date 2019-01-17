#include "stdafx.h"
#include "spa_consts.h"

namespace PA {

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
		Php::Class<tagBaseRequestID> reg("tagBaseRequestID");
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

	void tagChatRequestID::RegisterInto(Php::Namespace &spa) {
		Php::Class<tagChatRequestID> reg("tagChatRequestID");
		reg.property("idEnter", SPA::idEnter, Php::Const);
		reg.property("idSpeak", SPA::idSpeak, Php::Const);
		reg.property("idSpeakEx", SPA::idSpeakEx, Php::Const);
		reg.property("idExit", SPA::idExit, Php::Const);
		reg.property("idSendUserMessage", SPA::idSendUserMessage, Php::Const);
		reg.property("idSendUserMessageEx", SPA::idSendUserMessageEx, Php::Const);
		spa.add(reg);
	}

	void BaseServiceID::RegisterInto(Php::Namespace &spa) {
		Php::Class<BaseServiceID> reg("BaseServiceID");
		reg.property("sidReserved1", SPA::sidReserved1, Php::Const);
		reg.property("sidStartup", SPA::sidStartup, Php::Const);
		reg.property("sidChat", SPA::sidChat, Php::Const);
		reg.property("sidHTTP", SPA::sidHTTP, Php::Const);
		reg.property("sidFile", SPA::sidFile, Php::Const);
		reg.property("sidODBC", SPA::sidODBC, Php::Const);
		reg.property("sidQueue", SPA::sidChat, Php::Const);
		reg.property("sidMysql", (int32_t)SPA::Mysql::sidMysql, Php::Const);
		reg.property("sidSqlite", (int32_t)SPA::Sqlite::sidSqlite, Php::Const);
		reg.property("sidReserved", SPA::sidReserved, Php::Const);
		spa.add(reg);
	}

	void BaseExceptionCode::RegisterInto(Php::Namespace &spa) {
		Php::Class<BaseExceptionCode> reg("BaseExceptionCode");
		reg.property("becBAD_DESERIALIZATION", (int64_t)MB_BAD_DESERIALIZATION, Php::Const);
		reg.property("becSERIALIZATION_NOT_SUPPORTED", (int64_t)MB_SERIALIZATION_NOT_SUPPORTED, Php::Const);
		reg.property("becBAD_OPERATION", (int64_t)MB_BAD_OPERATION, Php::Const);
		reg.property("becBAD_INPUT", (int64_t)MB_BAD_INPUT, Php::Const);
		reg.property("becNOT_SUPPORTED", (int64_t)MB_NOT_SUPPORTED, Php::Const);
		reg.property("becSTL_EXCEPTION", (int64_t)MB_STL_EXCEPTION, Php::Const);
		reg.property("becUNKNOWN_EXCEPTION", (int64_t)MB_UNKNOWN_EXCEPTION, Php::Const);
		reg.property("becQUEUE_FILE_NOT_AVAILABLE", (int64_t)MB_QUEUE_FILE_NOT_AVAILABLE, Php::Const);
		reg.property("becALREADY_DEQUEUED", (int64_t)MB_ALREADY_DEQUEUED, Php::Const);
		reg.property("becROUTEE_DISCONNECTED", (int64_t)MB_ROUTEE_DISCONNECTED, Php::Const);
		reg.property("becREQUEST_ABORTED", (int64_t)MB_REQUEST_ABORTED, Php::Const);
		spa.add(reg);
	}

	void tagEncryptionMethod::RegisterInto(Php::Namespace &spa) {
		Php::Class<tagEncryptionMethod> reg("tagEncryptionMethod");
		reg.property("NoEncryption", SPA::NoEncryption, Php::Const);
		reg.property("TLSv1", SPA::TLSv1, Php::Const);
		spa.add(reg);
	}

	void tagSType::RegisterInto(Php::Namespace &spa) {
		Php::Class<tagSType> reg("tagShutdownType");
		reg.property("stReceive", SPA::stReceive, Php::Const);
		reg.property("stSend", SPA::stSend, Php::Const);
		reg.property("stBoth", SPA::stBoth, Php::Const);
		spa.add(reg);
	}

	void tagQueueStatus::RegisterInto(Php::Namespace &spa) {
		Php::Class<tagQueueStatus> reg("tagQueueStatus");
		reg.property("qsNormal", SPA::qsNormal, Php::Const);
		reg.property("qsMergeComplete", SPA::qsMergeComplete, Php::Const);
		reg.property("qsMergePushing", SPA::qsMergePushing, Php::Const);
		reg.property("qsMergeIncomplete", SPA::qsMergeIncomplete, Php::Const);
		reg.property("qsJobIncomplete", SPA::qsJobIncomplete, Php::Const);
		reg.property("qsCrash", SPA::qsCrash, Php::Const);
		reg.property("qsFileError", SPA::qsFileError, Php::Const);
		reg.property("qsBadPassword", SPA::qsBadPassword, Php::Const);
		reg.property("qsDuplicateName", SPA::qsDuplicateName, Php::Const);
		spa.add(reg);
	}

	void tagOptimistic::RegisterInto(Php::Namespace &spa) {
		Php::Class<tagOptimistic> reg("tagOptimistic");
		reg.property("oMemoryCached", SPA::oMemoryCached, Php::Const);
		reg.property("oSystemMemoryCached", SPA::oSystemMemoryCached, Php::Const);
		reg.property("oDiskCommitted", SPA::oDiskCommitted, Php::Const);
		spa.add(reg);
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

} //namespace PA
