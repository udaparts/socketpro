#include "stdafx.h"
#include "spa_consts.h"

namespace PA
{

    void tagOperationSystem::__construct(Php::Parameters & params) {
    }

    void tagOperationSystem::RegisterInto(Php::Namespace & spa) {
        Php::Class<tagOperationSystem> reg("OperationSystem");
        reg.method<&tagOperationSystem::__construct>(PHP_CONSTRUCT, Php::Private);
        reg.property("Win", (int) SPA::tagOperationSystem::osWin, Php::Const);
        reg.property("Apple", (int) SPA::tagOperationSystem::osApple, Php::Const);
        reg.property("Mac", (int) SPA::tagOperationSystem::osMac, Php::Const);
        reg.property("Unix", (int) SPA::tagOperationSystem::osUnix, Php::Const);
        reg.property("Linux", (int) SPA::tagOperationSystem::osLinux, Php::Const);
        reg.property("BSD", (int) SPA::tagOperationSystem::osBSD, Php::Const);
        reg.property("Android", (int) SPA::tagOperationSystem::osAndroid, Php::Const);
        reg.property("WinCE", (int) SPA::tagOperationSystem::osWinCE, Php::Const);
        reg.property("WinPhone", (int) SPA::tagOperationSystem::osWinPhone, Php::Const);
        spa.add(reg);
    }

    void tagBaseRequestID::__construct(Php::Parameters & params) {
    }

    void tagBaseRequestID::RegisterInto(Php::Namespace & spa) {
        Php::Class<tagBaseRequestID> reg("BaseID");
        reg.method<&tagBaseRequestID::__construct>(PHP_CONSTRUCT, Php::Private);
        reg.property("idUnknown", (unsigned short) SPA::tagBaseRequestID::idUnknown, Php::Const);
        reg.property("idSwitchTo", (unsigned short) SPA::tagBaseRequestID::idSwitchTo, Php::Const);
        reg.property("idRouteeChanged", (unsigned short) SPA::tagBaseRequestID::idRouteeChanged, Php::Const);
        reg.property("idEncrypted", (unsigned short) SPA::tagBaseRequestID::idEncrypted, Php::Const);
        reg.property("idBatchZipped", (unsigned short) SPA::tagBaseRequestID::idBatchZipped, Php::Const);
        reg.property("idCancel", (unsigned short) SPA::tagBaseRequestID::idCancel, Php::Const);
        reg.property("idGetSockOptAtSvr", (unsigned short) SPA::tagBaseRequestID::idGetSockOptAtSvr, Php::Const);
        reg.property("idSetSockOptAtSvr", (unsigned short) SPA::tagBaseRequestID::idSetSockOptAtSvr, Php::Const);
        reg.property("idDoEcho", (unsigned short) SPA::tagBaseRequestID::idDoEcho, Php::Const);
        reg.property("idTurnOnZipAtSvr", (unsigned short) SPA::tagBaseRequestID::idTurnOnZipAtSvr, Php::Const);
        reg.property("idStartBatching", (unsigned short) SPA::tagBaseRequestID::idStartBatching, Php::Const);
        reg.property("idCommitBatching", (unsigned short) SPA::tagBaseRequestID::idCommitBatching, Php::Const);
        reg.property("idStartMerge", (unsigned short) SPA::tagBaseRequestID::idStartMerge, Php::Const);
        reg.property("idEndMerge", (unsigned short) SPA::tagBaseRequestID::idEndMerge, Php::Const);
        reg.property("idPing", (unsigned short) SPA::tagBaseRequestID::idPing, Php::Const);
        reg.property("idEnableClientDequeue", (unsigned short) SPA::tagBaseRequestID::idEnableClientDequeue, Php::Const);
        reg.property("idServerException", (unsigned short) SPA::tagBaseRequestID::idServerException, Php::Const);
        reg.property("idAllMessagesDequeued", (unsigned short) SPA::tagBaseRequestID::idAllMessagesDequeued, Php::Const);
        reg.property("idHttpClose", (unsigned short) SPA::tagBaseRequestID::idHttpClose, Php::Const);
        reg.property("idSetZipLevelAtSvr", (unsigned short) SPA::tagBaseRequestID::idSetZipLevelAtSvr, Php::Const);
        reg.property("idStartJob", (unsigned short) SPA::tagBaseRequestID::idStartJob, Php::Const);
        reg.property("idEndJob", (unsigned short) SPA::tagBaseRequestID::idEndJob, Php::Const);
        reg.property("idRoutingData", (unsigned short) SPA::tagBaseRequestID::idRoutingData, Php::Const);
        reg.property("idDequeueConfirmed", (unsigned short) SPA::tagBaseRequestID::idDequeueConfirmed, Php::Const);
        reg.property("idMessageQueued", (unsigned short) SPA::tagBaseRequestID::idMessageQueued, Php::Const);
        reg.property("idStartQueue", (unsigned short) SPA::tagBaseRequestID::idStartQueue, Php::Const);
        reg.property("idStopQueue", (unsigned short) SPA::tagBaseRequestID::idStopQueue, Php::Const);
        reg.property("idRoutePeerUnavailable", (unsigned short) SPA::tagBaseRequestID::idRoutePeerUnavailable, Php::Const);
        reg.property("idDequeueBatchConfirmed", (unsigned short) SPA::tagBaseRequestID::idDequeueBatchConfirmed, Php::Const);
        reg.property("idInterrupt", (unsigned short) SPA::tagBaseRequestID::idInterrupt, Php::Const);
        reg.property("idReservedOne", (unsigned short) SPA::tagBaseRequestID::idReservedOne, Php::Const);
        reg.property("idReservedTwo", (unsigned short) SPA::tagBaseRequestID::idReservedTwo, Php::Const);

        //chat
        reg.property("idEnter", (unsigned short) SPA::tagChatRequestID::idEnter, Php::Const);
        reg.property("idSpeak", (unsigned short) SPA::tagChatRequestID::idSpeak, Php::Const);
        reg.property("idSpeakEx", (unsigned short) SPA::tagChatRequestID::idSpeakEx, Php::Const);
        reg.property("idExit", (unsigned short) SPA::tagChatRequestID::idExit, Php::Const);
        reg.property("idSendUserMessage", (unsigned short) SPA::tagChatRequestID::idSendUserMessage, Php::Const);
        reg.property("idSendUserMessageEx", (unsigned short) SPA::tagChatRequestID::idSendUserMessageEx, Php::Const);

        spa.add(reg);
    }

