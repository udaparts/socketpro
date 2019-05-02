

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Mon Dec 28 20:19:43 2009
 */
/* Compiler settings for .\USocket.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __USocket_h__
#define __USocket_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IJSSerialization_FWD_DEFINED__
#define __IJSSerialization_FWD_DEFINED__
typedef interface IJSSerialization IJSSerialization;
#endif 	/* __IJSSerialization_FWD_DEFINED__ */


#ifndef __IUChat_FWD_DEFINED__
#define __IUChat_FWD_DEFINED__
typedef interface IUChat IUChat;
#endif 	/* __IUChat_FWD_DEFINED__ */


#ifndef __ISocketBase_FWD_DEFINED__
#define __ISocketBase_FWD_DEFINED__
typedef interface ISocketBase ISocketBase;
#endif 	/* __ISocketBase_FWD_DEFINED__ */


#ifndef __IUSocket_FWD_DEFINED__
#define __IUSocket_FWD_DEFINED__
typedef interface IUSocket IUSocket;
#endif 	/* __IUSocket_FWD_DEFINED__ */


#ifndef __IUFast_FWD_DEFINED__
#define __IUFast_FWD_DEFINED__
typedef interface IUFast IUFast;
#endif 	/* __IUFast_FWD_DEFINED__ */


#ifndef __IUZip_FWD_DEFINED__
#define __IUZip_FWD_DEFINED__
typedef interface IUZip IUZip;
#endif 	/* __IUZip_FWD_DEFINED__ */


#ifndef __IUCert_FWD_DEFINED__
#define __IUCert_FWD_DEFINED__
typedef interface IUCert IUCert;
#endif 	/* __IUCert_FWD_DEFINED__ */


#ifndef ___IUSocketEvent_FWD_DEFINED__
#define ___IUSocketEvent_FWD_DEFINED__
typedef interface _IUSocketEvent _IUSocketEvent;
#endif 	/* ___IUSocketEvent_FWD_DEFINED__ */


#ifndef __IUSocketPool_FWD_DEFINED__
#define __IUSocketPool_FWD_DEFINED__
typedef interface IUSocketPool IUSocketPool;
#endif 	/* __IUSocketPool_FWD_DEFINED__ */


#ifndef __IUSocketScript_FWD_DEFINED__
#define __IUSocketScript_FWD_DEFINED__
typedef interface IUSocketScript IUSocketScript;
#endif 	/* __IUSocketScript_FWD_DEFINED__ */


#ifndef __USocket_FWD_DEFINED__
#define __USocket_FWD_DEFINED__

#ifdef __cplusplus
typedef class USocket USocket;
#else
typedef struct USocket USocket;
#endif /* __cplusplus */

#endif 	/* __USocket_FWD_DEFINED__ */


#ifndef ___IUSocketPoolEvents_FWD_DEFINED__
#define ___IUSocketPoolEvents_FWD_DEFINED__
typedef interface _IUSocketPoolEvents _IUSocketPoolEvents;
#endif 	/* ___IUSocketPoolEvents_FWD_DEFINED__ */


#ifndef __USocketPool_FWD_DEFINED__
#define __USocketPool_FWD_DEFINED__

#ifdef __cplusplus
typedef class USocketPool USocketPool;
#else
typedef struct USocketPool USocketPool;
#endif /* __cplusplus */

#endif 	/* __USocketPool_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __USOCKETLib_LIBRARY_DEFINED__
#define __USOCKETLib_LIBRARY_DEFINED__

/* library USOCKETLib */
/* [helpstring][version][uuid] */ 


enum tagAddressFamily
    {	afUnix	= 1,
	afINet	= 2,
	afImpLink	= 3,
	afPup	= 4,
	afChaos	= 5,
	afNS	= 6,
	afIPX	= afNS,
	afISO	= 7,
	afOSI	= afISO,
	afECMA	= 8,
	afDataKit	= 9,
	afCCITT	= 10,
	afSNA	= 11,
	afDECnet	= 12,
	afDLI	= 13,
	afLAT	= 14,
	afHyLink	= 15,
	afAppleTalk	= 16,
	afNetBIOS	= 17,
	afVoiceView	= 18,
	afFireFox	= 19,
	afUnknown1	= 20,
	afBan	= 21,
	afATM	= 22,
	afINet6	= 23,
	afCluster	= 24,
	af12844	= 25,
	afMax	= 26
    } ;

enum tagSocketType
    {	stSTREAM	= 1,
	stDGRAM	= 2,
	stRAW	= 3,
	stRDM	= 4,
	stSEQPACKET	= 5
    } ;

enum tagSocketProtocol
    {	spIP	= 0,
	spICMP	= 1,
	spIGMP	= 2,
	spTCP	= 6,
	spUDP	= 17
    } ;

enum tagSocketLevel
    {	slTcp	= 0x6,
	slSocket	= 0xffff
    } ;

enum tagSocketOption
    {	soTcpNoDelay	= 0x1,
	soDebug	= 0x1,
	soAcceptConn	= 0x2,
	soReuseAddr	= 0x4,
	soKeepAlive	= 0x8,
	soDontRoute	= 0x10,
	soBroadcast	= 0x20,
	soUseLoopback	= 0x40,
	soLinger	= 0x80,
	soOOBInLine	= 0x100,
	soSndBuf	= 0x1001,
	soRcvBuf	= 0x1002,
	soSndLowAt	= 0x1003,
	soRcvLowAt	= 0x1004,
	soSndTimeout	= 0x1005,
	soRcvTimeout	= 0x1006,
	soError	= 0x1007,
	soType	= 0x1008,
	soGroupID	= 0x2001,
	soGroupPriority	= 0x2002,
	soMaxMsgSize	= 0x2003
    } ;

enum tagSocketError
    {	seSSLError	= 1,
	seSSLWantRead	= 2,
	seSSLWantWrite	= 3,
	seSSLWantX509Lookup	= 4,
	seSSLSysCall	= 5,
	seSSLZeroReturn	= 6,
	seSSLWantConnect	= 7,
	seSSLWantAccept	= 8,
	seClientAuthenticationError	= 9,
	seBase	= 10000,
	seWouldBlock	= ( seBase + 35 ) ,
	seInProgress	= ( seWouldBlock + 1 ) ,
	seAlready	= ( seInProgress + 1 ) ,
	seNotSock	= ( seAlready + 1 ) ,
	seDestAddrReq	= ( seNotSock + 1 ) ,
	seMsgSize	= ( seDestAddrReq + 1 ) ,
	seProtoType	= ( seMsgSize + 1 ) ,
	seNoProtoOpt	= ( seProtoType + 1 ) ,
	seProtoNoSupport	= ( seNoProtoOpt + 1 ) ,
	seSocktNoSupport	= ( seProtoNoSupport + 1 ) ,
	seOpNotSupp	= ( seSocktNoSupport + 1 ) ,
	sePFNoSupport	= ( seOpNotSupp + 1 ) ,
	seAFNoSupport	= ( sePFNoSupport + 1 ) ,
	seAddrInUse	= ( seAFNoSupport + 1 ) ,
	seAddrNotAvail	= ( seAddrInUse + 1 ) ,
	seNetDown	= ( seAddrNotAvail + 1 ) ,
	seNetUnreach	= ( seNetDown + 1 ) ,
	setNetReset	= ( seNetUnreach + 1 ) ,
	seConnAborted	= ( setNetReset + 1 ) ,
	seConnReset	= ( seConnAborted + 1 ) ,
	seNoBufs	= ( seConnReset + 1 ) ,
	seIsConn	= ( seNoBufs + 1 ) ,
	seNotConn	= ( seIsConn + 1 ) ,
	seShutDown	= ( seNotConn + 1 ) ,
	seTooManyRefs	= ( seShutDown + 1 ) ,
	seTimedOut	= ( seTooManyRefs + 1 ) ,
	seConnRefused	= ( seTimedOut + 1 ) ,
	seLoop	= ( seConnRefused + 1 ) ,
	seNameTooLong	= ( seLoop + 1 ) ,
	seHostDown	= ( seNameTooLong + 1 ) ,
	seHostUnreach	= ( seHostDown + 1 ) ,
	seNotEmpty	= ( seHostUnreach + 1 ) ,
	seProclim	= ( seNotEmpty + 1 ) ,
	seUsers	= ( seProclim + 1 ) ,
	seDquot	= ( seUsers + 1 ) ,
	seStale	= ( seDquot + 1 ) ,
	seRemote	= ( seStale + 1 ) 
    } ;

enum tagSpecialSocket
    {	ssInvalidSocket	= -1
    } ;

enum tagEncryptionMethod
    {	NoEncryption	= 0,
	BlowFish	= 1,
	SSL23	= 2,
	MSSSL	= 3,
	TLSv1	= 4,
	MSTLSv1	= 5
    } ;

enum tagSSLEvent
    {	ssleHandshakeStarted	= 0x10,
	ssleHandshakeDone	= 0x20,
	ssleHandshakeLoop	= 0x1001,
	ssleHandshakeExit	= 0x1002,
	ssleReadAlert	= 0x4004,
	ssleWriteAlert	= 0x4008,
	ssleDoSSLConnect	= 0x811f
    } ;

enum tagHowtoShutdown
    {	hsReceive	= 0,
	hsSend	= 0x1,
	hsBoth	= 0x2
    } ;

enum tagIOCommand
    {	iocRead	= 0x4004667f,
	iocOOB	= 0x40047307,
	iocBIO	= 0x8004667e
    } ;

enum tagInterfaceType
    {	itOther	= 1,
	itEthernet	= 6,
	itTokenRing	= 9,
	itFDDI	= 15,
	itPPP	= 23,
	itLoopback	= 24,
	itSlip	= 28
    } ;

enum tagNetPerformance
    {	npUnknown	= 0,
	npVerySlow	= ( npUnknown + 1 ) ,
	npSlow	= ( npVerySlow + 1 ) ,
	npMiddle	= ( npSlow + 1 ) ,
	npFast	= ( npMiddle + 1 ) ,
	npSuper	= ( npFast + 1 ) 
    } ;

enum tagClientMessage
    {	msgSocketEvent	= 0x781,
	msgAsyncGetHostByName	= ( msgSocketEvent + 1 ) ,
	msgWSAGetHostByName	= ( msgAsyncGetHostByName + 1 ) ,
	msgWSAGetHostByAddr	= ( msgWSAGetHostByName + 1 ) ,
	msgSSLEvent	= ( msgWSAGetHostByAddr + 1 ) ,
	msgAllRequestsProcessed	= ( msgSSLEvent + 1 ) ,
	msgRequestRemoved	= ( msgAllRequestsProcessed + 1 ) 
    } ;

enum tagReturnFlag
    {	rfUnknown	= 0,
	rfComing	= 1,
	rfReceiving	= 2,
	rfDecrypted	= 4,
	rfUnzipped	= 16,
	rfCompleted	= 32,
	rfDataComing	= 64
    } ;

enum tagBaseRequestID
    {	idUnknown	= 0,
	idSwitchTo	= 1,
	idPublicKeyFromSvr	= ( idSwitchTo + 1 ) ,
	idEncrypted	= ( idPublicKeyFromSvr + 1 ) ,
	idBatchZipped	= ( idEncrypted + 1 ) ,
	idCancel	= ( idBatchZipped + 1 ) ,
	idGetSockOptAtSvr	= ( idCancel + 1 ) ,
	idSetSockOptAtSvr	= ( idGetSockOptAtSvr + 1 ) ,
	idDoEcho	= ( idSetSockOptAtSvr + 1 ) ,
	idTurnOnZipAtSvr	= ( idDoEcho + 1 ) ,
	idStartBatching	= ( idTurnOnZipAtSvr + 1 ) ,
	idCommitBatching	= ( idStartBatching + 1 ) ,
	idShrinkMemoryAtSvr	= ( idCommitBatching + 1 ) ,
	idEchoFromSvr	= ( idShrinkMemoryAtSvr + 1 ) ,
	idPing	= ( idEchoFromSvr + 1 ) ,
	idSendingTooFast	= ( idPing + 1 ) ,
	idSendMySpecificBytes	= ( idSendingTooFast + 1 ) ,
	idCleanTrack	= ( idSendMySpecificBytes + 1 ) ,
	idVerifySalt	= ( idCleanTrack + 1 ) ,
	idSetZipLevelAtSvr	= ( idVerifySalt + 1 ) ,
	idStartJob	= ( idSetZipLevelAtSvr + 1 ) ,
	idEndJob	= ( idStartJob + 1 ) ,
	idDropRequestResult	= ( idEndJob + 1 ) ,
	idReservedOne	= 0x100,
	idReservedTwo	= 0x200
    } ;

enum tagHttpRequestID
    {	idHeader	= 46,
	idGet	= ( idHeader + 1 ) ,
	idPost	= ( idGet + 1 ) ,
	idHead	= ( idPost + 1 ) ,
	idPut	= ( idHead + 1 ) ,
	idDelete	= ( idPut + 1 ) ,
	idOptions	= ( idDelete + 1 ) ,
	idTrace	= ( idOptions + 1 ) ,
	idConnect	= ( idTrace + 1 ) ,
	idMultiPart	= ( idConnect + 1 ) 
    } ;

enum tagChatRequestID
    {	idXEnter	= 28,
	idXSpeak	= 29,
	idXSpeakEx	= 30,
	idEnter	= 31,
	idExit	= ( idEnter + 1 ) ,
	idGetAllGroups	= ( idExit + 1 ) ,
	idGetAllListeners	= ( idGetAllGroups + 1 ) ,
	idSpeak	= ( idGetAllListeners + 1 ) ,
	idSpeakTo	= ( idSpeak + 1 ) ,
	idGetAllClients	= ( idSpeakTo + 1 ) ,
	idSpeakEx	= ( idGetAllClients + 1 ) ,
	idSpeakToEx	= ( idSpeakEx + 1 ) ,
	idSendUserMessage	= ( idSpeakToEx + 1 ) ,
	idSendUserMessageEx	= ( idSendUserMessage + 1 ) 
    } ;

enum tagServiceID
    {	sidReserved1	= 1,
	sidStartup	= 0x100,
	sidChat	= ( sidStartup + 1 ) ,
	sidWinFile	= ( sidChat + 1 ) ,
	sidStdIO	= ( sidWinFile + 1 ) ,
	sidOleDB	= ( sidStdIO + 1 ) ,
	sidODBC	= ( sidOleDB + 1 ) ,
	sidADODotNet	= ( sidODBC + 1 ) ,
	sidOracleOCI	= ( sidADODotNet + 1 ) ,
	sidJDBC	= ( sidOracleOCI + 1 ) ,
	sidRemoteAdmin	= ( sidJDBC + 1 ) ,
	sidHTTP	= ( sidRemoteAdmin + 1 ) ,
	sidHTTPOnRealServer	= ( sidHTTP + 1 ) ,
	sidReserved2	= 0x200
    } ;

enum tagUErrorCode
    {	uecOK	= 0,
	uecSvrNotRegistered	= 0x1,
	uecUnexpected	= 0x8f000000,
	uecSvsNotAvailable	= 0x8f100001,
	uecAuthenticationFailed	= 0x8f100002,
	uecClientRejected	= 0x8f100003,
	uecFailedInLoadingSSL	= 0x8f100004,
	uecFailedInStartingSSL	= 0x8f100005,
	uecNotImplemented	= 0x8f100006,
	uecFail	= 0x8f100007,
	uecFailedInZip	= 0x8f100008,
	uecFailedInUnzip	= 0x8f100009,
	uecFailedInEncrypt	= 0x8f10000a,
	uecFailedInDecrypt	= 0x8f10000b,
	uecReadOnly	= 0x8f10000c,
	uecNoMoreData	= 0x8f10000d,
	uecExceptionCatched	= 0x8f10000e,
	uecRequestCanceled	= 0x8f10000f,
	uecNotFound	= 0x8f100010,
	uecFailedInLoadingMsSSPI	= 0x8f100011,
	uecBadData	= 0x8f100012,
	uecWrongServerVersion	= 0x8f100013
    } ;

enum tagSocketPoolErrorCode
    {	specUnexpected	= 0x8f200000,
	specWrongApartment	= 0x8f200001,
	specLockTimeOut	= 0x8f200002,
	specNotPooledSocket	= 0x8f200003,
	specNoOpenedSocket	= 0x8f200004,
	specNotUSocket	= 0x8f200005,
	specNoFreeSocket	= 0x8f200006,
	specTooManySockets	= 0x8f200007,
	specPoolNotStarted	= 0x8f200008,
	specSocketOpenedAlready	= 0x8f200009,
	specLockingInterrupted	= 0x8f20000a,
	specLockingAbandoned	= 0x8f20000b,
	specNotInPool	= 0x8f20000c,
	specNotValidService	= 0x8f20000d,
	specPoolNotCreatedYet	= 0x8f20000e
    } ;