    void BaseServiceID::__construct(Php::Parameters & params) {
    }

    void BaseServiceID::RegisterInto(Php::Namespace & spa) {
        Php::Class<BaseServiceID> reg("SID");
        reg.method<&BaseServiceID::__construct>(PHP_CONSTRUCT, Php::Private);
        reg.property("sidReserved1", (int64_t) SPA::tagServiceID::sidReserved1, Php::Const);
        reg.property("sidStartup", (int64_t) SPA::tagServiceID::sidStartup, Php::Const);
        reg.property("sidChat", (int64_t) SPA::tagServiceID::sidChat, Php::Const);
        reg.property("sidHTTP", (int64_t) SPA::tagServiceID::sidHTTP, Php::Const);
        reg.property("sidFile", (int64_t) SPA::tagServiceID::sidFile, Php::Const);
        reg.property("sidODBC", (int64_t) SPA::tagServiceID::sidODBC, Php::Const);
        reg.property("sidQueue", (int64_t) SPA::tagServiceID::sidChat, Php::Const);
        reg.property("sidReserved", (int64_t) SPA::tagServiceID::sidReserved, Php::Const);
        reg.property("sidSqlite", (int64_t) SPA::Sqlite::sidSqlite, Php::Const);
        reg.property("sidMysql", (int64_t) SPA::Mysql::sidMysql, Php::Const);
        spa.add(reg);
    }

    void tagEncryptionMethod::__construct(Php::Parameters & params) {
    }

    void tagEncryptionMethod::RegisterInto(Php::Namespace & spa) {
        Php::Class<tagEncryptionMethod> reg("EM");
        reg.method<&tagEncryptionMethod::__construct>(PHP_CONSTRUCT, Php::Private);
        reg.property("NoEncryption", (int) SPA::tagEncryptionMethod::NoEncryption, Php::Const);
        reg.property("TLSv1", (int) SPA::tagEncryptionMethod::TLSv1, Php::Const);
        spa.add(reg);
    }

    void RegisterSpaConstsInto(Php::Namespace & spa) {
        tagOperationSystem::RegisterInto(spa);
        tagBaseRequestID::RegisterInto(spa);
        BaseServiceID::RegisterInto(spa);
        tagEncryptionMethod::RegisterInto(spa);
    }

} //namespace PA