enum tagOtherDefine
    {	odFastZip	= 0x40,
	odZipped	= 0x80,
	odUserRequestIDMin	= 0x2001,
	odUserRequestIDMax	= 0xffff,
	odUserServiceIDMin	= 0x10000000,
	odUserServiceIDMax	= 0xffffffff,
	odCancelAll	= 0xffffffff,
	odAllGroups	= 0xffffffff
    } ;

enum tagOperationSystem
    {	osUnknown	= 0,
	osWin9XMe	= 1,
	osWinNT	= 2,
	osWinCE	= 3,
	osLinux	= 64,
	osJava	= 128
    } ;
typedef 
enum tagZipLevel
    {	zlDefault	= 0,
	zlBestSpeed	= 1,
	zlBestCompression	= 2
    } 	ZipLevel;


enum tagAuthenticationMethod
    {	amOwn	= 0,
	amMixed	= ( amOwn + 1 ) ,
	amIntegrated	= ( amMixed + 1 ) ,
	amTrusted	= ( amIntegrated + 1 ) 
    } ;
typedef 
enum tagSocketPoolEvent
    {	speUnknown	= -1,
	speStarted	= ( speUnknown + 1 ) ,
	speCreatingThread	= ( speStarted + 1 ) ,
	speThreadCreated	= ( speCreatingThread + 1 ) ,
	speConnecting	= ( speThreadCreated + 1 ) ,
	speConnected	= ( speConnecting + 1 ) ,
	speKillingThread	= ( speConnected + 1 ) ,
	speShutdown	= ( speKillingThread + 1 ) ,
	speUSocketCreated	= ( speShutdown + 1 ) 
    } 	tagSocketPoolEvent;

#pragma pack(push,1)
struct CSwitchInfo
    {
    unsigned long m_ulSvsID;
    unsigned short m_usVerMajor;
    unsigned short m_usVerMinor;
    unsigned short m_usUSockMajor;
    unsigned short m_usUSockMinor;
    unsigned long m_ulParam1;
    unsigned long m_ulParam2;
    unsigned long m_ulParam3;
    unsigned long m_ulParam4;
    unsigned long m_ulParam5;
    } ;
#pragma pack(pop)
typedef 
enum tagDataFormat
    {	dfDefault	= 0,
	dfArray	= ( dfDefault + 1 ) ,
	dfDate	= ( dfArray + 1 ) 
    } 	DataFormat;


EXTERN_C const IID LIBID_USOCKETLib;

#ifndef __IJSSerialization_INTERFACE_DEFINED__
#define __IJSSerialization_INTERFACE_DEFINED__

/* interface IJSSerialization */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IJSSerialization;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F1F73888-E92E-411D-8AF0-34130A6958A1")
    IJSSerialization : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE save( 
            /* [in] */ VARIANT vtData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE saveBool( 
            /* [in] */ VARIANT_BOOL bData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE saveByte( 
            /* [in] */ BYTE bData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE saveInt16( 
            /* [in] */ short sData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE saveInt32( 
            /* [in] */ long nData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE saveFloat( 
            /* [in] */ float fData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE saveDouble( 
            /* [in] */ double dData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE saveString( 
            /* [in] */ BSTR bstrData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE saveGuid( 
            /* [in] */ BSTR bstrGuid) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE load( 
            /* [retval][out] */ VARIANT *pvtData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE loadBool( 
            /* [retval][out] */ VARIANT_BOOL *pbData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE loadByte( 
            /* [retval][out] */ BYTE *pbData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE loadInt16( 
            /* [retval][out] */ short *psData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE loadInt32( 
            /* [retval][out] */ long *pnData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE loadFloat( 
            /* [retval][out] */ float *pfData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE loadDouble( 
            /* [retval][out] */ double *pdData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE loadString( 
            /* [retval][out] */ BSTR *pbstrData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE loadGuid( 
            /* [retval][out] */ BSTR *pbstrGuid) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_size( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_size( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE saveDecimal( 
            /* [in] */ double dDecimal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE loadDecimal( 
            /* [retval][out] */ double *pdDecimal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE saveInt64( 
            /* [in] */ double dInt64) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE loadInt64( 
            /* [retval][out] */ double *pdInt64) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE saveDate( 
            /* [in] */ VARIANT vtDate) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE loadDate( 
            /* [retval][out] */ VARIANT *pvtDate) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_errorCode( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_errorMessage( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IJSSerializationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IJSSerialization * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IJSSerialization * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IJSSerialization * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IJSSerialization * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IJSSerialization * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IJSSerialization * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IJSSerialization * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *save )( 
            IJSSerialization * This,
            /* [in] */ VARIANT vtData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *saveBool )( 
            IJSSerialization * This,
            /* [in] */ VARIANT_BOOL bData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *saveByte )( 
            IJSSerialization * This,
            /* [in] */ BYTE bData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *saveInt16 )( 
            IJSSerialization * This,
            /* [in] */ short sData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *saveInt32 )( 
            IJSSerialization * This,
            /* [in] */ long nData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *saveFloat )( 
            IJSSerialization * This,
            /* [in] */ float fData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *saveDouble )( 
            IJSSerialization * This,
            /* [in] */ double dData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *saveString )( 
            IJSSerialization * This,
            /* [in] */ BSTR bstrData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *saveGuid )( 
            IJSSerialization * This,
            /* [in] */ BSTR bstrGuid);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *load )( 
            IJSSerialization * This,
            /* [retval][out] */ VARIANT *pvtData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *loadBool )( 
            IJSSerialization * This,
            /* [retval][out] */ VARIANT_BOOL *pbData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *loadByte )( 
            IJSSerialization * This,
            /* [retval][out] */ BYTE *pbData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *loadInt16 )( 
            IJSSerialization * This,
            /* [retval][out] */ short *psData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *loadInt32 )( 
            IJSSerialization * This,
            /* [retval][out] */ long *pnData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *loadFloat )( 
            IJSSerialization * This,
            /* [retval][out] */ float *pfData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *loadDouble )( 
            IJSSerialization * This,
            /* [retval][out] */ double *pdData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *loadString )( 
            IJSSerialization * This,
            /* [retval][out] */ BSTR *pbstrData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *loadGuid )( 
            IJSSerialization * This,
            /* [retval][out] */ BSTR *pbstrGuid);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_size )( 
            IJSSerialization * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_size )( 
            IJSSerialization * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *saveDecimal )( 
            IJSSerialization * This,
            /* [in] */ double dDecimal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *loadDecimal )( 
            IJSSerialization * This,
            /* [retval][out] */ double *pdDecimal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *saveInt64 )( 
            IJSSerialization * This,
            /* [in] */ double dInt64);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *loadInt64 )( 
            IJSSerialization * This,
            /* [retval][out] */ double *pdInt64);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *saveDate )( 
            IJSSerialization * This,
            /* [in] */ VARIANT vtDate);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *loadDate )( 
            IJSSerialization * This,
            /* [retval][out] */ VARIANT *pvtDate);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_errorCode )( 
            IJSSerialization * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_errorMessage )( 
            IJSSerialization * This,
            /* [retval][out] */ BSTR *pVal);
        
        END_INTERFACE
    } IJSSerializationVtbl;

    interface IJSSerialization
    {
        CONST_VTBL struct IJSSerializationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IJSSerialization_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IJSSerialization_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IJSSerialization_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IJSSerialization_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IJSSerialization_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IJSSerialization_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IJSSerialization_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IJSSerialization_save(This,vtData)	\
    ( (This)->lpVtbl -> save(This,vtData) ) 

#define IJSSerialization_saveBool(This,bData)	\
    ( (This)->lpVtbl -> saveBool(This,bData) ) 

#define IJSSerialization_saveByte(This,bData)	\
    ( (This)->lpVtbl -> saveByte(This,bData) ) 

#define IJSSerialization_saveInt16(This,sData)	\
    ( (This)->lpVtbl -> saveInt16(This,sData) ) 

#define IJSSerialization_saveInt32(This,nData)	\
    ( (This)->lpVtbl -> saveInt32(This,nData) ) 

#define IJSSerialization_saveFloat(This,fData)	\
    ( (This)->lpVtbl -> saveFloat(This,fData) ) 

#define IJSSerialization_saveDouble(This,dData)	\
    ( (This)->lpVtbl -> saveDouble(This,dData) ) 

#define IJSSerialization_saveString(This,bstrData)	\
    ( (This)->lpVtbl -> saveString(This,bstrData) ) 

#define IJSSerialization_saveGuid(This,bstrGuid)	\
    ( (This)->lpVtbl -> saveGuid(This,bstrGuid) ) 

#define IJSSerialization_load(This,pvtData)	\
    ( (This)->lpVtbl -> load(This,pvtData) ) 

#define IJSSerialization_loadBool(This,pbData)	\
    ( (This)->lpVtbl -> loadBool(This,pbData) ) 

#define IJSSerialization_loadByte(This,pbData)	\
    ( (This)->lpVtbl -> loadByte(This,pbData) ) 

#define IJSSerialization_loadInt16(This,psData)	\
    ( (This)->lpVtbl -> loadInt16(This,psData) ) 

#define IJSSerialization_loadInt32(This,pnData)	\
    ( (This)->lpVtbl -> loadInt32(This,pnData) ) 

#define IJSSerialization_loadFloat(This,pfData)	\
    ( (This)->lpVtbl -> loadFloat(This,pfData) ) 

#define IJSSerialization_loadDouble(This,pdData)	\
    ( (This)->lpVtbl -> loadDouble(This,pdData) ) 

#define IJSSerialization_loadString(This,pbstrData)	\
    ( (This)->lpVtbl -> loadString(This,pbstrData) ) 

#define IJSSerialization_loadGuid(This,pbstrGuid)	\
    ( (This)->lpVtbl -> loadGuid(This,pbstrGuid) ) 

#define IJSSerialization_get_size(This,pVal)	\
    ( (This)->lpVtbl -> get_size(This,pVal) ) 

#define IJSSerialization_put_size(This,newVal)	\
    ( (This)->lpVtbl -> put_size(This,newVal) ) 

#define IJSSerialization_saveDecimal(This,dDecimal)	\
    ( (This)->lpVtbl -> saveDecimal(This,dDecimal) ) 

#define IJSSerialization_loadDecimal(This,pdDecimal)	\
    ( (This)->lpVtbl -> loadDecimal(This,pdDecimal) ) 

#define IJSSerialization_saveInt64(This,dInt64)	\
    ( (This)->lpVtbl -> saveInt64(This,dInt64) ) 

#define IJSSerialization_loadInt64(This,pdInt64)	\
    ( (This)->lpVtbl -> loadInt64(This,pdInt64) ) 

#define IJSSerialization_saveDate(This,vtDate)	\
    ( (This)->lpVtbl -> saveDate(This,vtDate) ) 

#define IJSSerialization_loadDate(This,pvtDate)	\
    ( (This)->lpVtbl -> loadDate(This,pvtDate) ) 

#define IJSSerialization_get_errorCode(This,pVal)	\
    ( (This)->lpVtbl -> get_errorCode(This,pVal) ) 

#define IJSSerialization_get_errorMessage(This,pVal)	\
    ( (This)->lpVtbl -> get_errorMessage(This,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IJSSerialization_INTERFACE_DEFINED__ */


#ifndef __IUChat_INTERFACE_DEFINED__
#define __IUChat_INTERFACE_DEFINED__

/* interface IUChat */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IUChat;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2AFB8C2E-792C-10D6-B1D1-0010B5EC005B")
    IUChat : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Enter( 
            /* [in] */ long lGroups) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Exit( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Speak( 
            /* [in] */ VARIANT vtMsg,
            /* [defaultvalue][in] */ long lGroups = odAllGroups) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SpeakTo( 
            /* [in] */ BSTR bstrIPAddr,
            /* [in] */ long lPort,
            /* [in] */ VARIANT vtMsg) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetAllGroups( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetAllListeners( 
            /* [defaultvalue][in] */ long lGroups = odAllGroups) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetAllClients( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetGroupInfo( 
            /* [in] */ long lIndex,
            /* [out] */ long *plGroup,
            /* [retval][out] */ BSTR *pbstrDescription) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetInfo( 
            /* [in] */ long lIndex,
            /* [out] */ long *plGroup,
            /* [out] */ BSTR *pbstrUID,
            /* [out] */ long *plSvsID,
            /* [out] */ long *plPort,
            /* [retval][out] */ BSTR *pbstrIPAddress) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Listeners( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Message( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Groups( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SendUserMessage( 
            /* [in] */ BSTR bstrUserID,
            /* [in] */ VARIANT vtMsg) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE XEnter( 
            /* [in] */ VARIANT vtGroups) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE XSpeak( 
            /* [in] */ VARIANT vtMsg,
            /* [in] */ VARIANT vtGroups) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE XGetAllListeners( 
            /* [in] */ VARIANT vtGroups) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUChatVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUChat * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUChat * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUChat * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IUChat * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IUChat * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IUChat * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IUChat * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Enter )( 
            IUChat * This,
            /* [in] */ long lGroups);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Exit )( 
            IUChat * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Speak )( 
            IUChat * This,
            /* [in] */ VARIANT vtMsg,
            /* [defaultvalue][in] */ long lGroups);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SpeakTo )( 
            IUChat * This,
            /* [in] */ BSTR bstrIPAddr,
            /* [in] */ long lPort,
            /* [in] */ VARIANT vtMsg);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetAllGroups )( 
            IUChat * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetAllListeners )( 
            IUChat * This,
            /* [defaultvalue][in] */ long lGroups);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetAllClients )( 
            IUChat * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetGroupInfo )( 
            IUChat * This,
            /* [in] */ long lIndex,
            /* [out] */ long *plGroup,
            /* [retval][out] */ BSTR *pbstrDescription);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            IUChat * This,
            /* [in] */ long lIndex,
            /* [out] */ long *plGroup,
            /* [out] */ BSTR *pbstrUID,
            /* [out] */ long *plSvsID,
            /* [out] */ long *plPort,
            /* [retval][out] */ BSTR *pbstrIPAddress);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Listeners )( 
            IUChat * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Message )( 
            IUChat * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Groups )( 
            IUChat * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SendUserMessage )( 
            IUChat * This,
            /* [in] */ BSTR bstrUserID,
            /* [in] */ VARIANT vtMsg);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *XEnter )( 
            IUChat * This,
            /* [in] */ VARIANT vtGroups);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *XSpeak )( 
            IUChat * This,
            /* [in] */ VARIANT vtMsg,
            /* [in] */ VARIANT vtGroups);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *XGetAllListeners )( 
            IUChat * This,
            /* [in] */ VARIANT vtGroups);
        
        END_INTERFACE
    } IUChatVtbl;

    interface IUChat
    {
        CONST_VTBL struct IUChatVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUChat_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUChat_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUChat_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUChat_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IUChat_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IUChat_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IUChat_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IUChat_Enter(This,lGroups)	\
    ( (This)->lpVtbl -> Enter(This,lGroups) ) 

#define IUChat_Exit(This)	\
    ( (This)->lpVtbl -> Exit(This) ) 

#define IUChat_Speak(This,vtMsg,lGroups)	\
    ( (This)->lpVtbl -> Speak(This,vtMsg,lGroups) ) 

#define IUChat_SpeakTo(This,bstrIPAddr,lPort,vtMsg)	\
    ( (This)->lpVtbl -> SpeakTo(This,bstrIPAddr,lPort,vtMsg) ) 

#define IUChat_GetAllGroups(This)	\
    ( (This)->lpVtbl -> GetAllGroups(This) ) 

#define IUChat_GetAllListeners(This,lGroups)	\
    ( (This)->lpVtbl -> GetAllListeners(This,lGroups) ) 

#define IUChat_GetAllClients(This)	\
    ( (This)->lpVtbl -> GetAllClients(This) ) 

#define IUChat_GetGroupInfo(This,lIndex,plGroup,pbstrDescription)	\
    ( (This)->lpVtbl -> GetGroupInfo(This,lIndex,plGroup,pbstrDescription) ) 

#define IUChat_GetInfo(This,lIndex,plGroup,pbstrUID,plSvsID,plPort,pbstrIPAddress)	\
    ( (This)->lpVtbl -> GetInfo(This,lIndex,plGroup,pbstrUID,plSvsID,plPort,pbstrIPAddress) ) 

#define IUChat_get_Listeners(This,pVal)	\
    ( (This)->lpVtbl -> get_Listeners(This,pVal) ) 

#define IUChat_get_Message(This,pVal)	\
    ( (This)->lpVtbl -> get_Message(This,pVal) ) 

#define IUChat_get_Groups(This,pVal)	\
    ( (This)->lpVtbl -> get_Groups(This,pVal) ) 

#define IUChat_SendUserMessage(This,bstrUserID,vtMsg)	\
    ( (This)->lpVtbl -> SendUserMessage(This,bstrUserID,vtMsg) ) 

#define IUChat_XEnter(This,vtGroups)	\
    ( (This)->lpVtbl -> XEnter(This,vtGroups) ) 

#define IUChat_XSpeak(This,vtMsg,vtGroups)	\
    ( (This)->lpVtbl -> XSpeak(This,vtMsg,vtGroups) ) 

#define IUChat_XGetAllListeners(This,vtGroups)	\
    ( (This)->lpVtbl -> XGetAllListeners(This,vtGroups) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUChat_INTERFACE_DEFINED__ */


#ifndef __ISocketBase_INTERFACE_DEFINED__
#define __ISocketBase_INTERFACE_DEFINED__

/* interface ISocketBase */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ISocketBase;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1084FB17-4EDB-42F1-A4A8-129D70A0A8FA")
    ISocketBase : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Connect( 
            /* [in] */ BSTR bstrHostAddress,
            /* [in] */ long lHostPort,
            /* [defaultvalue][in] */ VARIANT_BOOL bSynConnecting = 0,
            /* [defaultvalue][in] */ long lSocketType = stSTREAM,
            /* [defaultvalue][in] */ long lAddrFormat = afINet,
            /* [defaultvalue][in] */ long lProtocol = spIP,
            /* [defaultvalue][in] */ long lLocalPort = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Disconnect( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Shutdown( 
            /* [defaultvalue][in] */ long lHow = hsSend) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetSockOpt( 
            /* [in] */ long lOptName,
            /* [in] */ long lOptValue,
            /* [defaultvalue][in] */ long lLevel = slSocket) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetSockOpt( 
            /* [in] */ long lOptName,
            /* [defaultvalue][in] */ long lLevel,
            /* [retval][out] */ long *plOptValue) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Send( 
            /* [in] */ VARIANT vtData,
            /* [retval][out] */ long *plSent) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Recv( 
            /* [retval][out] */ VARIANT *pvtData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IOCtl( 
            /* [defaultvalue][in] */ long lCommand,
            /* [retval][out] */ long *plArgment) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetInterfaceAttributes( 
            /* [out] */ long *lpMTU,
            /* [out] */ long *plMaxSpeed,
            /* [out] */ long *plIType,
            /* [out] */ BSTR *pbstrMask,
            /* [retval][out] */ BSTR *pbstrDesc) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetPeerName( 
            /* [out] */ long *plPeerPort,
            /* [retval][out] */ BSTR *pbstrPeerName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetLocalName( 
            /* [retval][out] */ BSTR *pbstrLocalName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetHostByName( 
            /* [in] */ BSTR bstrHost,
            /* [defaultvalue][in] */ VARIANT_BOOL bBlocking,
            /* [defaultvalue][out] */ long *phHandle,
            /* [defaultvalue][out] */ BSTR *pbstrIPAddr,
            /* [defaultvalue][out] */ BSTR *pbstrAlias,
            /* [retval][out] */ BSTR *pbstrHostName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetHostByAddr( 
            /* [in] */ BSTR bstrIPAddr,
            /* [defaultvalue][in] */ long lAddrFormat,
            /* [defaultvalue][in] */ VARIANT_BOOL bBlocking,
            /* [defaultvalue][out] */ long *phHandle,
            /* [defaultvalue][out] */ BSTR *pbstrAlias,
            /* [retval][out] */ BSTR *pbstrHostName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetProcAddress( 
            /* [in] */ BSTR bstrOpenSSLFuncName,
            /* [retval][out] */ long *plProcAddr) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetSockAddr( 
            /* [out] */ long *plPort,
            /* [retval][out] */ BSTR *pbstrSockAddr) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetBytesSent( 
            /* [defaultvalue][out] */ long *plHigh,
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetBytesReceived( 
            /* [defaultvalue][out] */ long *plHigh,
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ConnTimeout( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ConnTimeout( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Socket( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EncryptionMethod( 
            /* [retval][out] */ short *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EncryptionMethod( 
            /* [in] */ short newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SSLHandle( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PeerCertificate( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UseBaseSocketOnly( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UseBaseSocketOnly( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_hWnd( 
            /* [retval][out] */ long *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISocketBaseVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISocketBase * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISocketBase * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISocketBase * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISocketBase * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISocketBase * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISocketBase * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISocketBase * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Connect )( 
            ISocketBase * This,
            /* [in] */ BSTR bstrHostAddress,
            /* [in] */ long lHostPort,
            /* [defaultvalue][in] */ VARIANT_BOOL bSynConnecting,
            /* [defaultvalue][in] */ long lSocketType,
            /* [defaultvalue][in] */ long lAddrFormat,
            /* [defaultvalue][in] */ long lProtocol,
            /* [defaultvalue][in] */ long lLocalPort);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            ISocketBase * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Shutdown )( 
            ISocketBase * This,
            /* [defaultvalue][in] */ long lHow);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetSockOpt )( 
            ISocketBase * This,
            /* [in] */ long lOptName,
            /* [in] */ long lOptValue,
            /* [defaultvalue][in] */ long lLevel);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSockOpt )( 
            ISocketBase * This,
            /* [in] */ long lOptName,
            /* [defaultvalue][in] */ long lLevel,
            /* [retval][out] */ long *plOptValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Send )( 
            ISocketBase * This,
            /* [in] */ VARIANT vtData,
            /* [retval][out] */ long *plSent);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Recv )( 
            ISocketBase * This,
            /* [retval][out] */ VARIANT *pvtData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IOCtl )( 
            ISocketBase * This,
            /* [defaultvalue][in] */ long lCommand,
            /* [retval][out] */ long *plArgment);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetInterfaceAttributes )( 
            ISocketBase * This,
            /* [out] */ long *lpMTU,
            /* [out] */ long *plMaxSpeed,
            /* [out] */ long *plIType,
            /* [out] */ BSTR *pbstrMask,
            /* [retval][out] */ BSTR *pbstrDesc);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetPeerName )( 
            ISocketBase * This,
            /* [out] */ long *plPeerPort,
            /* [retval][out] */ BSTR *pbstrPeerName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetLocalName )( 
            ISocketBase * This,
            /* [retval][out] */ BSTR *pbstrLocalName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetHostByName )( 
            ISocketBase * This,
            /* [in] */ BSTR bstrHost,
            /* [defaultvalue][in] */ VARIANT_BOOL bBlocking,
            /* [defaultvalue][out] */ long *phHandle,
            /* [defaultvalue][out] */ BSTR *pbstrIPAddr,
            /* [defaultvalue][out] */ BSTR *pbstrAlias,
            /* [retval][out] */ BSTR *pbstrHostName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetHostByAddr )( 
            ISocketBase * This,
            /* [in] */ BSTR bstrIPAddr,
            /* [defaultvalue][in] */ long lAddrFormat,
            /* [defaultvalue][in] */ VARIANT_BOOL bBlocking,
            /* [defaultvalue][out] */ long *phHandle,
            /* [defaultvalue][out] */ BSTR *pbstrAlias,
            /* [retval][out] */ BSTR *pbstrHostName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetProcAddress )( 
            ISocketBase * This,
            /* [in] */ BSTR bstrOpenSSLFuncName,
            /* [retval][out] */ long *plProcAddr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSockAddr )( 
            ISocketBase * This,
            /* [out] */ long *plPort,
            /* [retval][out] */ BSTR *pbstrSockAddr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetBytesSent )( 
            ISocketBase * This,
            /* [defaultvalue][out] */ long *plHigh,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetBytesReceived )( 
            ISocketBase * This,
            /* [defaultvalue][out] */ long *plHigh,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ConnTimeout )( 
            ISocketBase * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ConnTimeout )( 
            ISocketBase * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Socket )( 
            ISocketBase * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EncryptionMethod )( 
            ISocketBase * This,
            /* [retval][out] */ short *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EncryptionMethod )( 
            ISocketBase * This,
            /* [in] */ short newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SSLHandle )( 
            ISocketBase * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PeerCertificate )( 
            ISocketBase * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UseBaseSocketOnly )( 
            ISocketBase * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UseBaseSocketOnly )( 
            ISocketBase * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_hWnd )( 
            ISocketBase * This,
            /* [retval][out] */ long *pVal);
        
        END_INTERFACE
    } ISocketBaseVtbl;

    interface ISocketBase
    {
        CONST_VTBL struct ISocketBaseVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISocketBase_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISocketBase_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISocketBase_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISocketBase_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ISocketBase_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ISocketBase_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ISocketBase_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ISocketBase_Connect(This,bstrHostAddress,lHostPort,bSynConnecting,lSocketType,lAddrFormat,lProtocol,lLocalPort)	\
    ( (This)->lpVtbl -> Connect(This,bstrHostAddress,lHostPort,bSynConnecting,lSocketType,lAddrFormat,lProtocol,lLocalPort) ) 

#define ISocketBase_Disconnect(This)	\
    ( (This)->lpVtbl -> Disconnect(This) ) 

#define ISocketBase_Shutdown(This,lHow)	\
    ( (This)->lpVtbl -> Shutdown(This,lHow) ) 

#define ISocketBase_SetSockOpt(This,lOptName,lOptValue,lLevel)	\
    ( (This)->lpVtbl -> SetSockOpt(This,lOptName,lOptValue,lLevel) ) 

#define ISocketBase_GetSockOpt(This,lOptName,lLevel,plOptValue)	\
    ( (This)->lpVtbl -> GetSockOpt(This,lOptName,lLevel,plOptValue) ) 

#define ISocketBase_Send(This,vtData,plSent)	\
    ( (This)->lpVtbl -> Send(This,vtData,plSent) ) 

#define ISocketBase_Recv(This,pvtData)	\
    ( (This)->lpVtbl -> Recv(This,pvtData) ) 

#define ISocketBase_IOCtl(This,lCommand,plArgment)	\
    ( (This)->lpVtbl -> IOCtl(This,lCommand,plArgment) ) 

#define ISocketBase_GetInterfaceAttributes(This,lpMTU,plMaxSpeed,plIType,pbstrMask,pbstrDesc)	\
    ( (This)->lpVtbl -> GetInterfaceAttributes(This,lpMTU,plMaxSpeed,plIType,pbstrMask,pbstrDesc) ) 

#define ISocketBase_GetPeerName(This,plPeerPort,pbstrPeerName)	\
    ( (This)->lpVtbl -> GetPeerName(This,plPeerPort,pbstrPeerName) ) 

#define ISocketBase_GetLocalName(This,pbstrLocalName)	\
    ( (This)->lpVtbl -> GetLocalName(This,pbstrLocalName) ) 

#define ISocketBase_GetHostByName(This,bstrHost,bBlocking,phHandle,pbstrIPAddr,pbstrAlias,pbstrHostName)	\
    ( (This)->lpVtbl -> GetHostByName(This,bstrHost,bBlocking,phHandle,pbstrIPAddr,pbstrAlias,pbstrHostName) ) 

#define ISocketBase_GetHostByAddr(This,bstrIPAddr,lAddrFormat,bBlocking,phHandle,pbstrAlias,pbstrHostName)	\
    ( (This)->lpVtbl -> GetHostByAddr(This,bstrIPAddr,lAddrFormat,bBlocking,phHandle,pbstrAlias,pbstrHostName) ) 

#define ISocketBase_GetProcAddress(This,bstrOpenSSLFuncName,plProcAddr)	\
    ( (This)->lpVtbl -> GetProcAddress(This,bstrOpenSSLFuncName,plProcAddr) ) 

#define ISocketBase_GetSockAddr(This,plPort,pbstrSockAddr)	\
    ( (This)->lpVtbl -> GetSockAddr(This,plPort,pbstrSockAddr) ) 

#define ISocketBase_GetBytesSent(This,plHigh,pVal)	\
    ( (This)->lpVtbl -> GetBytesSent(This,plHigh,pVal) ) 

#define ISocketBase_GetBytesReceived(This,plHigh,pVal)	\
    ( (This)->lpVtbl -> GetBytesReceived(This,plHigh,pVal) ) 

#define ISocketBase_get_ConnTimeout(This,pVal)	\
    ( (This)->lpVtbl -> get_ConnTimeout(This,pVal) ) 

#define ISocketBase_put_ConnTimeout(This,newVal)	\
    ( (This)->lpVtbl -> put_ConnTimeout(This,newVal) ) 

#define ISocketBase_get_Socket(This,pVal)	\
    ( (This)->lpVtbl -> get_Socket(This,pVal) ) 

#define ISocketBase_get_EncryptionMethod(This,pVal)	\
    ( (This)->lpVtbl -> get_EncryptionMethod(This,pVal) ) 

#define ISocketBase_put_EncryptionMethod(This,newVal)	\
    ( (This)->lpVtbl -> put_EncryptionMethod(This,newVal) ) 

#define ISocketBase_get_SSLHandle(This,pVal)	\
    ( (This)->lpVtbl -> get_SSLHandle(This,pVal) ) 

#define ISocketBase_get_PeerCertificate(This,pVal)	\
    ( (This)->lpVtbl -> get_PeerCertificate(This,pVal) ) 

#define ISocketBase_get_UseBaseSocketOnly(This,pVal)	\
    ( (This)->lpVtbl -> get_UseBaseSocketOnly(This,pVal) ) 

#define ISocketBase_put_UseBaseSocketOnly(This,newVal)	\
    ( (This)->lpVtbl -> put_UseBaseSocketOnly(This,newVal) ) 

#define ISocketBase_get_hWnd(This,pVal)	\
    ( (This)->lpVtbl -> get_hWnd(This,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISocketBase_INTERFACE_DEFINED__ */


#ifndef __IUSocket_INTERFACE_DEFINED__
#define __IUSocket_INTERFACE_DEFINED__

/* interface IUSocket */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IUSocket;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1B84FB17-40DB-42F1-A4A8-129D70A0A8FA")
    IUSocket : public ISocketBase
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SwitchTo( 
            /* [in] */ long lSvsID) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE StartBatching( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CommitBatching( 
            /* [defaultvalue][in] */ VARIANT_BOOL bSvrBatching = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AbortBatching( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE WaitAll( 
            /* [in] */ long lTimeOut,
            /* [retval][out] */ VARIANT_BOOL *pbTimeout) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DoEcho( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetRequestsInQueue( 
            /* [retval][out] */ VARIANT *pvtRequests) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SendRequest( 
            /* [in] */ short nRequestID,
            /* [in] */ VARIANT vtData,
            /* [defaultvalue][in] */ VARIANT_BOOL bKeepVTFormat = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetRtnBuffer( 
            /* [in] */ long nLen,
            /* [retval][out] */ VARIANT *pvtData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Cancel( 
            /* [defaultvalue][in] */ long lRequests = odCancelAll) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetSockOptAtSvr( 
            /* [in] */ long lOptName,
            /* [in] */ long lOptValue,
            /* [defaultvalue][in] */ long lLevel = slSocket) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetSockOptAtSvr( 
            /* [in] */ long lOptName,
            /* [defaultvalue][in] */ long lLevel = slSocket) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetOptValueAtSvr( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetClientVersion( 
            /* [in] */ short nMinor,
            /* [in] */ short nMajor) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE TurnOnZipAtSvr( 
            /* [defaultvalue][in] */ VARIANT_BOOL bZipOn = -1) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetClientVersion( 
            /* [out] */ short *pnMinor,
            /* [retval][out] */ short *pnMajor) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetServerVersion( 
            /* [out] */ short *pnMinor,
            /* [out] */ short *pnMajor) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ShrinkMemory( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ShrinkMemoryAtSvr( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetClientUSockVersion( 
            /* [out] */ short *pnMinor,
            /* [retval][out] */ short *pnMajor) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetServerUSockVersion( 
            /* [out] */ short *pnMinor,
            /* [retval][out] */ short *pnMajor) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DetectNetPerformance( 
            /* [retval][out] */ short *psNetPerformance) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IgnorePendingRequests( 
            /* [defaultvalue][in] */ long lRequests = -1,
            /* [defaultvalue][in] */ long lStartWith = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SendMySpecificBytes( 
            /* [in] */ VARIANT vtBytes) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ErrorMsg( 
            /* [retval][out] */ BSTR *pbstrErrorMsg) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Key( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Key( 
            /* [in] */ VARIANT newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ZipIsOn( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ZipIsOn( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UserID( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UserID( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Password( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Password( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RcvMemory( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SndMemory( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Syn( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Syn( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClientParams( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ClientParams( 
            /* [in] */ VARIANT newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ServerParams( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RecvTimeout( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_RecvTimeout( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SendTimeout( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SendTimeout( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_KeyFromSvr( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RequestsCanceled( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Frozen( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Frozen( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BytesInSndMemory( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BytesInRcvMemory( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ReturnEvents( 
            /* [retval][out] */ short *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ReturnEvents( 
            /* [in] */ short newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MaxRtns( 
            /* [retval][out] */ short *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MaxRtns( 
            /* [in] */ short newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RtnsInQueue( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CurrentSvsID( 
            /* [retval][out] */ long *plSvsID) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ZipOnAtSvr( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Rtn( 
            /* [retval][out] */ long *plRtn) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IsSameEndian( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BytesBatched( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Wait( 
            /* [in] */ short sRequestID,
            /* [in] */ long lTimeout,
            /* [defaultvalue][in] */ long lSvsID,
            /* [retval][out] */ VARIANT_BOOL *pbTimeout) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CountOfRequestsInQueue( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IsBatching( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CleanTrack( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LastRequestID( 
            /* [retval][out] */ short *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE StartJob( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EndJob( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUSocketVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUSocket * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUSocket * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUSocket * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IUSocket * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IUSocket * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IUSocket * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IUSocket * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Connect )( 
            IUSocket * This,
            /* [in] */ BSTR bstrHostAddress,
            /* [in] */ long lHostPort,
            /* [defaultvalue][in] */ VARIANT_BOOL bSynConnecting,
            /* [defaultvalue][in] */ long lSocketType,
            /* [defaultvalue][in] */ long lAddrFormat,
            /* [defaultvalue][in] */ long lProtocol,
            /* [defaultvalue][in] */ long lLocalPort);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            IUSocket * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Shutdown )( 
            IUSocket * This,
            /* [defaultvalue][in] */ long lHow);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetSockOpt )( 
            IUSocket * This,
            /* [in] */ long lOptName,
            /* [in] */ long lOptValue,
            /* [defaultvalue][in] */ long lLevel);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSockOpt )( 
            IUSocket * This,
            /* [in] */ long lOptName,
            /* [defaultvalue][in] */ long lLevel,
            /* [retval][out] */ long *plOptValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Send )( 
            IUSocket * This,
            /* [in] */ VARIANT vtData,
            /* [retval][out] */ long *plSent);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Recv )( 
            IUSocket * This,
            /* [retval][out] */ VARIANT *pvtData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IOCtl )( 
            IUSocket * This,
            /* [defaultvalue][in] */ long lCommand,
            /* [retval][out] */ long *plArgment);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetInterfaceAttributes )( 
            IUSocket * This,
            /* [out] */ long *lpMTU,
            /* [out] */ long *plMaxSpeed,
            /* [out] */ long *plIType,
            /* [out] */ BSTR *pbstrMask,
            /* [retval][out] */ BSTR *pbstrDesc);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetPeerName )( 
            IUSocket * This,
            /* [out] */ long *plPeerPort,
            /* [retval][out] */ BSTR *pbstrPeerName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetLocalName )( 
            IUSocket * This,
            /* [retval][out] */ BSTR *pbstrLocalName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetHostByName )( 
            IUSocket * This,
            /* [in] */ BSTR bstrHost,
            /* [defaultvalue][in] */ VARIANT_BOOL bBlocking,
            /* [defaultvalue][out] */ long *phHandle,
            /* [defaultvalue][out] */ BSTR *pbstrIPAddr,
            /* [defaultvalue][out] */ BSTR *pbstrAlias,
            /* [retval][out] */ BSTR *pbstrHostName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetHostByAddr )( 
            IUSocket * This,
            /* [in] */ BSTR bstrIPAddr,
            /* [defaultvalue][in] */ long lAddrFormat,
            /* [defaultvalue][in] */ VARIANT_BOOL bBlocking,
            /* [defaultvalue][out] */ long *phHandle,
            /* [defaultvalue][out] */ BSTR *pbstrAlias,
            /* [retval][out] */ BSTR *pbstrHostName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetProcAddress )( 
            IUSocket * This,
            /* [in] */ BSTR bstrOpenSSLFuncName,
            /* [retval][out] */ long *plProcAddr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSockAddr )( 
            IUSocket * This,
            /* [out] */ long *plPort,
            /* [retval][out] */ BSTR *pbstrSockAddr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetBytesSent )( 
            IUSocket * This,
            /* [defaultvalue][out] */ long *plHigh,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetBytesReceived )( 
            IUSocket * This,
            /* [defaultvalue][out] */ long *plHigh,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ConnTimeout )( 
            IUSocket * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ConnTimeout )( 
            IUSocket * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Socket )( 
            IUSocket * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EncryptionMethod )( 
            IUSocket * This,
            /* [retval][out] */ short *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EncryptionMethod )( 
            IUSocket * This,
            /* [in] */ short newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SSLHandle )( 
            IUSocket * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PeerCertificate )( 
            IUSocket * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UseBaseSocketOnly )( 
            IUSocket * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UseBaseSocketOnly )( 
            IUSocket * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_hWnd )( 
            IUSocket * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SwitchTo )( 
            IUSocket * This,
            /* [in] */ long lSvsID);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *StartBatching )( 
            IUSocket * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *CommitBatching )( 
            IUSocket * This,
            /* [defaultvalue][in] */ VARIANT_BOOL bSvrBatching);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *AbortBatching )( 
            IUSocket * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *WaitAll )( 
            IUSocket * This,
            /* [in] */ long lTimeOut,
            /* [retval][out] */ VARIANT_BOOL *pbTimeout);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DoEcho )( 
            IUSocket * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetRequestsInQueue )( 
            IUSocket * This,
            /* [retval][out] */ VARIANT *pvtRequests);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SendRequest )( 
            IUSocket * This,
            /* [in] */ short nRequestID,
            /* [in] */ VARIANT vtData,
            /* [defaultvalue][in] */ VARIANT_BOOL bKeepVTFormat);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetRtnBuffer )( 
            IUSocket * This,
            /* [in] */ long nLen,
            /* [retval][out] */ VARIANT *pvtData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            IUSocket * This,
            /* [defaultvalue][in] */ long lRequests);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetSockOptAtSvr )( 
            IUSocket * This,
            /* [in] */ long lOptName,
            /* [in] */ long lOptValue,
            /* [defaultvalue][in] */ long lLevel);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSockOptAtSvr )( 
            IUSocket * This,
            /* [in] */ long lOptName,
            /* [defaultvalue][in] */ long lLevel);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetOptValueAtSvr )( 
            IUSocket * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetClientVersion )( 
            IUSocket * This,
            /* [in] */ short nMinor,
            /* [in] */ short nMajor);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *TurnOnZipAtSvr )( 
            IUSocket * This,
            /* [defaultvalue][in] */ VARIANT_BOOL bZipOn);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetClientVersion )( 
            IUSocket * This,
            /* [out] */ short *pnMinor,
            /* [retval][out] */ short *pnMajor);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetServerVersion )( 
            IUSocket * This,
            /* [out] */ short *pnMinor,
            /* [out] */ short *pnMajor);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ShrinkMemory )( 
            IUSocket * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ShrinkMemoryAtSvr )( 
            IUSocket * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetClientUSockVersion )( 
            IUSocket * This,
            /* [out] */ short *pnMinor,
            /* [retval][out] */ short *pnMajor);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetServerUSockVersion )( 
            IUSocket * This,
            /* [out] */ short *pnMinor,
            /* [retval][out] */ short *pnMajor);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DetectNetPerformance )( 
            IUSocket * This,
            /* [retval][out] */ short *psNetPerformance);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IgnorePendingRequests )( 
            IUSocket * This,
            /* [defaultvalue][in] */ long lRequests,
            /* [defaultvalue][in] */ long lStartWith);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SendMySpecificBytes )( 
            IUSocket * This,
            /* [in] */ VARIANT vtBytes);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ErrorMsg )( 
            IUSocket * This,
            /* [retval][out] */ BSTR *pbstrErrorMsg);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Key )( 
            IUSocket * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Key )( 
            IUSocket * This,
            /* [in] */ VARIANT newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ZipIsOn )( 
            IUSocket * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ZipIsOn )( 
            IUSocket * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserID )( 
            IUSocket * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserID )( 
            IUSocket * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Password )( 
            IUSocket * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Password )( 
            IUSocket * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RcvMemory )( 
            IUSocket * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SndMemory )( 
            IUSocket * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Syn )( 
            IUSocket * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Syn )( 
            IUSocket * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ClientParams )( 
            IUSocket * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ClientParams )( 
            IUSocket * This,
            /* [in] */ VARIANT newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ServerParams )( 
            IUSocket * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RecvTimeout )( 
            IUSocket * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RecvTimeout )( 
            IUSocket * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SendTimeout )( 
            IUSocket * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SendTimeout )( 
            IUSocket * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_KeyFromSvr )( 
            IUSocket * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RequestsCanceled )( 
            IUSocket * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Frozen )( 
            IUSocket * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Frozen )( 
            IUSocket * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BytesInSndMemory )( 
            IUSocket * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BytesInRcvMemory )( 
            IUSocket * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReturnEvents )( 
            IUSocket * This,
            /* [retval][out] */ short *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReturnEvents )( 
            IUSocket * This,
            /* [in] */ short newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MaxRtns )( 
            IUSocket * This,
            /* [retval][out] */ short *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_MaxRtns )( 
            IUSocket * This,
            /* [in] */ short newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RtnsInQueue )( 
            IUSocket * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CurrentSvsID )( 
            IUSocket * This,
            /* [retval][out] */ long *plSvsID);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ZipOnAtSvr )( 
            IUSocket * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Rtn )( 
            IUSocket * This,
            /* [retval][out] */ long *plRtn);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IsSameEndian )( 
            IUSocket * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BytesBatched )( 
            IUSocket * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Wait )( 
            IUSocket * This,
            /* [in] */ short sRequestID,
            /* [in] */ long lTimeout,
            /* [defaultvalue][in] */ long lSvsID,
            /* [retval][out] */ VARIANT_BOOL *pbTimeout);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CountOfRequestsInQueue )( 
            IUSocket * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IsBatching )( 
            IUSocket * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *CleanTrack )( 
            IUSocket * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LastRequestID )( 
            IUSocket * This,
            /* [retval][out] */ short *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *StartJob )( 
            IUSocket * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *EndJob )( 
            IUSocket * This);
        
        END_INTERFACE
    } IUSocketVtbl;

    interface IUSocket
    {
        CONST_VTBL struct IUSocketVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUSocket_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUSocket_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUSocket_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUSocket_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IUSocket_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IUSocket_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IUSocket_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IUSocket_Connect(This,bstrHostAddress,lHostPort,bSynConnecting,lSocketType,lAddrFormat,lProtocol,lLocalPort)	\
    ( (This)->lpVtbl -> Connect(This,bstrHostAddress,lHostPort,bSynConnecting,lSocketType,lAddrFormat,lProtocol,lLocalPort) ) 

#define IUSocket_Disconnect(This)	\
    ( (This)->lpVtbl -> Disconnect(This) ) 

#define IUSocket_Shutdown(This,lHow)	\
    ( (This)->lpVtbl -> Shutdown(This,lHow) ) 

#define IUSocket_SetSockOpt(This,lOptName,lOptValue,lLevel)	\
    ( (This)->lpVtbl -> SetSockOpt(This,lOptName,lOptValue,lLevel) ) 

#define IUSocket_GetSockOpt(This,lOptName,lLevel,plOptValue)	\
    ( (This)->lpVtbl -> GetSockOpt(This,lOptName,lLevel,plOptValue) ) 

#define IUSocket_Send(This,vtData,plSent)	\
    ( (This)->lpVtbl -> Send(This,vtData,plSent) ) 

#define IUSocket_Recv(This,pvtData)	\
    ( (This)->lpVtbl -> Recv(This,pvtData) ) 

#define IUSocket_IOCtl(This,lCommand,plArgment)	\
    ( (This)->lpVtbl -> IOCtl(This,lCommand,plArgment) ) 

#define IUSocket_GetInterfaceAttributes(This,lpMTU,plMaxSpeed,plIType,pbstrMask,pbstrDesc)	\
    ( (This)->lpVtbl -> GetInterfaceAttributes(This,lpMTU,plMaxSpeed,plIType,pbstrMask,pbstrDesc) ) 

#define IUSocket_GetPeerName(This,plPeerPort,pbstrPeerName)	\
    ( (This)->lpVtbl -> GetPeerName(This,plPeerPort,pbstrPeerName) ) 

#define IUSocket_GetLocalName(This,pbstrLocalName)	\
    ( (This)->lpVtbl -> GetLocalName(This,pbstrLocalName) ) 

#define IUSocket_GetHostByName(This,bstrHost,bBlocking,phHandle,pbstrIPAddr,pbstrAlias,pbstrHostName)	\
    ( (This)->lpVtbl -> GetHostByName(This,bstrHost,bBlocking,phHandle,pbstrIPAddr,pbstrAlias,pbstrHostName) ) 

#define IUSocket_GetHostByAddr(This,bstrIPAddr,lAddrFormat,bBlocking,phHandle,pbstrAlias,pbstrHostName)	\
    ( (This)->lpVtbl -> GetHostByAddr(This,bstrIPAddr,lAddrFormat,bBlocking,phHandle,pbstrAlias,pbstrHostName) ) 

#define IUSocket_GetProcAddress(This,bstrOpenSSLFuncName,plProcAddr)	\
    ( (This)->lpVtbl -> GetProcAddress(This,bstrOpenSSLFuncName,plProcAddr) ) 

#define IUSocket_GetSockAddr(This,plPort,pbstrSockAddr)	\
    ( (This)->lpVtbl -> GetSockAddr(This,plPort,pbstrSockAddr) ) 

#define IUSocket_GetBytesSent(This,plHigh,pVal)	\
    ( (This)->lpVtbl -> GetBytesSent(This,plHigh,pVal) ) 

#define IUSocket_GetBytesReceived(This,plHigh,pVal)	\
    ( (This)->lpVtbl -> GetBytesReceived(This,plHigh,pVal) ) 

#define IUSocket_get_ConnTimeout(This,pVal)	\
    ( (This)->lpVtbl -> get_ConnTimeout(This,pVal) ) 

#define IUSocket_put_ConnTimeout(This,newVal)	\
    ( (This)->lpVtbl -> put_ConnTimeout(This,newVal) ) 

#define IUSocket_get_Socket(This,pVal)	\
    ( (This)->lpVtbl -> get_Socket(This,pVal) ) 

#define IUSocket_get_EncryptionMethod(This,pVal)	\
    ( (This)->lpVtbl -> get_EncryptionMethod(This,pVal) ) 

#define IUSocket_put_EncryptionMethod(This,newVal)	\
    ( (This)->lpVtbl -> put_EncryptionMethod(This,newVal) ) 

#define IUSocket_get_SSLHandle(This,pVal)	\
    ( (This)->lpVtbl -> get_SSLHandle(This,pVal) ) 

#define IUSocket_get_PeerCertificate(This,pVal)	\
    ( (This)->lpVtbl -> get_PeerCertificate(This,pVal) ) 

#define IUSocket_get_UseBaseSocketOnly(This,pVal)	\
    ( (This)->lpVtbl -> get_UseBaseSocketOnly(This,pVal) ) 

#define IUSocket_put_UseBaseSocketOnly(This,newVal)	\
    ( (This)->lpVtbl -> put_UseBaseSocketOnly(This,newVal) ) 

#define IUSocket_get_hWnd(This,pVal)	\
    ( (This)->lpVtbl -> get_hWnd(This,pVal) ) 


#define IUSocket_SwitchTo(This,lSvsID)	\
    ( (This)->lpVtbl -> SwitchTo(This,lSvsID) ) 

#define IUSocket_StartBatching(This)	\
    ( (This)->lpVtbl -> StartBatching(This) ) 

#define IUSocket_CommitBatching(This,bSvrBatching)	\
    ( (This)->lpVtbl -> CommitBatching(This,bSvrBatching) ) 

#define IUSocket_AbortBatching(This)	\
    ( (This)->lpVtbl -> AbortBatching(This) ) 

#define IUSocket_WaitAll(This,lTimeOut,pbTimeout)	\
    ( (This)->lpVtbl -> WaitAll(This,lTimeOut,pbTimeout) ) 

#define IUSocket_DoEcho(This)	\
    ( (This)->lpVtbl -> DoEcho(This) ) 

#define IUSocket_GetRequestsInQueue(This,pvtRequests)	\
    ( (This)->lpVtbl -> GetRequestsInQueue(This,pvtRequests) ) 

#define IUSocket_SendRequest(This,nRequestID,vtData,bKeepVTFormat)	\
    ( (This)->lpVtbl -> SendRequest(This,nRequestID,vtData,bKeepVTFormat) ) 

#define IUSocket_GetRtnBuffer(This,nLen,pvtData)	\
    ( (This)->lpVtbl -> GetRtnBuffer(This,nLen,pvtData) ) 

#define IUSocket_Cancel(This,lRequests)	\
    ( (This)->lpVtbl -> Cancel(This,lRequests) ) 

#define IUSocket_SetSockOptAtSvr(This,lOptName,lOptValue,lLevel)	\
    ( (This)->lpVtbl -> SetSockOptAtSvr(This,lOptName,lOptValue,lLevel) ) 

#define IUSocket_GetSockOptAtSvr(This,lOptName,lLevel)	\
    ( (This)->lpVtbl -> GetSockOptAtSvr(This,lOptName,lLevel) ) 

#define IUSocket_GetOptValueAtSvr(This,pVal)	\
    ( (This)->lpVtbl -> GetOptValueAtSvr(This,pVal) ) 

#define IUSocket_SetClientVersion(This,nMinor,nMajor)	\
    ( (This)->lpVtbl -> SetClientVersion(This,nMinor,nMajor) ) 

#define IUSocket_TurnOnZipAtSvr(This,bZipOn)	\
    ( (This)->lpVtbl -> TurnOnZipAtSvr(This,bZipOn) ) 

#define IUSocket_GetClientVersion(This,pnMinor,pnMajor)	\
    ( (This)->lpVtbl -> GetClientVersion(This,pnMinor,pnMajor) ) 

#define IUSocket_GetServerVersion(This,pnMinor,pnMajor)	\
    ( (This)->lpVtbl -> GetServerVersion(This,pnMinor,pnMajor) ) 

#define IUSocket_ShrinkMemory(This)	\
    ( (This)->lpVtbl -> ShrinkMemory(This) ) 

#define IUSocket_ShrinkMemoryAtSvr(This)	\
    ( (This)->lpVtbl -> ShrinkMemoryAtSvr(This) ) 

#define IUSocket_GetClientUSockVersion(This,pnMinor,pnMajor)	\
    ( (This)->lpVtbl -> GetClientUSockVersion(This,pnMinor,pnMajor) ) 

#define IUSocket_GetServerUSockVersion(This,pnMinor,pnMajor)	\
    ( (This)->lpVtbl -> GetServerUSockVersion(This,pnMinor,pnMajor) ) 

#define IUSocket_DetectNetPerformance(This,psNetPerformance)	\
    ( (This)->lpVtbl -> DetectNetPerformance(This,psNetPerformance) ) 

#define IUSocket_IgnorePendingRequests(This,lRequests,lStartWith)	\
    ( (This)->lpVtbl -> IgnorePendingRequests(This,lRequests,lStartWith) ) 

#define IUSocket_SendMySpecificBytes(This,vtBytes)	\
    ( (This)->lpVtbl -> SendMySpecificBytes(This,vtBytes) ) 

#define IUSocket_get_ErrorMsg(This,pbstrErrorMsg)	\
    ( (This)->lpVtbl -> get_ErrorMsg(This,pbstrErrorMsg) ) 

#define IUSocket_get_Key(This,pVal)	\
    ( (This)->lpVtbl -> get_Key(This,pVal) ) 

#define IUSocket_put_Key(This,newVal)	\
    ( (This)->lpVtbl -> put_Key(This,newVal) ) 

#define IUSocket_get_ZipIsOn(This,pVal)	\
    ( (This)->lpVtbl -> get_ZipIsOn(This,pVal) ) 

#define IUSocket_put_ZipIsOn(This,newVal)	\
    ( (This)->lpVtbl -> put_ZipIsOn(This,newVal) ) 

#define IUSocket_get_UserID(This,pVal)	\
    ( (This)->lpVtbl -> get_UserID(This,pVal) ) 

#define IUSocket_put_UserID(This,newVal)	\
    ( (This)->lpVtbl -> put_UserID(This,newVal) ) 

#define IUSocket_get_Password(This,pVal)	\
    ( (This)->lpVtbl -> get_Password(This,pVal) ) 

#define IUSocket_put_Password(This,newVal)	\
    ( (This)->lpVtbl -> put_Password(This,newVal) ) 

#define IUSocket_get_RcvMemory(This,pVal)	\
    ( (This)->lpVtbl -> get_RcvMemory(This,pVal) ) 

#define IUSocket_get_SndMemory(This,pVal)	\
    ( (This)->lpVtbl -> get_SndMemory(This,pVal) ) 

#define IUSocket_get_Syn(This,pVal)	\
    ( (This)->lpVtbl -> get_Syn(This,pVal) ) 

#define IUSocket_put_Syn(This,newVal)	\
    ( (This)->lpVtbl -> put_Syn(This,newVal) ) 

#define IUSocket_get_ClientParams(This,pVal)	\
    ( (This)->lpVtbl -> get_ClientParams(This,pVal) ) 

#define IUSocket_put_ClientParams(This,newVal)	\
    ( (This)->lpVtbl -> put_ClientParams(This,newVal) ) 

#define IUSocket_get_ServerParams(This,pVal)	\
    ( (This)->lpVtbl -> get_ServerParams(This,pVal) ) 

#define IUSocket_get_RecvTimeout(This,pVal)	\
    ( (This)->lpVtbl -> get_RecvTimeout(This,pVal) ) 

#define IUSocket_put_RecvTimeout(This,newVal)	\
    ( (This)->lpVtbl -> put_RecvTimeout(This,newVal) ) 

#define IUSocket_get_SendTimeout(This,pVal)	\
    ( (This)->lpVtbl -> get_SendTimeout(This,pVal) ) 

#define IUSocket_put_SendTimeout(This,newVal)	\
    ( (This)->lpVtbl -> put_SendTimeout(This,newVal) ) 

#define IUSocket_get_KeyFromSvr(This,pVal)	\
    ( (This)->lpVtbl -> get_KeyFromSvr(This,pVal) ) 

#define IUSocket_get_RequestsCanceled(This,pVal)	\
    ( (This)->lpVtbl -> get_RequestsCanceled(This,pVal) ) 

#define IUSocket_get_Frozen(This,pVal)	\
    ( (This)->lpVtbl -> get_Frozen(This,pVal) ) 

#define IUSocket_put_Frozen(This,newVal)	\
    ( (This)->lpVtbl -> put_Frozen(This,newVal) ) 

#define IUSocket_get_BytesInSndMemory(This,pVal)	\
    ( (This)->lpVtbl -> get_BytesInSndMemory(This,pVal) ) 

#define IUSocket_get_BytesInRcvMemory(This,pVal)	\
    ( (This)->lpVtbl -> get_BytesInRcvMemory(This,pVal) ) 

#define IUSocket_get_ReturnEvents(This,pVal)	\
    ( (This)->lpVtbl -> get_ReturnEvents(This,pVal) ) 

#define IUSocket_put_ReturnEvents(This,newVal)	\
    ( (This)->lpVtbl -> put_ReturnEvents(This,newVal) ) 

#define IUSocket_get_MaxRtns(This,pVal)	\
    ( (This)->lpVtbl -> get_MaxRtns(This,pVal) ) 

#define IUSocket_put_MaxRtns(This,newVal)	\
    ( (This)->lpVtbl -> put_MaxRtns(This,newVal) ) 

#define IUSocket_get_RtnsInQueue(This,pVal)	\
    ( (This)->lpVtbl -> get_RtnsInQueue(This,pVal) ) 

#define IUSocket_get_CurrentSvsID(This,plSvsID)	\
    ( (This)->lpVtbl -> get_CurrentSvsID(This,plSvsID) ) 

#define IUSocket_get_ZipOnAtSvr(This,pVal)	\
    ( (This)->lpVtbl -> get_ZipOnAtSvr(This,pVal) ) 

#define IUSocket_get_Rtn(This,plRtn)	\
    ( (This)->lpVtbl -> get_Rtn(This,plRtn) ) 

#define IUSocket_get_IsSameEndian(This,pVal)	\
    ( (This)->lpVtbl -> get_IsSameEndian(This,pVal) ) 

#define IUSocket_get_BytesBatched(This,pVal)	\
    ( (This)->lpVtbl -> get_BytesBatched(This,pVal) ) 

#define IUSocket_Wait(This,sRequestID,lTimeout,lSvsID,pbTimeout)	\
    ( (This)->lpVtbl -> Wait(This,sRequestID,lTimeout,lSvsID,pbTimeout) ) 

#define IUSocket_get_CountOfRequestsInQueue(This,pVal)	\
    ( (This)->lpVtbl -> get_CountOfRequestsInQueue(This,pVal) ) 

#define IUSocket_get_IsBatching(This,pVal)	\
    ( (This)->lpVtbl -> get_IsBatching(This,pVal) ) 

#define IUSocket_CleanTrack(This)	\
    ( (This)->lpVtbl -> CleanTrack(This) ) 

#define IUSocket_get_LastRequestID(This,pVal)	\
    ( (This)->lpVtbl -> get_LastRequestID(This,pVal) ) 

#define IUSocket_StartJob(This)	\
    ( (This)->lpVtbl -> StartJob(This) ) 

#define IUSocket_EndJob(This)	\
    ( (This)->lpVtbl -> EndJob(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUSocket_INTERFACE_DEFINED__ */


#ifndef __IUFast_INTERFACE_DEFINED__
#define __IUFast_INTERFACE_DEFINED__

/* interface IUFast */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IUFast;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2AFB8C2A-792C-15D6-B1D1-1010B5EC1C5B")
    IUFast : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetRtnBufferEx( 
            /* [in] */ unsigned long ulLen,
            /* [size_is][in] */ BYTE pBuffer[  ],
            /* [retval][out] */ unsigned long *plRtnSize) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SendRequestEx( 
            /* [in] */ unsigned short usRequestID,
            /* [in] */ unsigned long ulLen,
            /* [size_is][in] */ BYTE pBuffer[  ]) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SpeakEx( 
            /* [in] */ unsigned long ulLen,
            /* [size_is][in] */ BYTE pMessage[  ],
            /* [in] */ unsigned long ulGroups) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SpeakToEx( 
            /* [in] */ BSTR bstrIPAddr,
            /* [in] */ long lPort,
            /* [in] */ unsigned long ulLen,
            /* [size_is][in] */ BYTE pMessage[  ]) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SendUserMessageEx( 
            /* [in] */ BSTR bstrUserID,
            /* [in] */ unsigned long ulLen,
            /* [size_is][in] */ BYTE pMessage[  ]) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE XSpeakEx( 
            /* [in] */ unsigned long ulLen,
            /* [size_is][in] */ BYTE pMessage[  ],
            /* [in] */ unsigned long ulGroupCount,
            /* [size_is][in] */ unsigned long pGroups[  ]) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUFastVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUFast * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUFast * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUFast * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetRtnBufferEx )( 
            IUFast * This,
            /* [in] */ unsigned long ulLen,
            /* [size_is][in] */ BYTE pBuffer[  ],
            /* [retval][out] */ unsigned long *plRtnSize);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SendRequestEx )( 
            IUFast * This,
            /* [in] */ unsigned short usRequestID,
            /* [in] */ unsigned long ulLen,
            /* [size_is][in] */ BYTE pBuffer[  ]);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SpeakEx )( 
            IUFast * This,
            /* [in] */ unsigned long ulLen,
            /* [size_is][in] */ BYTE pMessage[  ],
            /* [in] */ unsigned long ulGroups);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SpeakToEx )( 
            IUFast * This,
            /* [in] */ BSTR bstrIPAddr,
            /* [in] */ long lPort,
            /* [in] */ unsigned long ulLen,
            /* [size_is][in] */ BYTE pMessage[  ]);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SendUserMessageEx )( 
            IUFast * This,
            /* [in] */ BSTR bstrUserID,
            /* [in] */ unsigned long ulLen,
            /* [size_is][in] */ BYTE pMessage[  ]);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *XSpeakEx )( 
            IUFast * This,
            /* [in] */ unsigned long ulLen,
            /* [size_is][in] */ BYTE pMessage[  ],
            /* [in] */ unsigned long ulGroupCount,
            /* [size_is][in] */ unsigned long pGroups[  ]);
        
        END_INTERFACE
    } IUFastVtbl;

    interface IUFast
    {
        CONST_VTBL struct IUFastVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUFast_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUFast_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUFast_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUFast_GetRtnBufferEx(This,ulLen,pBuffer,plRtnSize)	\
    ( (This)->lpVtbl -> GetRtnBufferEx(This,ulLen,pBuffer,plRtnSize) ) 

#define IUFast_SendRequestEx(This,usRequestID,ulLen,pBuffer)	\
    ( (This)->lpVtbl -> SendRequestEx(This,usRequestID,ulLen,pBuffer) ) 

#define IUFast_SpeakEx(This,ulLen,pMessage,ulGroups)	\
    ( (This)->lpVtbl -> SpeakEx(This,ulLen,pMessage,ulGroups) ) 

#define IUFast_SpeakToEx(This,bstrIPAddr,lPort,ulLen,pMessage)	\
    ( (This)->lpVtbl -> SpeakToEx(This,bstrIPAddr,lPort,ulLen,pMessage) ) 

#define IUFast_SendUserMessageEx(This,bstrUserID,ulLen,pMessage)	\
    ( (This)->lpVtbl -> SendUserMessageEx(This,bstrUserID,ulLen,pMessage) ) 

#define IUFast_XSpeakEx(This,ulLen,pMessage,ulGroupCount,pGroups)	\
    ( (This)->lpVtbl -> XSpeakEx(This,ulLen,pMessage,ulGroupCount,pGroups) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUFast_INTERFACE_DEFINED__ */


#ifndef __IUZip_INTERFACE_DEFINED__
#define __IUZip_INTERFACE_DEFINED__

/* interface IUZip */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IUZip;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2A0B8C2E-702C-1AD6-B1D1-2010B5EC0158")
    IUZip : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetZipLevelAtSvr( 
            /* [in] */ ZipLevel zipLevel) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ZipLevel( 
            /* [retval][out] */ ZipLevel *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ZipLevel( 
            /* [in] */ ZipLevel newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ZipLevelAtSvr( 
            /* [retval][out] */ ZipLevel *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUZipVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUZip * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUZip * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUZip * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IUZip * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IUZip * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IUZip * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IUZip * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetZipLevelAtSvr )( 
            IUZip * This,
            /* [in] */ ZipLevel zipLevel);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ZipLevel )( 
            IUZip * This,
            /* [retval][out] */ ZipLevel *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ZipLevel )( 
            IUZip * This,
            /* [in] */ ZipLevel newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ZipLevelAtSvr )( 
            IUZip * This,
            /* [retval][out] */ ZipLevel *pVal);
        
        END_INTERFACE
    } IUZipVtbl;

    interface IUZip
    {
        CONST_VTBL struct IUZipVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUZip_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUZip_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUZip_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUZip_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IUZip_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IUZip_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IUZip_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IUZip_SetZipLevelAtSvr(This,zipLevel)	\
    ( (This)->lpVtbl -> SetZipLevelAtSvr(This,zipLevel) ) 

#define IUZip_get_ZipLevel(This,pVal)	\
    ( (This)->lpVtbl -> get_ZipLevel(This,pVal) ) 

#define IUZip_put_ZipLevel(This,newVal)	\
    ( (This)->lpVtbl -> put_ZipLevel(This,newVal) ) 

#define IUZip_get_ZipLevelAtSvr(This,pVal)	\
    ( (This)->lpVtbl -> get_ZipLevelAtSvr(This,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUZip_INTERFACE_DEFINED__ */


#ifndef __IUCert_INTERFACE_DEFINED__
#define __IUCert_INTERFACE_DEFINED__

/* interface IUCert */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IUCert;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2FFB8C2E-702C-1AD6-B1D1-0010B5EC015B")
    IUCert : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CompareDomain( 
            /* [out] */ BSTR *pbstrDomain,
            /* [defaultvalue][out] */ BSTR *pbstrHost,
            /* [retval][out] */ VARIANT_BOOL *pbMatchable) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Verify( 
            /* [out] */ long *plErrorCode,
            /* [retval][out] */ BSTR *pbstrErrorMsg) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Version( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Issuer( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Subject( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Validity( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NotBefore( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NotAfter( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SigAlg( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SerialNumber( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Algorithm( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PublicKey( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IssuerUID( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SubjectUID( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CertPem( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VerifyLocation( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VerifyLocation( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SessionInfo( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddCertificateToStore( 
            /* [defaultvalue][in] */ VARIANT_BOOL bRoot = 0) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUCertVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUCert * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUCert * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUCert * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IUCert * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IUCert * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IUCert * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IUCert * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *CompareDomain )( 
            IUCert * This,
            /* [out] */ BSTR *pbstrDomain,
            /* [defaultvalue][out] */ BSTR *pbstrHost,
            /* [retval][out] */ VARIANT_BOOL *pbMatchable);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Verify )( 
            IUCert * This,
            /* [out] */ long *plErrorCode,
            /* [retval][out] */ BSTR *pbstrErrorMsg);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Version )( 
            IUCert * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Issuer )( 
            IUCert * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Subject )( 
            IUCert * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Validity )( 
            IUCert * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NotBefore )( 
            IUCert * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NotAfter )( 
            IUCert * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SigAlg )( 
            IUCert * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SerialNumber )( 
            IUCert * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Algorithm )( 
            IUCert * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PublicKey )( 
            IUCert * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IssuerUID )( 
            IUCert * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SubjectUID )( 
            IUCert * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CertPem )( 
            IUCert * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VerifyLocation )( 
            IUCert * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VerifyLocation )( 
            IUCert * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SessionInfo )( 
            IUCert * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *AddCertificateToStore )( 
            IUCert * This,
            /* [defaultvalue][in] */ VARIANT_BOOL bRoot);
        
        END_INTERFACE
    } IUCertVtbl;

    interface IUCert
    {
        CONST_VTBL struct IUCertVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUCert_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUCert_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUCert_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUCert_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IUCert_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IUCert_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IUCert_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IUCert_CompareDomain(This,pbstrDomain,pbstrHost,pbMatchable)	\
    ( (This)->lpVtbl -> CompareDomain(This,pbstrDomain,pbstrHost,pbMatchable) ) 

#define IUCert_Verify(This,plErrorCode,pbstrErrorMsg)	\
    ( (This)->lpVtbl -> Verify(This,plErrorCode,pbstrErrorMsg) ) 

#define IUCert_get_Version(This,pVal)	\
    ( (This)->lpVtbl -> get_Version(This,pVal) ) 

#define IUCert_get_Issuer(This,pVal)	\
    ( (This)->lpVtbl -> get_Issuer(This,pVal) ) 

#define IUCert_get_Subject(This,pVal)	\
    ( (This)->lpVtbl -> get_Subject(This,pVal) ) 

#define IUCert_get_Validity(This,pVal)	\
    ( (This)->lpVtbl -> get_Validity(This,pVal) ) 

#define IUCert_get_NotBefore(This,pVal)	\
    ( (This)->lpVtbl -> get_NotBefore(This,pVal) ) 

#define IUCert_get_NotAfter(This,pVal)	\
    ( (This)->lpVtbl -> get_NotAfter(This,pVal) ) 

#define IUCert_get_SigAlg(This,pVal)	\
    ( (This)->lpVtbl -> get_SigAlg(This,pVal) ) 

#define IUCert_get_SerialNumber(This,pVal)	\
    ( (This)->lpVtbl -> get_SerialNumber(This,pVal) ) 

#define IUCert_get_Algorithm(This,pVal)	\
    ( (This)->lpVtbl -> get_Algorithm(This,pVal) ) 

#define IUCert_get_PublicKey(This,pVal)	\
    ( (This)->lpVtbl -> get_PublicKey(This,pVal) ) 

#define IUCert_get_IssuerUID(This,pVal)	\
    ( (This)->lpVtbl -> get_IssuerUID(This,pVal) ) 

#define IUCert_get_SubjectUID(This,pVal)	\
    ( (This)->lpVtbl -> get_SubjectUID(This,pVal) ) 

#define IUCert_get_CertPem(This,pVal)	\
    ( (This)->lpVtbl -> get_CertPem(This,pVal) ) 

#define IUCert_get_VerifyLocation(This,pVal)	\
    ( (This)->lpVtbl -> get_VerifyLocation(This,pVal) ) 

#define IUCert_put_VerifyLocation(This,newVal)	\
    ( (This)->lpVtbl -> put_VerifyLocation(This,newVal) ) 

#define IUCert_get_SessionInfo(This,pVal)	\
    ( (This)->lpVtbl -> get_SessionInfo(This,pVal) ) 

#define IUCert_AddCertificateToStore(This,bRoot)	\
    ( (This)->lpVtbl -> AddCertificateToStore(This,bRoot) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUCert_INTERFACE_DEFINED__ */


#ifndef ___IUSocketEvent_DISPINTERFACE_DEFINED__
#define ___IUSocketEvent_DISPINTERFACE_DEFINED__

/* dispinterface _IUSocketEvent */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__IUSocketEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("F80A0F37-147E-4C43-B889-1D4C9A612BB9")
    _IUSocketEvent : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IUSocketEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _IUSocketEvent * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _IUSocketEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _IUSocketEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _IUSocketEvent * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _IUSocketEvent * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _IUSocketEvent * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _IUSocketEvent * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _IUSocketEventVtbl;

    interface _IUSocketEvent
    {
        CONST_VTBL struct _IUSocketEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IUSocketEvent_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define _IUSocketEvent_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define _IUSocketEvent_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define _IUSocketEvent_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define _IUSocketEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define _IUSocketEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define _IUSocketEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IUSocketEvent_DISPINTERFACE_DEFINED__ */


#ifndef __IUSocketPool_INTERFACE_DEFINED__
#define __IUSocketPool_INTERFACE_DEFINED__

/* interface IUSocketPool */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IUSocketPool;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("148F6E9F-756D-4B8E-96F5-7AE8CC4697E6")
    IUSocketPool : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE StartPool( 
            /* [in] */ BYTE bThreadCount,
            /* [in] */ BYTE bSocketsPerThread) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ShutdownPool( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE LockASocket( 
            /* [in] */ long lTimeout,
            /* [in] */ IUSocket *pIUSocketSameThreadWith,
            /* [retval][out] */ IUSocket **ppIUSocket) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE UnlockASocket( 
            /* [in] */ IUSocket *pIUSocket) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ConnectAll( 
            /* [in] */ BSTR bstrHost,
            /* [in] */ long lPort,
            /* [in] */ long lInitialSvsID,
            /* [in] */ BSTR bstrUID,
            /* [in] */ BSTR bstrPWD,
            /* [in] */ short EncryptionMethod,
            /* [defaultvalue][in] */ VARIANT_BOOL bEnableZip = -1) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DisconnectAll( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DisconnectedSockets( 
            /* [retval][out] */ BYTE *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ConnectedSockets( 
            /* [retval][out] */ BYTE *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IdleSockets( 
            /* [retval][out] */ BYTE *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LockedSockets( 
            /* [retval][out] */ BYTE *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ThreadCounts( 
            /* [retval][out] */ BYTE *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FindAClosedSocket( 
            /* [retval][out] */ IUSocket **ppIUSocket) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Connect( 
            /* [in] */ IUSocket *pIUSocket,
            /* [in] */ BSTR bstrHost,
            /* [in] */ long lPort,
            /* [in] */ long lInitialSvsID,
            /* [in] */ BSTR bstrUID,
            /* [in] */ BSTR bstrPWD,
            /* [in] */ short EncryptionMethod,
            /* [defaultvalue][in] */ VARIANT_BOOL bEnableZip = -1) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ConnectedSocketsEx( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DisconnectedSocketsEx( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IdleSocketsEx( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LockedSocketsEx( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddOneThreadIntoPool( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_USockets( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUSocketPoolVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUSocketPool * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUSocketPool * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUSocketPool * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IUSocketPool * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IUSocketPool * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IUSocketPool * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IUSocketPool * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *StartPool )( 
            IUSocketPool * This,
            /* [in] */ BYTE bThreadCount,
            /* [in] */ BYTE bSocketsPerThread);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ShutdownPool )( 
            IUSocketPool * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *LockASocket )( 
            IUSocketPool * This,
            /* [in] */ long lTimeout,
            /* [in] */ IUSocket *pIUSocketSameThreadWith,
            /* [retval][out] */ IUSocket **ppIUSocket);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *UnlockASocket )( 
            IUSocketPool * This,
            /* [in] */ IUSocket *pIUSocket);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ConnectAll )( 
            IUSocketPool * This,
            /* [in] */ BSTR bstrHost,
            /* [in] */ long lPort,
            /* [in] */ long lInitialSvsID,
            /* [in] */ BSTR bstrUID,
            /* [in] */ BSTR bstrPWD,
            /* [in] */ short EncryptionMethod,
            /* [defaultvalue][in] */ VARIANT_BOOL bEnableZip);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DisconnectAll )( 
            IUSocketPool * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DisconnectedSockets )( 
            IUSocketPool * This,
            /* [retval][out] */ BYTE *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ConnectedSockets )( 
            IUSocketPool * This,
            /* [retval][out] */ BYTE *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IdleSockets )( 
            IUSocketPool * This,
            /* [retval][out] */ BYTE *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LockedSockets )( 
            IUSocketPool * This,
            /* [retval][out] */ BYTE *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ThreadCounts )( 
            IUSocketPool * This,
            /* [retval][out] */ BYTE *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *FindAClosedSocket )( 
            IUSocketPool * This,
            /* [retval][out] */ IUSocket **ppIUSocket);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Connect )( 
            IUSocketPool * This,
            /* [in] */ IUSocket *pIUSocket,
            /* [in] */ BSTR bstrHost,
            /* [in] */ long lPort,
            /* [in] */ long lInitialSvsID,
            /* [in] */ BSTR bstrUID,
            /* [in] */ BSTR bstrPWD,
            /* [in] */ short EncryptionMethod,
            /* [defaultvalue][in] */ VARIANT_BOOL bEnableZip);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ConnectedSocketsEx )( 
            IUSocketPool * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DisconnectedSocketsEx )( 
            IUSocketPool * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IdleSocketsEx )( 
            IUSocketPool * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LockedSocketsEx )( 
            IUSocketPool * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *AddOneThreadIntoPool )( 
            IUSocketPool * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_USockets )( 
            IUSocketPool * This,
            /* [retval][out] */ VARIANT *pVal);
        
        END_INTERFACE
    } IUSocketPoolVtbl;

    interface IUSocketPool
    {
        CONST_VTBL struct IUSocketPoolVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUSocketPool_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUSocketPool_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUSocketPool_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUSocketPool_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IUSocketPool_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IUSocketPool_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IUSocketPool_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IUSocketPool_StartPool(This,bThreadCount,bSocketsPerThread)	\
    ( (This)->lpVtbl -> StartPool(This,bThreadCount,bSocketsPerThread) ) 

#define IUSocketPool_ShutdownPool(This)	\
    ( (This)->lpVtbl -> ShutdownPool(This) ) 

#define IUSocketPool_LockASocket(This,lTimeout,pIUSocketSameThreadWith,ppIUSocket)	\
    ( (This)->lpVtbl -> LockASocket(This,lTimeout,pIUSocketSameThreadWith,ppIUSocket) ) 

#define IUSocketPool_UnlockASocket(This,pIUSocket)	\
    ( (This)->lpVtbl -> UnlockASocket(This,pIUSocket) ) 

#define IUSocketPool_ConnectAll(This,bstrHost,lPort,lInitialSvsID,bstrUID,bstrPWD,EncryptionMethod,bEnableZip)	\
    ( (This)->lpVtbl -> ConnectAll(This,bstrHost,lPort,lInitialSvsID,bstrUID,bstrPWD,EncryptionMethod,bEnableZip) ) 

#define IUSocketPool_DisconnectAll(This)	\
    ( (This)->lpVtbl -> DisconnectAll(This) ) 

#define IUSocketPool_get_DisconnectedSockets(This,pVal)	\
    ( (This)->lpVtbl -> get_DisconnectedSockets(This,pVal) ) 

#define IUSocketPool_get_ConnectedSockets(This,pVal)	\
    ( (This)->lpVtbl -> get_ConnectedSockets(This,pVal) ) 

#define IUSocketPool_get_IdleSockets(This,pVal)	\
    ( (This)->lpVtbl -> get_IdleSockets(This,pVal) ) 

#define IUSocketPool_get_LockedSockets(This,pVal)	\
    ( (This)->lpVtbl -> get_LockedSockets(This,pVal) ) 

#define IUSocketPool_get_ThreadCounts(This,pVal)	\
    ( (This)->lpVtbl -> get_ThreadCounts(This,pVal) ) 

#define IUSocketPool_FindAClosedSocket(This,ppIUSocket)	\
    ( (This)->lpVtbl -> FindAClosedSocket(This,ppIUSocket) ) 

#define IUSocketPool_Connect(This,pIUSocket,bstrHost,lPort,lInitialSvsID,bstrUID,bstrPWD,EncryptionMethod,bEnableZip)	\
    ( (This)->lpVtbl -> Connect(This,pIUSocket,bstrHost,lPort,lInitialSvsID,bstrUID,bstrPWD,EncryptionMethod,bEnableZip) ) 

#define IUSocketPool_get_ConnectedSocketsEx(This,pVal)	\
    ( (This)->lpVtbl -> get_ConnectedSocketsEx(This,pVal) ) 

#define IUSocketPool_get_DisconnectedSocketsEx(This,pVal)	\
    ( (This)->lpVtbl -> get_DisconnectedSocketsEx(This,pVal) ) 

#define IUSocketPool_get_IdleSocketsEx(This,pVal)	\
    ( (This)->lpVtbl -> get_IdleSocketsEx(This,pVal) ) 

#define IUSocketPool_get_LockedSocketsEx(This,pVal)	\
    ( (This)->lpVtbl -> get_LockedSocketsEx(This,pVal) ) 

#define IUSocketPool_AddOneThreadIntoPool(This)	\
    ( (This)->lpVtbl -> AddOneThreadIntoPool(This) ) 

#define IUSocketPool_get_USockets(This,pVal)	\
    ( (This)->lpVtbl -> get_USockets(This,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUSocketPool_INTERFACE_DEFINED__ */


#ifndef __IUSocketScript_INTERFACE_DEFINED__
#define __IUSocketScript_INTERFACE_DEFINED__

/* interface IUSocketScript */
/* [object][hidden][dual][uuid] */ 


EXTERN_C const IID IID_IUSocketScript;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("140F6E9F-756D-4B8E-96F5-7AE80C4697E6")
    IUSocketScript : public IUSocket
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Enter( 
            /* [in] */ long lGroups) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Exit( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Speak( 
            /* [in] */ VARIANT vtMsg,
            /* [defaultvalue][in] */ long lGroups = odAllGroups) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SpeakTo( 
            /* [in] */ BSTR bstrIPAddr,
            /* [in] */ long lPort,
            /* [in] */ VARIANT vtMsg) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetAllGroups( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetAllListeners( 
            /* [defaultvalue][in] */ long lGroups = odAllGroups) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetAllClients( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Listeners( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Message( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Groups( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SendUserMessage( 
            /* [in] */ BSTR bstrUserID,
            /* [in] */ VARIANT vtMsg) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_JSUQueue( 
            /* [retval][out] */ IJSSerialization **ppVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SendJSRequest( 
            /* [in] */ short sRequestId) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_JSObject( 
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_JSObject( 
            /* [in] */ VARIANT newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetZipLevelAtSvr( 
            /* [in] */ ZipLevel zipLevel) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ZipLevel( 
            /* [retval][out] */ ZipLevel *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ZipLevel( 
            /* [in] */ ZipLevel newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ZipLevelAtSvr( 
            /* [retval][out] */ ZipLevel *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetGroupID( 
            /* [in] */ long lIndex,
            /* [retval][out] */ long *plGroupID) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetGroupDescription( 
            /* [in] */ long lIndex,
            /* [retval][out] */ BSTR *pbstrDescription) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetInfoGroupIDs( 
            /* [in] */ long lIndex,
            /* [retval][out] */ long *plGroupIDs) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetInfoUserID( 
            /* [in] */ long lIndex,
            /* [retval][out] */ BSTR *pbstrUserID) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetInfoServiceID( 
            /* [in] */ long lIndex,
            /* [retval][out] */ long *plSvsID) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetInfoPort( 
            /* [in] */ long lIndex,
            /* [retval][out] */ long *plPort) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetInfoIPAddress( 
            /* [in] */ long lIndex,
            /* [retval][out] */ BSTR *pbstrIPAddress) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUSocketScriptVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUSocketScript * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUSocketScript * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUSocketScript * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IUSocketScript * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IUSocketScript * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IUSocketScript * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IUSocketScript * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Connect )( 
            IUSocketScript * This,
            /* [in] */ BSTR bstrHostAddress,
            /* [in] */ long lHostPort,
            /* [defaultvalue][in] */ VARIANT_BOOL bSynConnecting,
            /* [defaultvalue][in] */ long lSocketType,
            /* [defaultvalue][in] */ long lAddrFormat,
            /* [defaultvalue][in] */ long lProtocol,
            /* [defaultvalue][in] */ long lLocalPort);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            IUSocketScript * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Shutdown )( 
            IUSocketScript * This,
            /* [defaultvalue][in] */ long lHow);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetSockOpt )( 
            IUSocketScript * This,
            /* [in] */ long lOptName,
            /* [in] */ long lOptValue,
            /* [defaultvalue][in] */ long lLevel);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSockOpt )( 
            IUSocketScript * This,
            /* [in] */ long lOptName,
            /* [defaultvalue][in] */ long lLevel,
            /* [retval][out] */ long *plOptValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Send )( 
            IUSocketScript * This,
            /* [in] */ VARIANT vtData,
            /* [retval][out] */ long *plSent);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Recv )( 
            IUSocketScript * This,
            /* [retval][out] */ VARIANT *pvtData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IOCtl )( 
            IUSocketScript * This,
            /* [defaultvalue][in] */ long lCommand,
            /* [retval][out] */ long *plArgment);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetInterfaceAttributes )( 
            IUSocketScript * This,
            /* [out] */ long *lpMTU,
            /* [out] */ long *plMaxSpeed,
            /* [out] */ long *plIType,
            /* [out] */ BSTR *pbstrMask,
            /* [retval][out] */ BSTR *pbstrDesc);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetPeerName )( 
            IUSocketScript * This,
            /* [out] */ long *plPeerPort,
            /* [retval][out] */ BSTR *pbstrPeerName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetLocalName )( 
            IUSocketScript * This,
            /* [retval][out] */ BSTR *pbstrLocalName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetHostByName )( 
            IUSocketScript * This,
            /* [in] */ BSTR bstrHost,
            /* [defaultvalue][in] */ VARIANT_BOOL bBlocking,
            /* [defaultvalue][out] */ long *phHandle,
            /* [defaultvalue][out] */ BSTR *pbstrIPAddr,
            /* [defaultvalue][out] */ BSTR *pbstrAlias,
            /* [retval][out] */ BSTR *pbstrHostName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetHostByAddr )( 
            IUSocketScript * This,
            /* [in] */ BSTR bstrIPAddr,
            /* [defaultvalue][in] */ long lAddrFormat,
            /* [defaultvalue][in] */ VARIANT_BOOL bBlocking,
            /* [defaultvalue][out] */ long *phHandle,
            /* [defaultvalue][out] */ BSTR *pbstrAlias,
            /* [retval][out] */ BSTR *pbstrHostName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetProcAddress )( 
            IUSocketScript * This,
            /* [in] */ BSTR bstrOpenSSLFuncName,
            /* [retval][out] */ long *plProcAddr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSockAddr )( 
            IUSocketScript * This,
            /* [out] */ long *plPort,
            /* [retval][out] */ BSTR *pbstrSockAddr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetBytesSent )( 
            IUSocketScript * This,
            /* [defaultvalue][out] */ long *plHigh,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetBytesReceived )( 
            IUSocketScript * This,
            /* [defaultvalue][out] */ long *plHigh,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ConnTimeout )( 
            IUSocketScript * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ConnTimeout )( 
            IUSocketScript * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Socket )( 
            IUSocketScript * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_EncryptionMethod )( 
            IUSocketScript * This,
            /* [retval][out] */ short *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_EncryptionMethod )( 
            IUSocketScript * This,
            /* [in] */ short newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SSLHandle )( 
            IUSocketScript * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PeerCertificate )( 
            IUSocketScript * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UseBaseSocketOnly )( 
            IUSocketScript * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UseBaseSocketOnly )( 
            IUSocketScript * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_hWnd )( 
            IUSocketScript * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SwitchTo )( 
            IUSocketScript * This,
            /* [in] */ long lSvsID);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *StartBatching )( 
            IUSocketScript * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *CommitBatching )( 
            IUSocketScript * This,
            /* [defaultvalue][in] */ VARIANT_BOOL bSvrBatching);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *AbortBatching )( 
            IUSocketScript * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *WaitAll )( 
            IUSocketScript * This,
            /* [in] */ long lTimeOut,
            /* [retval][out] */ VARIANT_BOOL *pbTimeout);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DoEcho )( 
            IUSocketScript * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetRequestsInQueue )( 
            IUSocketScript * This,
            /* [retval][out] */ VARIANT *pvtRequests);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SendRequest )( 
            IUSocketScript * This,
            /* [in] */ short nRequestID,
            /* [in] */ VARIANT vtData,
            /* [defaultvalue][in] */ VARIANT_BOOL bKeepVTFormat);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetRtnBuffer )( 
            IUSocketScript * This,
            /* [in] */ long nLen,
            /* [retval][out] */ VARIANT *pvtData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            IUSocketScript * This,
            /* [defaultvalue][in] */ long lRequests);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetSockOptAtSvr )( 
            IUSocketScript * This,
            /* [in] */ long lOptName,
            /* [in] */ long lOptValue,
            /* [defaultvalue][in] */ long lLevel);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSockOptAtSvr )( 
            IUSocketScript * This,
            /* [in] */ long lOptName,
            /* [defaultvalue][in] */ long lLevel);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetOptValueAtSvr )( 
            IUSocketScript * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetClientVersion )( 
            IUSocketScript * This,
            /* [in] */ short nMinor,
            /* [in] */ short nMajor);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *TurnOnZipAtSvr )( 
            IUSocketScript * This,
            /* [defaultvalue][in] */ VARIANT_BOOL bZipOn);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetClientVersion )( 
            IUSocketScript * This,
            /* [out] */ short *pnMinor,
            /* [retval][out] */ short *pnMajor);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetServerVersion )( 
            IUSocketScript * This,
            /* [out] */ short *pnMinor,
            /* [out] */ short *pnMajor);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ShrinkMemory )( 
            IUSocketScript * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ShrinkMemoryAtSvr )( 
            IUSocketScript * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetClientUSockVersion )( 
            IUSocketScript * This,
            /* [out] */ short *pnMinor,
            /* [retval][out] */ short *pnMajor);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetServerUSockVersion )( 
            IUSocketScript * This,
            /* [out] */ short *pnMinor,
            /* [retval][out] */ short *pnMajor);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DetectNetPerformance )( 
            IUSocketScript * This,
            /* [retval][out] */ short *psNetPerformance);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IgnorePendingRequests )( 
            IUSocketScript * This,
            /* [defaultvalue][in] */ long lRequests,
            /* [defaultvalue][in] */ long lStartWith);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SendMySpecificBytes )( 
            IUSocketScript * This,
            /* [in] */ VARIANT vtBytes);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ErrorMsg )( 
            IUSocketScript * This,
            /* [retval][out] */ BSTR *pbstrErrorMsg);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Key )( 
            IUSocketScript * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Key )( 
            IUSocketScript * This,
            /* [in] */ VARIANT newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ZipIsOn )( 
            IUSocketScript * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ZipIsOn )( 
            IUSocketScript * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_UserID )( 
            IUSocketScript * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_UserID )( 
            IUSocketScript * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Password )( 
            IUSocketScript * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Password )( 
            IUSocketScript * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RcvMemory )( 
            IUSocketScript * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SndMemory )( 
            IUSocketScript * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Syn )( 
            IUSocketScript * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Syn )( 
            IUSocketScript * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ClientParams )( 
            IUSocketScript * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ClientParams )( 
            IUSocketScript * This,
            /* [in] */ VARIANT newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ServerParams )( 
            IUSocketScript * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RecvTimeout )( 
            IUSocketScript * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_RecvTimeout )( 
            IUSocketScript * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SendTimeout )( 
            IUSocketScript * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_SendTimeout )( 
            IUSocketScript * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_KeyFromSvr )( 
            IUSocketScript * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RequestsCanceled )( 
            IUSocketScript * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Frozen )( 
            IUSocketScript * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Frozen )( 
            IUSocketScript * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BytesInSndMemory )( 
            IUSocketScript * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BytesInRcvMemory )( 
            IUSocketScript * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ReturnEvents )( 
            IUSocketScript * This,
            /* [retval][out] */ short *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ReturnEvents )( 
            IUSocketScript * This,
            /* [in] */ short newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MaxRtns )( 
            IUSocketScript * This,
            /* [retval][out] */ short *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_MaxRtns )( 
            IUSocketScript * This,
            /* [in] */ short newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_RtnsInQueue )( 
            IUSocketScript * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CurrentSvsID )( 
            IUSocketScript * This,
            /* [retval][out] */ long *plSvsID);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ZipOnAtSvr )( 
            IUSocketScript * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Rtn )( 
            IUSocketScript * This,
            /* [retval][out] */ long *plRtn);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IsSameEndian )( 
            IUSocketScript * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BytesBatched )( 
            IUSocketScript * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Wait )( 
            IUSocketScript * This,
            /* [in] */ short sRequestID,
            /* [in] */ long lTimeout,
            /* [defaultvalue][in] */ long lSvsID,
            /* [retval][out] */ VARIANT_BOOL *pbTimeout);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CountOfRequestsInQueue )( 
            IUSocketScript * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_IsBatching )( 
            IUSocketScript * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *CleanTrack )( 
            IUSocketScript * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_LastRequestID )( 
            IUSocketScript * This,
            /* [retval][out] */ short *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *StartJob )( 
            IUSocketScript * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *EndJob )( 
            IUSocketScript * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Enter )( 
            IUSocketScript * This,
            /* [in] */ long lGroups);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Exit )( 
            IUSocketScript * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Speak )( 
            IUSocketScript * This,
            /* [in] */ VARIANT vtMsg,
            /* [defaultvalue][in] */ long lGroups);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SpeakTo )( 
            IUSocketScript * This,
            /* [in] */ BSTR bstrIPAddr,
            /* [in] */ long lPort,
            /* [in] */ VARIANT vtMsg);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetAllGroups )( 
            IUSocketScript * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetAllListeners )( 
            IUSocketScript * This,
            /* [defaultvalue][in] */ long lGroups);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetAllClients )( 
            IUSocketScript * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Listeners )( 
            IUSocketScript * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Message )( 
            IUSocketScript * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Groups )( 
            IUSocketScript * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SendUserMessage )( 
            IUSocketScript * This,
            /* [in] */ BSTR bstrUserID,
            /* [in] */ VARIANT vtMsg);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_JSUQueue )( 
            IUSocketScript * This,
            /* [retval][out] */ IJSSerialization **ppVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SendJSRequest )( 
            IUSocketScript * This,
            /* [in] */ short sRequestId);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_JSObject )( 
            IUSocketScript * This,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_JSObject )( 
            IUSocketScript * This,
            /* [in] */ VARIANT newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetZipLevelAtSvr )( 
            IUSocketScript * This,
            /* [in] */ ZipLevel zipLevel);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ZipLevel )( 
            IUSocketScript * This,
            /* [retval][out] */ ZipLevel *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ZipLevel )( 
            IUSocketScript * This,
            /* [in] */ ZipLevel newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ZipLevelAtSvr )( 
            IUSocketScript * This,
            /* [retval][out] */ ZipLevel *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetGroupID )( 
            IUSocketScript * This,
            /* [in] */ long lIndex,
            /* [retval][out] */ long *plGroupID);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetGroupDescription )( 
            IUSocketScript * This,
            /* [in] */ long lIndex,
            /* [retval][out] */ BSTR *pbstrDescription);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetInfoGroupIDs )( 
            IUSocketScript * This,
            /* [in] */ long lIndex,
            /* [retval][out] */ long *plGroupIDs);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetInfoUserID )( 
            IUSocketScript * This,
            /* [in] */ long lIndex,
            /* [retval][out] */ BSTR *pbstrUserID);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetInfoServiceID )( 
            IUSocketScript * This,
            /* [in] */ long lIndex,
            /* [retval][out] */ long *plSvsID);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetInfoPort )( 
            IUSocketScript * This,
            /* [in] */ long lIndex,
            /* [retval][out] */ long *plPort);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetInfoIPAddress )( 
            IUSocketScript * This,
            /* [in] */ long lIndex,
            /* [retval][out] */ BSTR *pbstrIPAddress);
        
        END_INTERFACE
    } IUSocketScriptVtbl;

    interface IUSocketScript
    {
        CONST_VTBL struct IUSocketScriptVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUSocketScript_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUSocketScript_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUSocketScript_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUSocketScript_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IUSocketScript_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IUSocketScript_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IUSocketScript_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IUSocketScript_Connect(This,bstrHostAddress,lHostPort,bSynConnecting,lSocketType,lAddrFormat,lProtocol,lLocalPort)	\
    ( (This)->lpVtbl -> Connect(This,bstrHostAddress,lHostPort,bSynConnecting,lSocketType,lAddrFormat,lProtocol,lLocalPort) ) 

#define IUSocketScript_Disconnect(This)	\
    ( (This)->lpVtbl -> Disconnect(This) ) 

#define IUSocketScript_Shutdown(This,lHow)	\
    ( (This)->lpVtbl -> Shutdown(This,lHow) ) 

#define IUSocketScript_SetSockOpt(This,lOptName,lOptValue,lLevel)	\
    ( (This)->lpVtbl -> SetSockOpt(This,lOptName,lOptValue,lLevel) ) 

#define IUSocketScript_GetSockOpt(This,lOptName,lLevel,plOptValue)	\
    ( (This)->lpVtbl -> GetSockOpt(This,lOptName,lLevel,plOptValue) ) 

#define IUSocketScript_Send(This,vtData,plSent)	\
    ( (This)->lpVtbl -> Send(This,vtData,plSent) ) 

#define IUSocketScript_Recv(This,pvtData)	\
    ( (This)->lpVtbl -> Recv(This,pvtData) ) 

#define IUSocketScript_IOCtl(This,lCommand,plArgment)	\
    ( (This)->lpVtbl -> IOCtl(This,lCommand,plArgment) ) 

#define IUSocketScript_GetInterfaceAttributes(This,lpMTU,plMaxSpeed,plIType,pbstrMask,pbstrDesc)	\
    ( (This)->lpVtbl -> GetInterfaceAttributes(This,lpMTU,plMaxSpeed,plIType,pbstrMask,pbstrDesc) ) 

#define IUSocketScript_GetPeerName(This,plPeerPort,pbstrPeerName)	\
    ( (This)->lpVtbl -> GetPeerName(This,plPeerPort,pbstrPeerName) ) 

#define IUSocketScript_GetLocalName(This,pbstrLocalName)	\
    ( (This)->lpVtbl -> GetLocalName(This,pbstrLocalName) ) 

#define IUSocketScript_GetHostByName(This,bstrHost,bBlocking,phHandle,pbstrIPAddr,pbstrAlias,pbstrHostName)	\
    ( (This)->lpVtbl -> GetHostByName(This,bstrHost,bBlocking,phHandle,pbstrIPAddr,pbstrAlias,pbstrHostName) ) 

#define IUSocketScript_GetHostByAddr(This,bstrIPAddr,lAddrFormat,bBlocking,phHandle,pbstrAlias,pbstrHostName)	\
    ( (This)->lpVtbl -> GetHostByAddr(This,bstrIPAddr,lAddrFormat,bBlocking,phHandle,pbstrAlias,pbstrHostName) ) 

#define IUSocketScript_GetProcAddress(This,bstrOpenSSLFuncName,plProcAddr)	\
    ( (This)->lpVtbl -> GetProcAddress(This,bstrOpenSSLFuncName,plProcAddr) ) 

#define IUSocketScript_GetSockAddr(This,plPort,pbstrSockAddr)	\
    ( (This)->lpVtbl -> GetSockAddr(This,plPort,pbstrSockAddr) ) 

#define IUSocketScript_GetBytesSent(This,plHigh,pVal)	\
    ( (This)->lpVtbl -> GetBytesSent(This,plHigh,pVal) ) 

#define IUSocketScript_GetBytesReceived(This,plHigh,pVal)	\
    ( (This)->lpVtbl -> GetBytesReceived(This,plHigh,pVal) ) 

#define IUSocketScript_get_ConnTimeout(This,pVal)	\
    ( (This)->lpVtbl -> get_ConnTimeout(This,pVal) ) 

#define IUSocketScript_put_ConnTimeout(This,newVal)	\
    ( (This)->lpVtbl -> put_ConnTimeout(This,newVal) ) 

#define IUSocketScript_get_Socket(This,pVal)	\
    ( (This)->lpVtbl -> get_Socket(This,pVal) ) 

#define IUSocketScript_get_EncryptionMethod(This,pVal)	\
    ( (This)->lpVtbl -> get_EncryptionMethod(This,pVal) ) 

#define IUSocketScript_put_EncryptionMethod(This,newVal)	\
    ( (This)->lpVtbl -> put_EncryptionMethod(This,newVal) ) 

#define IUSocketScript_get_SSLHandle(This,pVal)	\
    ( (This)->lpVtbl -> get_SSLHandle(This,pVal) ) 

#define IUSocketScript_get_PeerCertificate(This,pVal)	\
    ( (This)->lpVtbl -> get_PeerCertificate(This,pVal) ) 

#define IUSocketScript_get_UseBaseSocketOnly(This,pVal)	\
    ( (This)->lpVtbl -> get_UseBaseSocketOnly(This,pVal) ) 

#define IUSocketScript_put_UseBaseSocketOnly(This,newVal)	\
    ( (This)->lpVtbl -> put_UseBaseSocketOnly(This,newVal) ) 

#define IUSocketScript_get_hWnd(This,pVal)	\
    ( (This)->lpVtbl -> get_hWnd(This,pVal) ) 


#define IUSocketScript_SwitchTo(This,lSvsID)	\
    ( (This)->lpVtbl -> SwitchTo(This,lSvsID) ) 

#define IUSocketScript_StartBatching(This)	\
    ( (This)->lpVtbl -> StartBatching(This) ) 

#define IUSocketScript_CommitBatching(This,bSvrBatching)	\
    ( (This)->lpVtbl -> CommitBatching(This,bSvrBatching) ) 

#define IUSocketScript_AbortBatching(This)	\
    ( (This)->lpVtbl -> AbortBatching(This) ) 

#define IUSocketScript_WaitAll(This,lTimeOut,pbTimeout)	\
    ( (This)->lpVtbl -> WaitAll(This,lTimeOut,pbTimeout) ) 

#define IUSocketScript_DoEcho(This)	\
    ( (This)->lpVtbl -> DoEcho(This) ) 

#define IUSocketScript_GetRequestsInQueue(This,pvtRequests)	\
    ( (This)->lpVtbl -> GetRequestsInQueue(This,pvtRequests) ) 

#define IUSocketScript_SendRequest(This,nRequestID,vtData,bKeepVTFormat)	\
    ( (This)->lpVtbl -> SendRequest(This,nRequestID,vtData,bKeepVTFormat) ) 

#define IUSocketScript_GetRtnBuffer(This,nLen,pvtData)	\
    ( (This)->lpVtbl -> GetRtnBuffer(This,nLen,pvtData) ) 

#define IUSocketScript_Cancel(This,lRequests)	\
    ( (This)->lpVtbl -> Cancel(This,lRequests) ) 

#define IUSocketScript_SetSockOptAtSvr(This,lOptName,lOptValue,lLevel)	\
    ( (This)->lpVtbl -> SetSockOptAtSvr(This,lOptName,lOptValue,lLevel) ) 

#define IUSocketScript_GetSockOptAtSvr(This,lOptName,lLevel)	\
    ( (This)->lpVtbl -> GetSockOptAtSvr(This,lOptName,lLevel) ) 

#define IUSocketScript_GetOptValueAtSvr(This,pVal)	\
    ( (This)->lpVtbl -> GetOptValueAtSvr(This,pVal) ) 

#define IUSocketScript_SetClientVersion(This,nMinor,nMajor)	\
    ( (This)->lpVtbl -> SetClientVersion(This,nMinor,nMajor) ) 

#define IUSocketScript_TurnOnZipAtSvr(This,bZipOn)	\
    ( (This)->lpVtbl -> TurnOnZipAtSvr(This,bZipOn) ) 

#define IUSocketScript_GetClientVersion(This,pnMinor,pnMajor)	\
    ( (This)->lpVtbl -> GetClientVersion(This,pnMinor,pnMajor) ) 

#define IUSocketScript_GetServerVersion(This,pnMinor,pnMajor)	\
    ( (This)->lpVtbl -> GetServerVersion(This,pnMinor,pnMajor) ) 

#define IUSocketScript_ShrinkMemory(This)	\
    ( (This)->lpVtbl -> ShrinkMemory(This) ) 

#define IUSocketScript_ShrinkMemoryAtSvr(This)	\
    ( (This)->lpVtbl -> ShrinkMemoryAtSvr(This) ) 

#define IUSocketScript_GetClientUSockVersion(This,pnMinor,pnMajor)	\
    ( (This)->lpVtbl -> GetClientUSockVersion(This,pnMinor,pnMajor) ) 

#define IUSocketScript_GetServerUSockVersion(This,pnMinor,pnMajor)	\
    ( (This)->lpVtbl -> GetServerUSockVersion(This,pnMinor,pnMajor) ) 

#define IUSocketScript_DetectNetPerformance(This,psNetPerformance)	\
    ( (This)->lpVtbl -> DetectNetPerformance(This,psNetPerformance) ) 

#define IUSocketScript_IgnorePendingRequests(This,lRequests,lStartWith)	\
    ( (This)->lpVtbl -> IgnorePendingRequests(This,lRequests,lStartWith) ) 

#define IUSocketScript_SendMySpecificBytes(This,vtBytes)	\
    ( (This)->lpVtbl -> SendMySpecificBytes(This,vtBytes) ) 

#define IUSocketScript_get_ErrorMsg(This,pbstrErrorMsg)	\
    ( (This)->lpVtbl -> get_ErrorMsg(This,pbstrErrorMsg) ) 

#define IUSocketScript_get_Key(This,pVal)	\
    ( (This)->lpVtbl -> get_Key(This,pVal) ) 

#define IUSocketScript_put_Key(This,newVal)	\
    ( (This)->lpVtbl -> put_Key(This,newVal) ) 

#define IUSocketScript_get_ZipIsOn(This,pVal)	\
    ( (This)->lpVtbl -> get_ZipIsOn(This,pVal) ) 

#define IUSocketScript_put_ZipIsOn(This,newVal)	\
    ( (This)->lpVtbl -> put_ZipIsOn(This,newVal) ) 

#define IUSocketScript_get_UserID(This,pVal)	\
    ( (This)->lpVtbl -> get_UserID(This,pVal) ) 

#define IUSocketScript_put_UserID(This,newVal)	\
    ( (This)->lpVtbl -> put_UserID(This,newVal) ) 

#define IUSocketScript_get_Password(This,pVal)	\
    ( (This)->lpVtbl -> get_Password(This,pVal) ) 

#define IUSocketScript_put_Password(This,newVal)	\
    ( (This)->lpVtbl -> put_Password(This,newVal) ) 

#define IUSocketScript_get_RcvMemory(This,pVal)	\
    ( (This)->lpVtbl -> get_RcvMemory(This,pVal) ) 

#define IUSocketScript_get_SndMemory(This,pVal)	\
    ( (This)->lpVtbl -> get_SndMemory(This,pVal) ) 

#define IUSocketScript_get_Syn(This,pVal)	\
    ( (This)->lpVtbl -> get_Syn(This,pVal) ) 

#define IUSocketScript_put_Syn(This,newVal)	\
    ( (This)->lpVtbl -> put_Syn(This,newVal) ) 

#define IUSocketScript_get_ClientParams(This,pVal)	\
    ( (This)->lpVtbl -> get_ClientParams(This,pVal) ) 

#define IUSocketScript_put_ClientParams(This,newVal)	\
    ( (This)->lpVtbl -> put_ClientParams(This,newVal) ) 

#define IUSocketScript_get_ServerParams(This,pVal)	\
    ( (This)->lpVtbl -> get_ServerParams(This,pVal) ) 

#define IUSocketScript_get_RecvTimeout(This,pVal)	\
    ( (This)->lpVtbl -> get_RecvTimeout(This,pVal) ) 

#define IUSocketScript_put_RecvTimeout(This,newVal)	\
    ( (This)->lpVtbl -> put_RecvTimeout(This,newVal) ) 

#define IUSocketScript_get_SendTimeout(This,pVal)	\
    ( (This)->lpVtbl -> get_SendTimeout(This,pVal) ) 

#define IUSocketScript_put_SendTimeout(This,newVal)	\
    ( (This)->lpVtbl -> put_SendTimeout(This,newVal) ) 

#define IUSocketScript_get_KeyFromSvr(This,pVal)	\
    ( (This)->lpVtbl -> get_KeyFromSvr(This,pVal) ) 

#define IUSocketScript_get_RequestsCanceled(This,pVal)	\
    ( (This)->lpVtbl -> get_RequestsCanceled(This,pVal) ) 

#define IUSocketScript_get_Frozen(This,pVal)	\
    ( (This)->lpVtbl -> get_Frozen(This,pVal) ) 

#define IUSocketScript_put_Frozen(This,newVal)	\
    ( (This)->lpVtbl -> put_Frozen(This,newVal) ) 

#define IUSocketScript_get_BytesInSndMemory(This,pVal)	\
    ( (This)->lpVtbl -> get_BytesInSndMemory(This,pVal) ) 

#define IUSocketScript_get_BytesInRcvMemory(This,pVal)	\
    ( (This)->lpVtbl -> get_BytesInRcvMemory(This,pVal) ) 

#define IUSocketScript_get_ReturnEvents(This,pVal)	\
    ( (This)->lpVtbl -> get_ReturnEvents(This,pVal) ) 

#define IUSocketScript_put_ReturnEvents(This,newVal)	\
    ( (This)->lpVtbl -> put_ReturnEvents(This,newVal) ) 

#define IUSocketScript_get_MaxRtns(This,pVal)	\
    ( (This)->lpVtbl -> get_MaxRtns(This,pVal) ) 

#define IUSocketScript_put_MaxRtns(This,newVal)	\
    ( (This)->lpVtbl -> put_MaxRtns(This,newVal) ) 

#define IUSocketScript_get_RtnsInQueue(This,pVal)	\
    ( (This)->lpVtbl -> get_RtnsInQueue(This,pVal) ) 

#define IUSocketScript_get_CurrentSvsID(This,plSvsID)	\
    ( (This)->lpVtbl -> get_CurrentSvsID(This,plSvsID) ) 

#define IUSocketScript_get_ZipOnAtSvr(This,pVal)	\
    ( (This)->lpVtbl -> get_ZipOnAtSvr(This,pVal) ) 

#define IUSocketScript_get_Rtn(This,plRtn)	\
    ( (This)->lpVtbl -> get_Rtn(This,plRtn) ) 

#define IUSocketScript_get_IsSameEndian(This,pVal)	\
    ( (This)->lpVtbl -> get_IsSameEndian(This,pVal) ) 

#define IUSocketScript_get_BytesBatched(This,pVal)	\
    ( (This)->lpVtbl -> get_BytesBatched(This,pVal) ) 

#define IUSocketScript_Wait(This,sRequestID,lTimeout,lSvsID,pbTimeout)	\
    ( (This)->lpVtbl -> Wait(This,sRequestID,lTimeout,lSvsID,pbTimeout) ) 

#define IUSocketScript_get_CountOfRequestsInQueue(This,pVal)	\
    ( (This)->lpVtbl -> get_CountOfRequestsInQueue(This,pVal) ) 

#define IUSocketScript_get_IsBatching(This,pVal)	\
    ( (This)->lpVtbl -> get_IsBatching(This,pVal) ) 

#define IUSocketScript_CleanTrack(This)	\
    ( (This)->lpVtbl -> CleanTrack(This) ) 

#define IUSocketScript_get_LastRequestID(This,pVal)	\
    ( (This)->lpVtbl -> get_LastRequestID(This,pVal) ) 

#define IUSocketScript_StartJob(This)	\
    ( (This)->lpVtbl -> StartJob(This) ) 

#define IUSocketScript_EndJob(This)	\
    ( (This)->lpVtbl -> EndJob(This) ) 


#define IUSocketScript_Enter(This,lGroups)	\
    ( (This)->lpVtbl -> Enter(This,lGroups) ) 

#define IUSocketScript_Exit(This)	\
    ( (This)->lpVtbl -> Exit(This) ) 

#define IUSocketScript_Speak(This,vtMsg,lGroups)	\
    ( (This)->lpVtbl -> Speak(This,vtMsg,lGroups) ) 

#define IUSocketScript_SpeakTo(This,bstrIPAddr,lPort,vtMsg)	\
    ( (This)->lpVtbl -> SpeakTo(This,bstrIPAddr,lPort,vtMsg) ) 

#define IUSocketScript_GetAllGroups(This)	\
    ( (This)->lpVtbl -> GetAllGroups(This) ) 

#define IUSocketScript_GetAllListeners(This,lGroups)	\
    ( (This)->lpVtbl -> GetAllListeners(This,lGroups) ) 

#define IUSocketScript_GetAllClients(This)	\
    ( (This)->lpVtbl -> GetAllClients(This) ) 

#define IUSocketScript_get_Listeners(This,pVal)	\
    ( (This)->lpVtbl -> get_Listeners(This,pVal) ) 

#define IUSocketScript_get_Message(This,pVal)	\
    ( (This)->lpVtbl -> get_Message(This,pVal) ) 

#define IUSocketScript_get_Groups(This,pVal)	\
    ( (This)->lpVtbl -> get_Groups(This,pVal) ) 

#define IUSocketScript_SendUserMessage(This,bstrUserID,vtMsg)	\
    ( (This)->lpVtbl -> SendUserMessage(This,bstrUserID,vtMsg) ) 

#define IUSocketScript_get_JSUQueue(This,ppVal)	\
    ( (This)->lpVtbl -> get_JSUQueue(This,ppVal) ) 

#define IUSocketScript_SendJSRequest(This,sRequestId)	\
    ( (This)->lpVtbl -> SendJSRequest(This,sRequestId) ) 

#define IUSocketScript_get_JSObject(This,pVal)	\
    ( (This)->lpVtbl -> get_JSObject(This,pVal) ) 

#define IUSocketScript_put_JSObject(This,newVal)	\
    ( (This)->lpVtbl -> put_JSObject(This,newVal) ) 

#define IUSocketScript_SetZipLevelAtSvr(This,zipLevel)	\
    ( (This)->lpVtbl -> SetZipLevelAtSvr(This,zipLevel) ) 

#define IUSocketScript_get_ZipLevel(This,pVal)	\
    ( (This)->lpVtbl -> get_ZipLevel(This,pVal) ) 

#define IUSocketScript_put_ZipLevel(This,newVal)	\
    ( (This)->lpVtbl -> put_ZipLevel(This,newVal) ) 

#define IUSocketScript_get_ZipLevelAtSvr(This,pVal)	\
    ( (This)->lpVtbl -> get_ZipLevelAtSvr(This,pVal) ) 

#define IUSocketScript_GetGroupID(This,lIndex,plGroupID)	\
    ( (This)->lpVtbl -> GetGroupID(This,lIndex,plGroupID) ) 

#define IUSocketScript_GetGroupDescription(This,lIndex,pbstrDescription)	\
    ( (This)->lpVtbl -> GetGroupDescription(This,lIndex,pbstrDescription) ) 

#define IUSocketScript_GetInfoGroupIDs(This,lIndex,plGroupIDs)	\
    ( (This)->lpVtbl -> GetInfoGroupIDs(This,lIndex,plGroupIDs) ) 

#define IUSocketScript_GetInfoUserID(This,lIndex,pbstrUserID)	\
    ( (This)->lpVtbl -> GetInfoUserID(This,lIndex,pbstrUserID) ) 

#define IUSocketScript_GetInfoServiceID(This,lIndex,plSvsID)	\
    ( (This)->lpVtbl -> GetInfoServiceID(This,lIndex,plSvsID) ) 

#define IUSocketScript_GetInfoPort(This,lIndex,plPort)	\
    ( (This)->lpVtbl -> GetInfoPort(This,lIndex,plPort) ) 

#define IUSocketScript_GetInfoIPAddress(This,lIndex,pbstrIPAddress)	\
    ( (This)->lpVtbl -> GetInfoIPAddress(This,lIndex,pbstrIPAddress) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUSocketScript_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_USocket;

#ifdef __cplusplus

class DECLSPEC_UUID("AAD83433-CC8D-4F6D-9C62-E224DCA81358")
USocket;
#endif

#ifndef ___IUSocketPoolEvents_DISPINTERFACE_DEFINED__
#define ___IUSocketPoolEvents_DISPINTERFACE_DEFINED__

/* dispinterface _IUSocketPoolEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__IUSocketPoolEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("2CC0BCFE-0361-4788-96E5-76A64D2B11BE")
    _IUSocketPoolEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IUSocketPoolEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _IUSocketPoolEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _IUSocketPoolEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _IUSocketPoolEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _IUSocketPoolEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _IUSocketPoolEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _IUSocketPoolEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _IUSocketPoolEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _IUSocketPoolEventsVtbl;

    interface _IUSocketPoolEvents
    {
        CONST_VTBL struct _IUSocketPoolEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IUSocketPoolEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define _IUSocketPoolEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define _IUSocketPoolEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define _IUSocketPoolEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define _IUSocketPoolEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define _IUSocketPoolEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define _IUSocketPoolEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IUSocketPoolEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_USocketPool;

#ifdef __cplusplus

class DECLSPEC_UUID("0A3CD705-6C31-4C9C-91BE-BDFF4EECF8CB")
USocketPool;
#endif
#endif /* __USOCKETLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


