/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Wed Apr 18 21:52:28 2007
 */
/* Compiler settings for C:\Program Files\UDAParts\SocketPro\DataBase\UDB\UDB.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __UDB_h__
#define __UDB_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IUObjBase_FWD_DEFINED__
#define __IUObjBase_FWD_DEFINED__
typedef interface IUObjBase IUObjBase;
#endif 	/* __IUObjBase_FWD_DEFINED__ */


#ifndef ___IURequestEvent_FWD_DEFINED__
#define ___IURequestEvent_FWD_DEFINED__
typedef interface _IURequestEvent _IURequestEvent;
#endif 	/* ___IURequestEvent_FWD_DEFINED__ */


#ifndef __IUDataSource_FWD_DEFINED__
#define __IUDataSource_FWD_DEFINED__
typedef interface IUDataSource IUDataSource;
#endif 	/* __IUDataSource_FWD_DEFINED__ */


#ifndef __IUSession_FWD_DEFINED__
#define __IUSession_FWD_DEFINED__
typedef interface IUSession IUSession;
#endif 	/* __IUSession_FWD_DEFINED__ */


#ifndef __IUCommand_FWD_DEFINED__
#define __IUCommand_FWD_DEFINED__
typedef interface IUCommand IUCommand;
#endif 	/* __IUCommand_FWD_DEFINED__ */


#ifndef __IURowset_FWD_DEFINED__
#define __IURowset_FWD_DEFINED__
typedef interface IURowset IURowset;
#endif 	/* __IURowset_FWD_DEFINED__ */


#ifndef __IUDataReader_FWD_DEFINED__
#define __IUDataReader_FWD_DEFINED__
typedef interface IUDataReader IUDataReader;
#endif 	/* __IUDataReader_FWD_DEFINED__ */


#ifndef __UDataSource_FWD_DEFINED__
#define __UDataSource_FWD_DEFINED__

#ifdef __cplusplus
typedef class UDataSource UDataSource;
#else
typedef struct UDataSource UDataSource;
#endif /* __cplusplus */

#endif 	/* __UDataSource_FWD_DEFINED__ */


#ifndef __USession_FWD_DEFINED__
#define __USession_FWD_DEFINED__

#ifdef __cplusplus
typedef class USession USession;
#else
typedef struct USession USession;
#endif /* __cplusplus */

#endif 	/* __USession_FWD_DEFINED__ */


#ifndef __UCommand_FWD_DEFINED__
#define __UCommand_FWD_DEFINED__

#ifdef __cplusplus
typedef class UCommand UCommand;
#else
typedef struct UCommand UCommand;
#endif /* __cplusplus */

#endif 	/* __UCommand_FWD_DEFINED__ */


#ifndef __URowset_FWD_DEFINED__
#define __URowset_FWD_DEFINED__

#ifdef __cplusplus
typedef class URowset URowset;
#else
typedef struct URowset URowset;
#endif /* __cplusplus */

#endif 	/* __URowset_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __UDBLib_LIBRARY_DEFINED__
#define __UDBLib_LIBRARY_DEFINED__

/* library UDBLib */
/* [helpstring][version][uuid] */ 


enum tagDatasourceHint
    {	dhDefault	= 0,
	dhFromGlobal	= 1,
	dhFromIBindResource	= 2
    };

enum tagSessionHint
    {	shDefault	= 0,
	shFromDS	= 1,
	shFromGlobal	= 2,
	shFromIBindResource	= 4
    };

enum tagCommandHint
    {	chDefault	= 0,
	chFromGlobal	= 1,
	chCommandPrepare	= 2
    };

enum tagCursorType
    {	ctForwardOnly	= 0,
	ctStatic	= 1,
	ctKeyset	= 2,
	ctDynamic	= 3
    };

enum tagCreatedObject
    {	coNothing	= 0,
	coDataSource	= 1,
	coSession	= coDataSource + 1,
	coCommand	= coSession + 1,
	coRowset	= coCommand + 1,
	coMultipleResults	= coRowset + 1,
	coView	= coMultipleResults + 1,
	coChapter	= coView + 1,
	coBinder	= coChapter + 1,
	coRow	= coBinder + 1,
	coStream	= coRow + 1,
	coEnumerator	= coStream + 1,
	coDataSet	= coEnumerator + 1
    };

enum tagRowsetHint
    {	rhDefault	= 0,
	rhFromCommand	= 0,
	rhTable	= 1,
	rhReadOnly	= 2,
	rhBatchUpdate	= 4,
	rhScrollable	= 8,
	rhUseBookmark	= 16,
	rhUseCCE	= 32,
	rhServerDataOnInsert	= 64,
	rhFromIMultipleResults	= 128,
	rhFromIBindResource	= 256,
	rhFromICreateRow	= 512,
	rhKeepPreviousRowset	= 1024
    };

enum tagIsolationLevel
    {	ilUnspecified	= 0xffffffff,
	ilChaos	= 0x10,
	ilReadUncommitted	= 0x100,
	ilBrowse	= 0x100,
	ilReadCommitted	= 0x1000,
	ilCursorStability	= 0x1000,
	ilRepeatableRead	= 0x10000,
	ilSerializable	= 0x100000,
	ilIsolated	= 0x100000
    };

enum tagDBRequestID
    {	idDSOpen	= 86,
	idDSClose	= idDSOpen + 1,
	idDSGetProperty	= idDSClose + 1,
	idDSGetPropFlags	= idDSGetProperty + 1,
	idDSOpenFromHandle	= idDSGetPropFlags + 1,
	idDSSetProperty	= idDSOpenFromHandle + 1,
	idSessionOpen	= 96,
	idSessionClose	= idSessionOpen + 1,
	idSessionBeginTrans	= idSessionClose + 1,
	idSessionCommit	= idSessionBeginTrans + 1,
	idSessionRollback	= idSessionCommit + 1,
	idSessionSetProperty	= idSessionRollback + 1,
	idSessionGetProperty	= idSessionSetProperty + 1,
	idSessionOpenFromHandle	= idSessionGetProperty + 1,
	idCmndOpen	= 116,
	idCmndClose	= idCmndOpen + 1,
	idCmndExecuteSQL	= idCmndClose + 1,
	idCmndCancel	= idCmndExecuteSQL + 1,
	idCmndGetProperty	= idCmndCancel + 1,
	idCmndSetProperty	= idCmndGetProperty + 1,
	idCmndPrepare	= idCmndSetProperty + 1,
	idCmndUnprepare	= idCmndPrepare + 1,
	idCmndDoBatch	= idCmndUnprepare + 1,
	idCmndOpenFromHandle	= idCmndDoBatch + 1,
	idCmndGetOutputParams	= idCmndOpenFromHandle + 1,
	idCmndUseStorageObjectForBLOB	= idCmndGetOutputParams + 1,
	idCmndReleaseCreatedObject	= idCmndUseStorageObjectForBLOB + 1,
	idRowsetOpen	= 136,
	idRowsetClose	= idRowsetOpen + 1,
	idRowsetMoveFirst	= idRowsetClose + 1,
	idRowsetMoveLast	= idRowsetMoveFirst + 1,
	idRowsetMovePrev	= idRowsetMoveLast + 1,
	idRowsetMoveNext	= idRowsetMovePrev + 1,
	idRowsetUpdate	= idRowsetMoveNext + 1,
	idRowsetUpdateBatch	= idRowsetUpdate + 1,
	idRowsetAdd	= idRowsetUpdateBatch + 1,
	idRowsetDelete	= idRowsetAdd + 1,
	idRowsetAsynFetch	= idRowsetDelete + 1,
	idRowsetSetDataType	= idRowsetAsynFetch + 1,
	idRowsetOpenFromHandle	= idRowsetSetDataType + 1,
	idRowsetGetBatchRecords	= idRowsetOpenFromHandle + 1,
	idRowsetBookmark	= idRowsetGetBatchRecords + 1,
	idRowsetUndo	= idRowsetBookmark + 1,
	idRowsetGetRowsAt	= idRowsetUndo + 1,
	idRowsetGetSchemaRowset	= idRowsetGetRowsAt + 1,
	idRowsetGetProviders	= idRowsetGetSchemaRowset + 1,
	idRowsetGetProperty	= idRowsetGetProviders + 1,
	idRowsetUseStorageObjectForBLOB	= idRowsetGetProperty + 1,
	idRowsetSendSubBatch	= idRowsetUseStorageObjectForBLOB + 1,
	idRowsetSendBLOB	= idRowsetSendSubBatch + 1,
	idRowsetStartFetchingBatch	= idRowsetSendBLOB + 1,
	idRowsetGetBatchRecordsEx	= idRowsetStartFetchingBatch + 1,
	idRowsetGetBatchRecordsLast	= idRowsetGetBatchRecordsEx + 1
    };

enum tagSockDataType
    {	sdVT_EMPTY	= 0,
	sdVT_I2	= 2,
	sdVT_I4	= 3,
	sdVT_R4	= 4,
	sdVT_R8	= 5,
	sdVT_CY	= 6,
	sdVT_DATE	= 7,
	sdVT_BOOL	= 11,
	sdVT_VARIANT	= 12,
	sdVT_DECIMAL	= 14,
	sdVT_I1	= 16,
	sdVT_UI1	= 17,
	sdVT_UI2	= 18,
	sdVT_UI4	= 19,
	sdVT_I8	= 20,
	sdVT_UI8	= 21,
	sdVT_BYTES	= 128,
	sdVT_STR	= 129,
	sdVT_WSTR	= 130
    };

enum tagSockDBParamType
    {	sdParamInput	= 1,
	sdParamOutput	= 2,
	sdParamInputOutput	= 3
    };

enum tagDBError
    {	dbeFail	= 0x8f300001,
	dbeDataTypeError	= 0x8f300002,
	dbeLengthTooShort	= 0x8f300003
    };

EXTERN_C const IID LIBID_UDBLib;

#ifndef __IUObjBase_INTERFACE_DEFINED__
#define __IUObjBase_INTERFACE_DEFINED__

/* interface IUObjBase */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IUObjBase;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("20F08C2E-792C-10D6-B1D1-0010B5EC085B")
    IUObjBase : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AttachSocket( 
            /* [in] */ IUnknown __RPC_FAR *pIUnknownToUSocket) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Rtn( 
            /* [retval][out] */ long __RPC_FAR *plResult) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ErrorMsg( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUObjBaseVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IUObjBase __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IUObjBase __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IUObjBase __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IUObjBase __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IUObjBase __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IUObjBase __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IUObjBase __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AttachSocket )( 
            IUObjBase __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pIUnknownToUSocket);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Rtn )( 
            IUObjBase __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plResult);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ErrorMsg )( 
            IUObjBase __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        END_INTERFACE
    } IUObjBaseVtbl;

    interface IUObjBase
    {
        CONST_VTBL struct IUObjBaseVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUObjBase_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUObjBase_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUObjBase_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUObjBase_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IUObjBase_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IUObjBase_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IUObjBase_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IUObjBase_AttachSocket(This,pIUnknownToUSocket)	\
    (This)->lpVtbl -> AttachSocket(This,pIUnknownToUSocket)

#define IUObjBase_get_Rtn(This,plResult)	\
    (This)->lpVtbl -> get_Rtn(This,plResult)

#define IUObjBase_get_ErrorMsg(This,pVal)	\
    (This)->lpVtbl -> get_ErrorMsg(This,pVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUObjBase_AttachSocket_Proxy( 
    IUObjBase __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pIUnknownToUSocket);


void __RPC_STUB IUObjBase_AttachSocket_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUObjBase_get_Rtn_Proxy( 
    IUObjBase __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *plResult);


void __RPC_STUB IUObjBase_get_Rtn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUObjBase_get_ErrorMsg_Proxy( 
    IUObjBase __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IUObjBase_get_ErrorMsg_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IUObjBase_INTERFACE_DEFINED__ */


#ifndef ___IURequestEvent_DISPINTERFACE_DEFINED__
#define ___IURequestEvent_DISPINTERFACE_DEFINED__

/* dispinterface _IURequestEvent */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__IURequestEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("F8000F37-147E-4C43-B889-1D4C9A612009")
    _IURequestEvent : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IURequestEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            _IURequestEvent __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            _IURequestEvent __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            _IURequestEvent __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            _IURequestEvent __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            _IURequestEvent __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            _IURequestEvent __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            _IURequestEvent __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } _IURequestEventVtbl;

    interface _IURequestEvent
    {
        CONST_VTBL struct _IURequestEventVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IURequestEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IURequestEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IURequestEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IURequestEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _IURequestEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _IURequestEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _IURequestEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IURequestEvent_DISPINTERFACE_DEFINED__ */


#ifndef __IUDataSource_INTERFACE_DEFINED__
#define __IUDataSource_INTERFACE_DEFINED__

/* interface IUDataSource */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IUDataSource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5731AC6E-E2B6-4352-9593-CAB22BCF57EA")
    IUDataSource : public IUObjBase
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Open( 
            /* [in] */ BSTR bstrConnection,
            /* [defaultvalue][in] */ long lHint = dhDefault) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetProperty( 
            /* [in] */ long lPropID,
            /* [defaultvalue][in] */ BSTR bstrPropSet = L"{c8b522bb-5cf3-11ce-ade5-00aa0044773d}") = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetPropFlags( 
            /* [in] */ long lPropID,
            /* [defaultvalue][in] */ BSTR bstrPropSet = L"{c8b522be-5cf3-11ce-ade5-00aa0044773d}") = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OpenFromHandle( 
            /* [in] */ long lHandle) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Handle( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Flags( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Property( 
            /* [retval][out] */ VARIANT __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ParentHandle( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ParentHandle( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetProperty( 
            /* [in] */ VARIANT vtValue,
            /* [in] */ long lPropID,
            /* [defaultvalue][in] */ BSTR bstrPropSet = L"{c8b522ba-5cf3-11ce-ade5-00aa0044773d}") = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUDataSourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IUDataSource __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IUDataSource __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IUDataSource __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IUDataSource __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IUDataSource __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IUDataSource __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IUDataSource __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AttachSocket )( 
            IUDataSource __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pIUnknownToUSocket);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Rtn )( 
            IUDataSource __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plResult);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ErrorMsg )( 
            IUDataSource __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Close )( 
            IUDataSource __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Open )( 
            IUDataSource __RPC_FAR * This,
            /* [in] */ BSTR bstrConnection,
            /* [defaultvalue][in] */ long lHint);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProperty )( 
            IUDataSource __RPC_FAR * This,
            /* [in] */ long lPropID,
            /* [defaultvalue][in] */ BSTR bstrPropSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPropFlags )( 
            IUDataSource __RPC_FAR * This,
            /* [in] */ long lPropID,
            /* [defaultvalue][in] */ BSTR bstrPropSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenFromHandle )( 
            IUDataSource __RPC_FAR * This,
            /* [in] */ long lHandle);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Handle )( 
            IUDataSource __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Flags )( 
            IUDataSource __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Property )( 
            IUDataSource __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ParentHandle )( 
            IUDataSource __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ParentHandle )( 
            IUDataSource __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetProperty )( 
            IUDataSource __RPC_FAR * This,
            /* [in] */ VARIANT vtValue,
            /* [in] */ long lPropID,
            /* [defaultvalue][in] */ BSTR bstrPropSet);
        
        END_INTERFACE
    } IUDataSourceVtbl;

    interface IUDataSource
    {
        CONST_VTBL struct IUDataSourceVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUDataSource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUDataSource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUDataSource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUDataSource_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IUDataSource_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IUDataSource_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IUDataSource_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IUDataSource_AttachSocket(This,pIUnknownToUSocket)	\
    (This)->lpVtbl -> AttachSocket(This,pIUnknownToUSocket)

#define IUDataSource_get_Rtn(This,plResult)	\
    (This)->lpVtbl -> get_Rtn(This,plResult)

#define IUDataSource_get_ErrorMsg(This,pVal)	\
    (This)->lpVtbl -> get_ErrorMsg(This,pVal)


#define IUDataSource_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IUDataSource_Open(This,bstrConnection,lHint)	\
    (This)->lpVtbl -> Open(This,bstrConnection,lHint)

#define IUDataSource_GetProperty(This,lPropID,bstrPropSet)	\
    (This)->lpVtbl -> GetProperty(This,lPropID,bstrPropSet)

#define IUDataSource_GetPropFlags(This,lPropID,bstrPropSet)	\
    (This)->lpVtbl -> GetPropFlags(This,lPropID,bstrPropSet)

#define IUDataSource_OpenFromHandle(This,lHandle)	\
    (This)->lpVtbl -> OpenFromHandle(This,lHandle)

#define IUDataSource_get_Handle(This,pVal)	\
    (This)->lpVtbl -> get_Handle(This,pVal)

#define IUDataSource_get_Flags(This,pVal)	\
    (This)->lpVtbl -> get_Flags(This,pVal)

#define IUDataSource_get_Property(This,pVal)	\
    (This)->lpVtbl -> get_Property(This,pVal)

#define IUDataSource_get_ParentHandle(This,pVal)	\
    (This)->lpVtbl -> get_ParentHandle(This,pVal)

#define IUDataSource_put_ParentHandle(This,newVal)	\
    (This)->lpVtbl -> put_ParentHandle(This,newVal)

#define IUDataSource_SetProperty(This,vtValue,lPropID,bstrPropSet)	\
    (This)->lpVtbl -> SetProperty(This,vtValue,lPropID,bstrPropSet)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUDataSource_Close_Proxy( 
    IUDataSource __RPC_FAR * This);


void __RPC_STUB IUDataSource_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUDataSource_Open_Proxy( 
    IUDataSource __RPC_FAR * This,
    /* [in] */ BSTR bstrConnection,
    /* [defaultvalue][in] */ long lHint);


void __RPC_STUB IUDataSource_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUDataSource_GetProperty_Proxy( 
    IUDataSource __RPC_FAR * This,
    /* [in] */ long lPropID,
    /* [defaultvalue][in] */ BSTR bstrPropSet);


void __RPC_STUB IUDataSource_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUDataSource_GetPropFlags_Proxy( 
    IUDataSource __RPC_FAR * This,
    /* [in] */ long lPropID,
    /* [defaultvalue][in] */ BSTR bstrPropSet);


void __RPC_STUB IUDataSource_GetPropFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUDataSource_OpenFromHandle_Proxy( 
    IUDataSource __RPC_FAR * This,
    /* [in] */ long lHandle);


void __RPC_STUB IUDataSource_OpenFromHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUDataSource_get_Handle_Proxy( 
    IUDataSource __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IUDataSource_get_Handle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUDataSource_get_Flags_Proxy( 
    IUDataSource __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IUDataSource_get_Flags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUDataSource_get_Property_Proxy( 
    IUDataSource __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *pVal);


void __RPC_STUB IUDataSource_get_Property_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUDataSource_get_ParentHandle_Proxy( 
    IUDataSource __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IUDataSource_get_ParentHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IUDataSource_put_ParentHandle_Proxy( 
    IUDataSource __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IUDataSource_put_ParentHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUDataSource_SetProperty_Proxy( 
    IUDataSource __RPC_FAR * This,
    /* [in] */ VARIANT vtValue,
    /* [in] */ long lPropID,
    /* [defaultvalue][in] */ BSTR bstrPropSet);


void __RPC_STUB IUDataSource_SetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IUDataSource_INTERFACE_DEFINED__ */


#ifndef __IUSession_INTERFACE_DEFINED__
#define __IUSession_INTERFACE_DEFINED__

/* interface IUSession */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IUSession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0356C85C-18DC-497A-8142-439CC9CEE96E")
    IUSession : public IUObjBase
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Open( 
            /* [defaultvalue][in] */ BSTR bstrData = L"",
            /* [defaultvalue][in] */ long lHint = shDefault) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Commit( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Rollback( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE BeginTrans( 
            /* [defaultvalue][in] */ long lISolationLevel = ilReadCommitted) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetProperty( 
            /* [in] */ VARIANT vtValue,
            /* [defaultvalue][in] */ long lPropID = 0xbeL,
            /* [defaultvalue][in] */ BSTR bstrPropSet = L"{c8b522c6-5cf3-11ce-ade5-00aa0044773d}") = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetProperty( 
            /* [defaultvalue][in] */ long lPropID = 0xbeL,
            /* [defaultvalue][in] */ BSTR bstrPropSet = L"{c8b522c6-5cf3-11ce-ade5-00aa0044773d}") = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OpenFromHandle( 
            /* [in] */ long lHandle) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Handle( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Property( 
            /* [retval][out] */ VARIANT __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ParentHandle( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ParentHandle( 
            /* [in] */ long newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUSessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IUSession __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IUSession __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IUSession __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IUSession __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IUSession __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IUSession __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IUSession __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AttachSocket )( 
            IUSession __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pIUnknownToUSocket);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Rtn )( 
            IUSession __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plResult);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ErrorMsg )( 
            IUSession __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Close )( 
            IUSession __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Open )( 
            IUSession __RPC_FAR * This,
            /* [defaultvalue][in] */ BSTR bstrData,
            /* [defaultvalue][in] */ long lHint);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Commit )( 
            IUSession __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Rollback )( 
            IUSession __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginTrans )( 
            IUSession __RPC_FAR * This,
            /* [defaultvalue][in] */ long lISolationLevel);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetProperty )( 
            IUSession __RPC_FAR * This,
            /* [in] */ VARIANT vtValue,
            /* [defaultvalue][in] */ long lPropID,
            /* [defaultvalue][in] */ BSTR bstrPropSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProperty )( 
            IUSession __RPC_FAR * This,
            /* [defaultvalue][in] */ long lPropID,
            /* [defaultvalue][in] */ BSTR bstrPropSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenFromHandle )( 
            IUSession __RPC_FAR * This,
            /* [in] */ long lHandle);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Handle )( 
            IUSession __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Property )( 
            IUSession __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ParentHandle )( 
            IUSession __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ParentHandle )( 
            IUSession __RPC_FAR * This,
            /* [in] */ long newVal);
        
        END_INTERFACE
    } IUSessionVtbl;

    interface IUSession
    {
        CONST_VTBL struct IUSessionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUSession_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUSession_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUSession_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUSession_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IUSession_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IUSession_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IUSession_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IUSession_AttachSocket(This,pIUnknownToUSocket)	\
    (This)->lpVtbl -> AttachSocket(This,pIUnknownToUSocket)

#define IUSession_get_Rtn(This,plResult)	\
    (This)->lpVtbl -> get_Rtn(This,plResult)

#define IUSession_get_ErrorMsg(This,pVal)	\
    (This)->lpVtbl -> get_ErrorMsg(This,pVal)


#define IUSession_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IUSession_Open(This,bstrData,lHint)	\
    (This)->lpVtbl -> Open(This,bstrData,lHint)

#define IUSession_Commit(This)	\
    (This)->lpVtbl -> Commit(This)

#define IUSession_Rollback(This)	\
    (This)->lpVtbl -> Rollback(This)

#define IUSession_BeginTrans(This,lISolationLevel)	\
    (This)->lpVtbl -> BeginTrans(This,lISolationLevel)

#define IUSession_SetProperty(This,vtValue,lPropID,bstrPropSet)	\
    (This)->lpVtbl -> SetProperty(This,vtValue,lPropID,bstrPropSet)

#define IUSession_GetProperty(This,lPropID,bstrPropSet)	\
    (This)->lpVtbl -> GetProperty(This,lPropID,bstrPropSet)

#define IUSession_OpenFromHandle(This,lHandle)	\
    (This)->lpVtbl -> OpenFromHandle(This,lHandle)

#define IUSession_get_Handle(This,pVal)	\
    (This)->lpVtbl -> get_Handle(This,pVal)

#define IUSession_get_Property(This,pVal)	\
    (This)->lpVtbl -> get_Property(This,pVal)

#define IUSession_get_ParentHandle(This,pVal)	\
    (This)->lpVtbl -> get_ParentHandle(This,pVal)

#define IUSession_put_ParentHandle(This,newVal)	\
    (This)->lpVtbl -> put_ParentHandle(This,newVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUSession_Close_Proxy( 
    IUSession __RPC_FAR * This);


void __RPC_STUB IUSession_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUSession_Open_Proxy( 
    IUSession __RPC_FAR * This,
    /* [defaultvalue][in] */ BSTR bstrData,
    /* [defaultvalue][in] */ long lHint);


void __RPC_STUB IUSession_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUSession_Commit_Proxy( 
    IUSession __RPC_FAR * This);


void __RPC_STUB IUSession_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUSession_Rollback_Proxy( 
    IUSession __RPC_FAR * This);


void __RPC_STUB IUSession_Rollback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUSession_BeginTrans_Proxy( 
    IUSession __RPC_FAR * This,
    /* [defaultvalue][in] */ long lISolationLevel);


void __RPC_STUB IUSession_BeginTrans_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUSession_SetProperty_Proxy( 
    IUSession __RPC_FAR * This,
    /* [in] */ VARIANT vtValue,
    /* [defaultvalue][in] */ long lPropID,
    /* [defaultvalue][in] */ BSTR bstrPropSet);


void __RPC_STUB IUSession_SetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUSession_GetProperty_Proxy( 
    IUSession __RPC_FAR * This,
    /* [defaultvalue][in] */ long lPropID,
    /* [defaultvalue][in] */ BSTR bstrPropSet);


void __RPC_STUB IUSession_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUSession_OpenFromHandle_Proxy( 
    IUSession __RPC_FAR * This,
    /* [in] */ long lHandle);


void __RPC_STUB IUSession_OpenFromHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUSession_get_Handle_Proxy( 
    IUSession __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IUSession_get_Handle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUSession_get_Property_Proxy( 
    IUSession __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *pVal);


void __RPC_STUB IUSession_get_Property_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUSession_get_ParentHandle_Proxy( 
    IUSession __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IUSession_get_ParentHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IUSession_put_ParentHandle_Proxy( 
    IUSession __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IUSession_put_ParentHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IUSession_INTERFACE_DEFINED__ */


#ifndef __IUCommand_INTERFACE_DEFINED__
#define __IUCommand_INTERFACE_DEFINED__

/* interface IUCommand */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IUCommand;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9CE3E546-BE1A-4E0B-8B66-500107FFF69F")
    IUCommand : public IUObjBase
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Open( 
            /* [defaultvalue][in] */ BSTR bstrStatement = L"",
            /* [defaultvalue][in] */ long lHint = chDefault) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetProperty( 
            /* [in] */ long lPropID,
            /* [in] */ VARIANT vtValue,
            /* [defaultvalue][in] */ BSTR bstrPropSet = L"{c8b522be-5cf3-11ce-ade5-00aa0044773d}") = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetProperty( 
            /* [in] */ long lPropID,
            /* [defaultvalue][in] */ BSTR bstrPropSet = L"{c8b522be-5cf3-11ce-ade5-00aa0044773d}") = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ExecuteSQL( 
            /* [in] */ BSTR bstrSQL,
            /* [in] */ short sCreatedObject,
            /* [defaultvalue][in] */ short sCursorType = ctStatic,
            /* [defaultvalue][in] */ long lHint = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Prepare( 
            /* [defaultvalue][in] */ long lExpectedRuns = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Unprepare( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DoBatch( 
            /* [defaultvalue][in] */ short sCreatedObject = coNothing,
            /* [defaultvalue][in] */ short sCursorType = ctStatic,
            /* [defaultvalue][in] */ long lHint = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OpenFromHandle( 
            /* [in] */ long lHandle) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetOutputParams( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddParamInfo( 
            /* [in] */ short nDBType,
            /* [defaultvalue][in] */ long nDBParamIO = sdParamInput,
            /* [defaultvalue][in] */ long lLen = 0,
            /* [defaultvalue][in] */ BSTR bstrPName = L"",
            /* [defaultvalue][in] */ unsigned char bPrecision = 0,
            /* [defaultvalue][in] */ unsigned char bScale = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetParamInfo( 
            /* [in] */ long lIndex,
            /* [defaultvalue][out] */ short __RPC_FAR *pnDBType = 0,
            /* [defaultvalue][out] */ long __RPC_FAR *plDBParamIO = 0,
            /* [defaultvalue][out] */ long __RPC_FAR *plLen = 0,
            /* [defaultvalue][out] */ BSTR __RPC_FAR *pbstrPName = 0,
            /* [defaultvalue][out] */ unsigned char __RPC_FAR *pbPrecision = 0,
            /* [defaultvalue][out] */ unsigned char __RPC_FAR *pbScale = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCountParamInfos( 
            /* [retval][out] */ long __RPC_FAR *plCount) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCountParamData( 
            /* [retval][out] */ long __RPC_FAR *plCount) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AppendParamData( 
            /* [in] */ VARIANT vtParamData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CleanParamData( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetOutputData( 
            /* [in] */ long nIndex,
            /* [retval][out] */ VARIANT __RPC_FAR *pvtData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CleanParamInfo( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE UseStorageObjectForBLOB( 
            /* [defaultvalue][in] */ VARIANT_BOOL bStorageObjectForBLOB = -1) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ShrinkMemory( 
            /* [in] */ long lNewSize) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCountOutputData( 
            /* [retval][out] */ long __RPC_FAR *plCount) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ReleaseCreatedObject( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ParentHandle( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ParentHandle( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Handle( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CreatedObjectHandle( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Property( 
            /* [retval][out] */ VARIANT __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AffectedRows( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUCommandVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IUCommand __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IUCommand __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IUCommand __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IUCommand __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IUCommand __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IUCommand __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IUCommand __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AttachSocket )( 
            IUCommand __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pIUnknownToUSocket);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Rtn )( 
            IUCommand __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plResult);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ErrorMsg )( 
            IUCommand __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Close )( 
            IUCommand __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Open )( 
            IUCommand __RPC_FAR * This,
            /* [defaultvalue][in] */ BSTR bstrStatement,
            /* [defaultvalue][in] */ long lHint);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetProperty )( 
            IUCommand __RPC_FAR * This,
            /* [in] */ long lPropID,
            /* [in] */ VARIANT vtValue,
            /* [defaultvalue][in] */ BSTR bstrPropSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProperty )( 
            IUCommand __RPC_FAR * This,
            /* [in] */ long lPropID,
            /* [defaultvalue][in] */ BSTR bstrPropSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Cancel )( 
            IUCommand __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExecuteSQL )( 
            IUCommand __RPC_FAR * This,
            /* [in] */ BSTR bstrSQL,
            /* [in] */ short sCreatedObject,
            /* [defaultvalue][in] */ short sCursorType,
            /* [defaultvalue][in] */ long lHint);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Prepare )( 
            IUCommand __RPC_FAR * This,
            /* [defaultvalue][in] */ long lExpectedRuns);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Unprepare )( 
            IUCommand __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DoBatch )( 
            IUCommand __RPC_FAR * This,
            /* [defaultvalue][in] */ short sCreatedObject,
            /* [defaultvalue][in] */ short sCursorType,
            /* [defaultvalue][in] */ long lHint);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenFromHandle )( 
            IUCommand __RPC_FAR * This,
            /* [in] */ long lHandle);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOutputParams )( 
            IUCommand __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddParamInfo )( 
            IUCommand __RPC_FAR * This,
            /* [in] */ short nDBType,
            /* [defaultvalue][in] */ long nDBParamIO,
            /* [defaultvalue][in] */ long lLen,
            /* [defaultvalue][in] */ BSTR bstrPName,
            /* [defaultvalue][in] */ unsigned char bPrecision,
            /* [defaultvalue][in] */ unsigned char bScale);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetParamInfo )( 
            IUCommand __RPC_FAR * This,
            /* [in] */ long lIndex,
            /* [defaultvalue][out] */ short __RPC_FAR *pnDBType,
            /* [defaultvalue][out] */ long __RPC_FAR *plDBParamIO,
            /* [defaultvalue][out] */ long __RPC_FAR *plLen,
            /* [defaultvalue][out] */ BSTR __RPC_FAR *pbstrPName,
            /* [defaultvalue][out] */ unsigned char __RPC_FAR *pbPrecision,
            /* [defaultvalue][out] */ unsigned char __RPC_FAR *pbScale);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCountParamInfos )( 
            IUCommand __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plCount);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCountParamData )( 
            IUCommand __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plCount);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AppendParamData )( 
            IUCommand __RPC_FAR * This,
            /* [in] */ VARIANT vtParamData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CleanParamData )( 
            IUCommand __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOutputData )( 
            IUCommand __RPC_FAR * This,
            /* [in] */ long nIndex,
            /* [retval][out] */ VARIANT __RPC_FAR *pvtData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CleanParamInfo )( 
            IUCommand __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UseStorageObjectForBLOB )( 
            IUCommand __RPC_FAR * This,
            /* [defaultvalue][in] */ VARIANT_BOOL bStorageObjectForBLOB);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShrinkMemory )( 
            IUCommand __RPC_FAR * This,
            /* [in] */ long lNewSize);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCountOutputData )( 
            IUCommand __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plCount);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseCreatedObject )( 
            IUCommand __RPC_FAR * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ParentHandle )( 
            IUCommand __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ParentHandle )( 
            IUCommand __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Handle )( 
            IUCommand __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CreatedObjectHandle )( 
            IUCommand __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Property )( 
            IUCommand __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AffectedRows )( 
            IUCommand __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        END_INTERFACE
    } IUCommandVtbl;

    interface IUCommand
    {
        CONST_VTBL struct IUCommandVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUCommand_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUCommand_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUCommand_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUCommand_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IUCommand_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IUCommand_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IUCommand_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IUCommand_AttachSocket(This,pIUnknownToUSocket)	\
    (This)->lpVtbl -> AttachSocket(This,pIUnknownToUSocket)

#define IUCommand_get_Rtn(This,plResult)	\
    (This)->lpVtbl -> get_Rtn(This,plResult)

#define IUCommand_get_ErrorMsg(This,pVal)	\
    (This)->lpVtbl -> get_ErrorMsg(This,pVal)


#define IUCommand_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IUCommand_Open(This,bstrStatement,lHint)	\
    (This)->lpVtbl -> Open(This,bstrStatement,lHint)

#define IUCommand_SetProperty(This,lPropID,vtValue,bstrPropSet)	\
    (This)->lpVtbl -> SetProperty(This,lPropID,vtValue,bstrPropSet)

#define IUCommand_GetProperty(This,lPropID,bstrPropSet)	\
    (This)->lpVtbl -> GetProperty(This,lPropID,bstrPropSet)

#define IUCommand_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)

#define IUCommand_ExecuteSQL(This,bstrSQL,sCreatedObject,sCursorType,lHint)	\
    (This)->lpVtbl -> ExecuteSQL(This,bstrSQL,sCreatedObject,sCursorType,lHint)

#define IUCommand_Prepare(This,lExpectedRuns)	\
    (This)->lpVtbl -> Prepare(This,lExpectedRuns)

#define IUCommand_Unprepare(This)	\
    (This)->lpVtbl -> Unprepare(This)

#define IUCommand_DoBatch(This,sCreatedObject,sCursorType,lHint)	\
    (This)->lpVtbl -> DoBatch(This,sCreatedObject,sCursorType,lHint)

#define IUCommand_OpenFromHandle(This,lHandle)	\
    (This)->lpVtbl -> OpenFromHandle(This,lHandle)

#define IUCommand_GetOutputParams(This)	\
    (This)->lpVtbl -> GetOutputParams(This)

#define IUCommand_AddParamInfo(This,nDBType,nDBParamIO,lLen,bstrPName,bPrecision,bScale)	\
    (This)->lpVtbl -> AddParamInfo(This,nDBType,nDBParamIO,lLen,bstrPName,bPrecision,bScale)

#define IUCommand_GetParamInfo(This,lIndex,pnDBType,plDBParamIO,plLen,pbstrPName,pbPrecision,pbScale)	\
    (This)->lpVtbl -> GetParamInfo(This,lIndex,pnDBType,plDBParamIO,plLen,pbstrPName,pbPrecision,pbScale)

#define IUCommand_GetCountParamInfos(This,plCount)	\
    (This)->lpVtbl -> GetCountParamInfos(This,plCount)

#define IUCommand_GetCountParamData(This,plCount)	\
    (This)->lpVtbl -> GetCountParamData(This,plCount)

#define IUCommand_AppendParamData(This,vtParamData)	\
    (This)->lpVtbl -> AppendParamData(This,vtParamData)

#define IUCommand_CleanParamData(This)	\
    (This)->lpVtbl -> CleanParamData(This)

#define IUCommand_GetOutputData(This,nIndex,pvtData)	\
    (This)->lpVtbl -> GetOutputData(This,nIndex,pvtData)

#define IUCommand_CleanParamInfo(This)	\
    (This)->lpVtbl -> CleanParamInfo(This)

#define IUCommand_UseStorageObjectForBLOB(This,bStorageObjectForBLOB)	\
    (This)->lpVtbl -> UseStorageObjectForBLOB(This,bStorageObjectForBLOB)

#define IUCommand_ShrinkMemory(This,lNewSize)	\
    (This)->lpVtbl -> ShrinkMemory(This,lNewSize)

#define IUCommand_GetCountOutputData(This,plCount)	\
    (This)->lpVtbl -> GetCountOutputData(This,plCount)

#define IUCommand_ReleaseCreatedObject(This)	\
    (This)->lpVtbl -> ReleaseCreatedObject(This)

#define IUCommand_get_ParentHandle(This,pVal)	\
    (This)->lpVtbl -> get_ParentHandle(This,pVal)

#define IUCommand_put_ParentHandle(This,newVal)	\
    (This)->lpVtbl -> put_ParentHandle(This,newVal)

#define IUCommand_get_Handle(This,pVal)	\
    (This)->lpVtbl -> get_Handle(This,pVal)

#define IUCommand_get_CreatedObjectHandle(This,pVal)	\
    (This)->lpVtbl -> get_CreatedObjectHandle(This,pVal)

#define IUCommand_get_Property(This,pVal)	\
    (This)->lpVtbl -> get_Property(This,pVal)

#define IUCommand_get_AffectedRows(This,pVal)	\
    (This)->lpVtbl -> get_AffectedRows(This,pVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_Close_Proxy( 
    IUCommand __RPC_FAR * This);


void __RPC_STUB IUCommand_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_Open_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [defaultvalue][in] */ BSTR bstrStatement,
    /* [defaultvalue][in] */ long lHint);


void __RPC_STUB IUCommand_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_SetProperty_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [in] */ long lPropID,
    /* [in] */ VARIANT vtValue,
    /* [defaultvalue][in] */ BSTR bstrPropSet);


void __RPC_STUB IUCommand_SetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_GetProperty_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [in] */ long lPropID,
    /* [defaultvalue][in] */ BSTR bstrPropSet);


void __RPC_STUB IUCommand_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_Cancel_Proxy( 
    IUCommand __RPC_FAR * This);


void __RPC_STUB IUCommand_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_ExecuteSQL_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [in] */ BSTR bstrSQL,
    /* [in] */ short sCreatedObject,
    /* [defaultvalue][in] */ short sCursorType,
    /* [defaultvalue][in] */ long lHint);


void __RPC_STUB IUCommand_ExecuteSQL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_Prepare_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [defaultvalue][in] */ long lExpectedRuns);


void __RPC_STUB IUCommand_Prepare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_Unprepare_Proxy( 
    IUCommand __RPC_FAR * This);


void __RPC_STUB IUCommand_Unprepare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_DoBatch_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [defaultvalue][in] */ short sCreatedObject,
    /* [defaultvalue][in] */ short sCursorType,
    /* [defaultvalue][in] */ long lHint);


void __RPC_STUB IUCommand_DoBatch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_OpenFromHandle_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [in] */ long lHandle);


void __RPC_STUB IUCommand_OpenFromHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_GetOutputParams_Proxy( 
    IUCommand __RPC_FAR * This);


void __RPC_STUB IUCommand_GetOutputParams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_AddParamInfo_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [in] */ short nDBType,
    /* [defaultvalue][in] */ long nDBParamIO,
    /* [defaultvalue][in] */ long lLen,
    /* [defaultvalue][in] */ BSTR bstrPName,
    /* [defaultvalue][in] */ unsigned char bPrecision,
    /* [defaultvalue][in] */ unsigned char bScale);


void __RPC_STUB IUCommand_AddParamInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_GetParamInfo_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [in] */ long lIndex,
    /* [defaultvalue][out] */ short __RPC_FAR *pnDBType,
    /* [defaultvalue][out] */ long __RPC_FAR *plDBParamIO,
    /* [defaultvalue][out] */ long __RPC_FAR *plLen,
    /* [defaultvalue][out] */ BSTR __RPC_FAR *pbstrPName,
    /* [defaultvalue][out] */ unsigned char __RPC_FAR *pbPrecision,
    /* [defaultvalue][out] */ unsigned char __RPC_FAR *pbScale);


void __RPC_STUB IUCommand_GetParamInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_GetCountParamInfos_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *plCount);


void __RPC_STUB IUCommand_GetCountParamInfos_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_GetCountParamData_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *plCount);


void __RPC_STUB IUCommand_GetCountParamData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_AppendParamData_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [in] */ VARIANT vtParamData);


void __RPC_STUB IUCommand_AppendParamData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_CleanParamData_Proxy( 
    IUCommand __RPC_FAR * This);


void __RPC_STUB IUCommand_CleanParamData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_GetOutputData_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [in] */ long nIndex,
    /* [retval][out] */ VARIANT __RPC_FAR *pvtData);


void __RPC_STUB IUCommand_GetOutputData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_CleanParamInfo_Proxy( 
    IUCommand __RPC_FAR * This);


void __RPC_STUB IUCommand_CleanParamInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_UseStorageObjectForBLOB_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [defaultvalue][in] */ VARIANT_BOOL bStorageObjectForBLOB);


void __RPC_STUB IUCommand_UseStorageObjectForBLOB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_ShrinkMemory_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [in] */ long lNewSize);


void __RPC_STUB IUCommand_ShrinkMemory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_GetCountOutputData_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *plCount);


void __RPC_STUB IUCommand_GetCountOutputData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUCommand_ReleaseCreatedObject_Proxy( 
    IUCommand __RPC_FAR * This);


void __RPC_STUB IUCommand_ReleaseCreatedObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUCommand_get_ParentHandle_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IUCommand_get_ParentHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IUCommand_put_ParentHandle_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IUCommand_put_ParentHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUCommand_get_Handle_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IUCommand_get_Handle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUCommand_get_CreatedObjectHandle_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IUCommand_get_CreatedObjectHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUCommand_get_Property_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *pVal);


void __RPC_STUB IUCommand_get_Property_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUCommand_get_AffectedRows_Proxy( 
    IUCommand __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IUCommand_get_AffectedRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IUCommand_INTERFACE_DEFINED__ */


#ifndef __IURowset_INTERFACE_DEFINED__
#define __IURowset_INTERFACE_DEFINED__

/* interface IURowset */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IURowset;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("94A4D811-5121-434F-9861-66FE4099DC2A")
    IURowset : public IUObjBase
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Open( 
            /* [defaultvalue][in] */ BSTR bstrTableName = L"",
            /* [defaultvalue][in] */ short sCursorType = ctStatic,
            /* [defaultvalue][in] */ long lHint = 0,
            /* [defaultvalue][in] */ short sBatchSize = 0,
            /* [defaultvalue][in] */ VARIANT_BOOL bNoDelayForBLOB = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE MoveFirst( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE MoveLast( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE MovePrev( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE MoveNext( 
            /* [defaultvalue][in] */ long lSkip = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Delete( 
            /* [defaultvalue][in] */ short sRow = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Add( 
            /* [defaultvalue][in] */ VARIANT_BOOL bNeedNewRecord = 0,
            /* [defaultvalue][in] */ short sRow = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Update( 
            /* [defaultvalue][in] */ short sRow = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE UpdateBatch( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OpenFromHandle( 
            /* [in] */ long lHandle,
            /* [defaultvalue][in] */ VARIANT_BOOL bKeepPrevRowset = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Bookmark( 
            /* [in] */ short sRow) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AsynFetch( 
            /* [defaultvalue][in] */ VARIANT_BOOL bFromBeginning = -1,
            /* [defaultvalue][in] */ short sSubBatchSize = 0,
            /* [defaultvalue][in] */ long lRows = -1) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetBatchRecords( 
            /* [defaultvalue][in] */ short sSubBatchSize = 0,
            /* [defaultvalue][in] */ VARIANT_BOOL bFirst = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetProviders( 
            /* [defaultvalue][in] */ VARIANT_BOOL bKeepPrevRowset = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Undo( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetSchemaRowset( 
            /* [in] */ BSTR bstrSchemaGUID,
            /* [in] */ VARIANT vtRestrictions,
            /* [defaultvalue][in] */ short nBatchSize = 0,
            /* [defaultvalue][in] */ VARIANT_BOOL bKeepPrevRowset = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetProperty( 
            /* [in] */ long lPropID,
            /* [defaultvalue][in] */ BSTR bstrPropSet = L"{c8b522be-5cf3-11ce-ade5-00aa0044773d}") = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetDataType( 
            /* [in] */ long lCol,
            /* [in] */ short nDBType,
            /* [defaultvalue][in] */ long lLen = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetRowsAt( 
            /* [in] */ VARIANT vtBookmarkValue,
            /* [defaultvalue][in] */ long lRowsOffset = 0,
            /* [defaultvalue][in] */ short sSubBatchSize = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE UseStorageObjectForBLOB( 
            /* [defaultvalue][in] */ VARIANT_BOOL bUseStorageObject = -1) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetData( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [retval][out] */ VARIANT __RPC_FAR *pvtData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsBLOB( 
            /* [in] */ long lCol,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbBLOB) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetColName( 
            /* [in] */ long lCol,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrColName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetDataType( 
            /* [in] */ long lCol,
            /* [retval][out] */ short __RPC_FAR *psDBType) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetStatus( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [retval][out] */ long __RPC_FAR *plStatus) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FindColOrdinal( 
            /* [in] */ BSTR bstrColName,
            /* [defaultvalue][in] */ VARIANT_BOOL bCaseSensitive,
            /* [retval][out] */ long __RPC_FAR *plCol) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetData( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [in] */ VARIANT vtData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsEOF( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbIsEOF) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetRowsFetched( 
            /* [retval][out] */ short __RPC_FAR *psCount) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCols( 
            /* [retval][out] */ long __RPC_FAR *plCols) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsModified( 
            /* [in] */ short sRow,
            /* [in] */ long lCol,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbModified) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetMaxLen( 
            /* [in] */ long lCol,
            /* [retval][out] */ long __RPC_FAR *plMaxLen) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsWritable( 
            /* [in] */ long lCol,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbWritable) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetRawDataType( 
            /* [in] */ long lCol,
            /* [retval][out] */ short __RPC_FAR *psDataType) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetDataSize( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [retval][out] */ long __RPC_FAR *plSize) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetBatchSize( 
            /* [retval][out] */ short __RPC_FAR *psBatchSize) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetColFlags( 
            /* [in] */ long lCol,
            /* [retval][out] */ long __RPC_FAR *plFlags) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetRawMaxColLen( 
            /* [in] */ long lCol,
            /* [retval][out] */ long __RPC_FAR *plLen) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ShrinkMemory( 
            /* [in] */ long lNewSize) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsNullable( 
            /* [in] */ long lCol,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbIsNullable) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE BLOBDelayed( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbDelayed) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsDelayedUpdate( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbDelayedUpdate) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsReadOnly( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbIsReadOnly) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCountOfBLOBsPerRow( 
            /* [retval][out] */ long __RPC_FAR *plCountBLOBs) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ParentHandle( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ParentHandle( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Handle( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Property( 
            /* [retval][out] */ VARIANT __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BookmarkValue( 
            /* [retval][out] */ VARIANT __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetBatchRecordsEx( 
            /* [defaultvalue][in] */ short sSubBatchSize = 0,
            /* [defaultvalue][in] */ long lSkip = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetBatchRecordsLast( 
            /* [defaultvalue][in] */ short sSubBatchSize = 0) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IURowsetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IURowset __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IURowset __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IURowset __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IURowset __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IURowset __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IURowset __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IURowset __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AttachSocket )( 
            IURowset __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pIUnknownToUSocket);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Rtn )( 
            IURowset __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plResult);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ErrorMsg )( 
            IURowset __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Open )( 
            IURowset __RPC_FAR * This,
            /* [defaultvalue][in] */ BSTR bstrTableName,
            /* [defaultvalue][in] */ short sCursorType,
            /* [defaultvalue][in] */ long lHint,
            /* [defaultvalue][in] */ short sBatchSize,
            /* [defaultvalue][in] */ VARIANT_BOOL bNoDelayForBLOB);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Close )( 
            IURowset __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveFirst )( 
            IURowset __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveLast )( 
            IURowset __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MovePrev )( 
            IURowset __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveNext )( 
            IURowset __RPC_FAR * This,
            /* [defaultvalue][in] */ long lSkip);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Delete )( 
            IURowset __RPC_FAR * This,
            /* [defaultvalue][in] */ short sRow);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Add )( 
            IURowset __RPC_FAR * This,
            /* [defaultvalue][in] */ VARIANT_BOOL bNeedNewRecord,
            /* [defaultvalue][in] */ short sRow);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Update )( 
            IURowset __RPC_FAR * This,
            /* [defaultvalue][in] */ short sRow);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UpdateBatch )( 
            IURowset __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenFromHandle )( 
            IURowset __RPC_FAR * This,
            /* [in] */ long lHandle,
            /* [defaultvalue][in] */ VARIANT_BOOL bKeepPrevRowset);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Bookmark )( 
            IURowset __RPC_FAR * This,
            /* [in] */ short sRow);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AsynFetch )( 
            IURowset __RPC_FAR * This,
            /* [defaultvalue][in] */ VARIANT_BOOL bFromBeginning,
            /* [defaultvalue][in] */ short sSubBatchSize,
            /* [defaultvalue][in] */ long lRows);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBatchRecords )( 
            IURowset __RPC_FAR * This,
            /* [defaultvalue][in] */ short sSubBatchSize,
            /* [defaultvalue][in] */ VARIANT_BOOL bFirst);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProviders )( 
            IURowset __RPC_FAR * This,
            /* [defaultvalue][in] */ VARIANT_BOOL bKeepPrevRowset);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Undo )( 
            IURowset __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSchemaRowset )( 
            IURowset __RPC_FAR * This,
            /* [in] */ BSTR bstrSchemaGUID,
            /* [in] */ VARIANT vtRestrictions,
            /* [defaultvalue][in] */ short nBatchSize,
            /* [defaultvalue][in] */ VARIANT_BOOL bKeepPrevRowset);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProperty )( 
            IURowset __RPC_FAR * This,
            /* [in] */ long lPropID,
            /* [defaultvalue][in] */ BSTR bstrPropSet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDataType )( 
            IURowset __RPC_FAR * This,
            /* [in] */ long lCol,
            /* [in] */ short nDBType,
            /* [defaultvalue][in] */ long lLen);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRowsAt )( 
            IURowset __RPC_FAR * This,
            /* [in] */ VARIANT vtBookmarkValue,
            /* [defaultvalue][in] */ long lRowsOffset,
            /* [defaultvalue][in] */ short sSubBatchSize);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UseStorageObjectForBLOB )( 
            IURowset __RPC_FAR * This,
            /* [defaultvalue][in] */ VARIANT_BOOL bUseStorageObject);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetData )( 
            IURowset __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [retval][out] */ VARIANT __RPC_FAR *pvtData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsBLOB )( 
            IURowset __RPC_FAR * This,
            /* [in] */ long lCol,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbBLOB);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetColName )( 
            IURowset __RPC_FAR * This,
            /* [in] */ long lCol,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrColName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDataType )( 
            IURowset __RPC_FAR * This,
            /* [in] */ long lCol,
            /* [retval][out] */ short __RPC_FAR *psDBType);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStatus )( 
            IURowset __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [retval][out] */ long __RPC_FAR *plStatus);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindColOrdinal )( 
            IURowset __RPC_FAR * This,
            /* [in] */ BSTR bstrColName,
            /* [defaultvalue][in] */ VARIANT_BOOL bCaseSensitive,
            /* [retval][out] */ long __RPC_FAR *plCol);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetData )( 
            IURowset __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [in] */ VARIANT vtData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsEOF )( 
            IURowset __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbIsEOF);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRowsFetched )( 
            IURowset __RPC_FAR * This,
            /* [retval][out] */ short __RPC_FAR *psCount);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCols )( 
            IURowset __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plCols);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsModified )( 
            IURowset __RPC_FAR * This,
            /* [in] */ short sRow,
            /* [in] */ long lCol,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbModified);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMaxLen )( 
            IURowset __RPC_FAR * This,
            /* [in] */ long lCol,
            /* [retval][out] */ long __RPC_FAR *plMaxLen);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsWritable )( 
            IURowset __RPC_FAR * This,
            /* [in] */ long lCol,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbWritable);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRawDataType )( 
            IURowset __RPC_FAR * This,
            /* [in] */ long lCol,
            /* [retval][out] */ short __RPC_FAR *psDataType);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDataSize )( 
            IURowset __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [retval][out] */ long __RPC_FAR *plSize);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBatchSize )( 
            IURowset __RPC_FAR * This,
            /* [retval][out] */ short __RPC_FAR *psBatchSize);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetColFlags )( 
            IURowset __RPC_FAR * This,
            /* [in] */ long lCol,
            /* [retval][out] */ long __RPC_FAR *plFlags);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRawMaxColLen )( 
            IURowset __RPC_FAR * This,
            /* [in] */ long lCol,
            /* [retval][out] */ long __RPC_FAR *plLen);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShrinkMemory )( 
            IURowset __RPC_FAR * This,
            /* [in] */ long lNewSize);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsNullable )( 
            IURowset __RPC_FAR * This,
            /* [in] */ long lCol,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbIsNullable);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BLOBDelayed )( 
            IURowset __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbDelayed);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsDelayedUpdate )( 
            IURowset __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbDelayedUpdate);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsReadOnly )( 
            IURowset __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbIsReadOnly);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCountOfBLOBsPerRow )( 
            IURowset __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plCountBLOBs);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ParentHandle )( 
            IURowset __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ParentHandle )( 
            IURowset __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Handle )( 
            IURowset __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Property )( 
            IURowset __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_BookmarkValue )( 
            IURowset __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBatchRecordsEx )( 
            IURowset __RPC_FAR * This,
            /* [defaultvalue][in] */ short sSubBatchSize,
            /* [defaultvalue][in] */ long lSkip);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBatchRecordsLast )( 
            IURowset __RPC_FAR * This,
            /* [defaultvalue][in] */ short sSubBatchSize);
        
        END_INTERFACE
    } IURowsetVtbl;

    interface IURowset
    {
        CONST_VTBL struct IURowsetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IURowset_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IURowset_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IURowset_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IURowset_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IURowset_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IURowset_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IURowset_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IURowset_AttachSocket(This,pIUnknownToUSocket)	\
    (This)->lpVtbl -> AttachSocket(This,pIUnknownToUSocket)

#define IURowset_get_Rtn(This,plResult)	\
    (This)->lpVtbl -> get_Rtn(This,plResult)

#define IURowset_get_ErrorMsg(This,pVal)	\
    (This)->lpVtbl -> get_ErrorMsg(This,pVal)


#define IURowset_Open(This,bstrTableName,sCursorType,lHint,sBatchSize,bNoDelayForBLOB)	\
    (This)->lpVtbl -> Open(This,bstrTableName,sCursorType,lHint,sBatchSize,bNoDelayForBLOB)

#define IURowset_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IURowset_MoveFirst(This)	\
    (This)->lpVtbl -> MoveFirst(This)

#define IURowset_MoveLast(This)	\
    (This)->lpVtbl -> MoveLast(This)

#define IURowset_MovePrev(This)	\
    (This)->lpVtbl -> MovePrev(This)

#define IURowset_MoveNext(This,lSkip)	\
    (This)->lpVtbl -> MoveNext(This,lSkip)

#define IURowset_Delete(This,sRow)	\
    (This)->lpVtbl -> Delete(This,sRow)

#define IURowset_Add(This,bNeedNewRecord,sRow)	\
    (This)->lpVtbl -> Add(This,bNeedNewRecord,sRow)

#define IURowset_Update(This,sRow)	\
    (This)->lpVtbl -> Update(This,sRow)

#define IURowset_UpdateBatch(This)	\
    (This)->lpVtbl -> UpdateBatch(This)

#define IURowset_OpenFromHandle(This,lHandle,bKeepPrevRowset)	\
    (This)->lpVtbl -> OpenFromHandle(This,lHandle,bKeepPrevRowset)

#define IURowset_Bookmark(This,sRow)	\
    (This)->lpVtbl -> Bookmark(This,sRow)

#define IURowset_AsynFetch(This,bFromBeginning,sSubBatchSize,lRows)	\
    (This)->lpVtbl -> AsynFetch(This,bFromBeginning,sSubBatchSize,lRows)

#define IURowset_GetBatchRecords(This,sSubBatchSize,bFirst)	\
    (This)->lpVtbl -> GetBatchRecords(This,sSubBatchSize,bFirst)

#define IURowset_GetProviders(This,bKeepPrevRowset)	\
    (This)->lpVtbl -> GetProviders(This,bKeepPrevRowset)

#define IURowset_Undo(This)	\
    (This)->lpVtbl -> Undo(This)

#define IURowset_GetSchemaRowset(This,bstrSchemaGUID,vtRestrictions,nBatchSize,bKeepPrevRowset)	\
    (This)->lpVtbl -> GetSchemaRowset(This,bstrSchemaGUID,vtRestrictions,nBatchSize,bKeepPrevRowset)

#define IURowset_GetProperty(This,lPropID,bstrPropSet)	\
    (This)->lpVtbl -> GetProperty(This,lPropID,bstrPropSet)

#define IURowset_SetDataType(This,lCol,nDBType,lLen)	\
    (This)->lpVtbl -> SetDataType(This,lCol,nDBType,lLen)

#define IURowset_GetRowsAt(This,vtBookmarkValue,lRowsOffset,sSubBatchSize)	\
    (This)->lpVtbl -> GetRowsAt(This,vtBookmarkValue,lRowsOffset,sSubBatchSize)

#define IURowset_UseStorageObjectForBLOB(This,bUseStorageObject)	\
    (This)->lpVtbl -> UseStorageObjectForBLOB(This,bUseStorageObject)

#define IURowset_GetData(This,nRow,lCol,pvtData)	\
    (This)->lpVtbl -> GetData(This,nRow,lCol,pvtData)

#define IURowset_IsBLOB(This,lCol,pbBLOB)	\
    (This)->lpVtbl -> IsBLOB(This,lCol,pbBLOB)

#define IURowset_GetColName(This,lCol,pbstrColName)	\
    (This)->lpVtbl -> GetColName(This,lCol,pbstrColName)

#define IURowset_GetDataType(This,lCol,psDBType)	\
    (This)->lpVtbl -> GetDataType(This,lCol,psDBType)

#define IURowset_GetStatus(This,nRow,lCol,plStatus)	\
    (This)->lpVtbl -> GetStatus(This,nRow,lCol,plStatus)

#define IURowset_FindColOrdinal(This,bstrColName,bCaseSensitive,plCol)	\
    (This)->lpVtbl -> FindColOrdinal(This,bstrColName,bCaseSensitive,plCol)

#define IURowset_SetData(This,nRow,lCol,vtData)	\
    (This)->lpVtbl -> SetData(This,nRow,lCol,vtData)

#define IURowset_IsEOF(This,pbIsEOF)	\
    (This)->lpVtbl -> IsEOF(This,pbIsEOF)

#define IURowset_GetRowsFetched(This,psCount)	\
    (This)->lpVtbl -> GetRowsFetched(This,psCount)

#define IURowset_GetCols(This,plCols)	\
    (This)->lpVtbl -> GetCols(This,plCols)

#define IURowset_IsModified(This,sRow,lCol,pbModified)	\
    (This)->lpVtbl -> IsModified(This,sRow,lCol,pbModified)

#define IURowset_GetMaxLen(This,lCol,plMaxLen)	\
    (This)->lpVtbl -> GetMaxLen(This,lCol,plMaxLen)

#define IURowset_IsWritable(This,lCol,pbWritable)	\
    (This)->lpVtbl -> IsWritable(This,lCol,pbWritable)

#define IURowset_GetRawDataType(This,lCol,psDataType)	\
    (This)->lpVtbl -> GetRawDataType(This,lCol,psDataType)

#define IURowset_GetDataSize(This,nRow,lCol,plSize)	\
    (This)->lpVtbl -> GetDataSize(This,nRow,lCol,plSize)

#define IURowset_GetBatchSize(This,psBatchSize)	\
    (This)->lpVtbl -> GetBatchSize(This,psBatchSize)

#define IURowset_GetColFlags(This,lCol,plFlags)	\
    (This)->lpVtbl -> GetColFlags(This,lCol,plFlags)

#define IURowset_GetRawMaxColLen(This,lCol,plLen)	\
    (This)->lpVtbl -> GetRawMaxColLen(This,lCol,plLen)

#define IURowset_ShrinkMemory(This,lNewSize)	\
    (This)->lpVtbl -> ShrinkMemory(This,lNewSize)

#define IURowset_IsNullable(This,lCol,pbIsNullable)	\
    (This)->lpVtbl -> IsNullable(This,lCol,pbIsNullable)

#define IURowset_BLOBDelayed(This,pbDelayed)	\
    (This)->lpVtbl -> BLOBDelayed(This,pbDelayed)

#define IURowset_IsDelayedUpdate(This,pbDelayedUpdate)	\
    (This)->lpVtbl -> IsDelayedUpdate(This,pbDelayedUpdate)

#define IURowset_IsReadOnly(This,pbIsReadOnly)	\
    (This)->lpVtbl -> IsReadOnly(This,pbIsReadOnly)

#define IURowset_GetCountOfBLOBsPerRow(This,plCountBLOBs)	\
    (This)->lpVtbl -> GetCountOfBLOBsPerRow(This,plCountBLOBs)

#define IURowset_get_ParentHandle(This,pVal)	\
    (This)->lpVtbl -> get_ParentHandle(This,pVal)

#define IURowset_put_ParentHandle(This,newVal)	\
    (This)->lpVtbl -> put_ParentHandle(This,newVal)

#define IURowset_get_Handle(This,pVal)	\
    (This)->lpVtbl -> get_Handle(This,pVal)

#define IURowset_get_Property(This,pVal)	\
    (This)->lpVtbl -> get_Property(This,pVal)

#define IURowset_get_BookmarkValue(This,pVal)	\
    (This)->lpVtbl -> get_BookmarkValue(This,pVal)

#define IURowset_GetBatchRecordsEx(This,sSubBatchSize,lSkip)	\
    (This)->lpVtbl -> GetBatchRecordsEx(This,sSubBatchSize,lSkip)

#define IURowset_GetBatchRecordsLast(This,sSubBatchSize)	\
    (This)->lpVtbl -> GetBatchRecordsLast(This,sSubBatchSize)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_Open_Proxy( 
    IURowset __RPC_FAR * This,
    /* [defaultvalue][in] */ BSTR bstrTableName,
    /* [defaultvalue][in] */ short sCursorType,
    /* [defaultvalue][in] */ long lHint,
    /* [defaultvalue][in] */ short sBatchSize,
    /* [defaultvalue][in] */ VARIANT_BOOL bNoDelayForBLOB);


void __RPC_STUB IURowset_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_Close_Proxy( 
    IURowset __RPC_FAR * This);


void __RPC_STUB IURowset_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_MoveFirst_Proxy( 
    IURowset __RPC_FAR * This);


void __RPC_STUB IURowset_MoveFirst_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_MoveLast_Proxy( 
    IURowset __RPC_FAR * This);


void __RPC_STUB IURowset_MoveLast_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_MovePrev_Proxy( 
    IURowset __RPC_FAR * This);


void __RPC_STUB IURowset_MovePrev_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_MoveNext_Proxy( 
    IURowset __RPC_FAR * This,
    /* [defaultvalue][in] */ long lSkip);


void __RPC_STUB IURowset_MoveNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_Delete_Proxy( 
    IURowset __RPC_FAR * This,
    /* [defaultvalue][in] */ short sRow);


void __RPC_STUB IURowset_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_Add_Proxy( 
    IURowset __RPC_FAR * This,
    /* [defaultvalue][in] */ VARIANT_BOOL bNeedNewRecord,
    /* [defaultvalue][in] */ short sRow);


void __RPC_STUB IURowset_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_Update_Proxy( 
    IURowset __RPC_FAR * This,
    /* [defaultvalue][in] */ short sRow);


void __RPC_STUB IURowset_Update_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_UpdateBatch_Proxy( 
    IURowset __RPC_FAR * This);


void __RPC_STUB IURowset_UpdateBatch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_OpenFromHandle_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ long lHandle,
    /* [defaultvalue][in] */ VARIANT_BOOL bKeepPrevRowset);


void __RPC_STUB IURowset_OpenFromHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_Bookmark_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ short sRow);


void __RPC_STUB IURowset_Bookmark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_AsynFetch_Proxy( 
    IURowset __RPC_FAR * This,
    /* [defaultvalue][in] */ VARIANT_BOOL bFromBeginning,
    /* [defaultvalue][in] */ short sSubBatchSize,
    /* [defaultvalue][in] */ long lRows);


void __RPC_STUB IURowset_AsynFetch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_GetBatchRecords_Proxy( 
    IURowset __RPC_FAR * This,
    /* [defaultvalue][in] */ short sSubBatchSize,
    /* [defaultvalue][in] */ VARIANT_BOOL bFirst);


void __RPC_STUB IURowset_GetBatchRecords_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_GetProviders_Proxy( 
    IURowset __RPC_FAR * This,
    /* [defaultvalue][in] */ VARIANT_BOOL bKeepPrevRowset);


void __RPC_STUB IURowset_GetProviders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_Undo_Proxy( 
    IURowset __RPC_FAR * This);


void __RPC_STUB IURowset_Undo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_GetSchemaRowset_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ BSTR bstrSchemaGUID,
    /* [in] */ VARIANT vtRestrictions,
    /* [defaultvalue][in] */ short nBatchSize,
    /* [defaultvalue][in] */ VARIANT_BOOL bKeepPrevRowset);


void __RPC_STUB IURowset_GetSchemaRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_GetProperty_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ long lPropID,
    /* [defaultvalue][in] */ BSTR bstrPropSet);


void __RPC_STUB IURowset_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_SetDataType_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ long lCol,
    /* [in] */ short nDBType,
    /* [defaultvalue][in] */ long lLen);


void __RPC_STUB IURowset_SetDataType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_GetRowsAt_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ VARIANT vtBookmarkValue,
    /* [defaultvalue][in] */ long lRowsOffset,
    /* [defaultvalue][in] */ short sSubBatchSize);


void __RPC_STUB IURowset_GetRowsAt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_UseStorageObjectForBLOB_Proxy( 
    IURowset __RPC_FAR * This,
    /* [defaultvalue][in] */ VARIANT_BOOL bUseStorageObject);


void __RPC_STUB IURowset_UseStorageObjectForBLOB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_GetData_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [retval][out] */ VARIANT __RPC_FAR *pvtData);


void __RPC_STUB IURowset_GetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_IsBLOB_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ long lCol,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbBLOB);


void __RPC_STUB IURowset_IsBLOB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_GetColName_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ long lCol,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrColName);


void __RPC_STUB IURowset_GetColName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_GetDataType_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ long lCol,
    /* [retval][out] */ short __RPC_FAR *psDBType);


void __RPC_STUB IURowset_GetDataType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_GetStatus_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [retval][out] */ long __RPC_FAR *plStatus);


void __RPC_STUB IURowset_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_FindColOrdinal_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ BSTR bstrColName,
    /* [defaultvalue][in] */ VARIANT_BOOL bCaseSensitive,
    /* [retval][out] */ long __RPC_FAR *plCol);


void __RPC_STUB IURowset_FindColOrdinal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_SetData_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [in] */ VARIANT vtData);


void __RPC_STUB IURowset_SetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_IsEOF_Proxy( 
    IURowset __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbIsEOF);


void __RPC_STUB IURowset_IsEOF_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_GetRowsFetched_Proxy( 
    IURowset __RPC_FAR * This,
    /* [retval][out] */ short __RPC_FAR *psCount);


void __RPC_STUB IURowset_GetRowsFetched_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_GetCols_Proxy( 
    IURowset __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *plCols);


void __RPC_STUB IURowset_GetCols_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_IsModified_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ short sRow,
    /* [in] */ long lCol,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbModified);


void __RPC_STUB IURowset_IsModified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_GetMaxLen_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ long lCol,
    /* [retval][out] */ long __RPC_FAR *plMaxLen);


void __RPC_STUB IURowset_GetMaxLen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_IsWritable_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ long lCol,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbWritable);


void __RPC_STUB IURowset_IsWritable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_GetRawDataType_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ long lCol,
    /* [retval][out] */ short __RPC_FAR *psDataType);


void __RPC_STUB IURowset_GetRawDataType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_GetDataSize_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [retval][out] */ long __RPC_FAR *plSize);


void __RPC_STUB IURowset_GetDataSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_GetBatchSize_Proxy( 
    IURowset __RPC_FAR * This,
    /* [retval][out] */ short __RPC_FAR *psBatchSize);


void __RPC_STUB IURowset_GetBatchSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_GetColFlags_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ long lCol,
    /* [retval][out] */ long __RPC_FAR *plFlags);


void __RPC_STUB IURowset_GetColFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_GetRawMaxColLen_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ long lCol,
    /* [retval][out] */ long __RPC_FAR *plLen);


void __RPC_STUB IURowset_GetRawMaxColLen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_ShrinkMemory_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ long lNewSize);


void __RPC_STUB IURowset_ShrinkMemory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_IsNullable_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ long lCol,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbIsNullable);


void __RPC_STUB IURowset_IsNullable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_BLOBDelayed_Proxy( 
    IURowset __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbDelayed);


void __RPC_STUB IURowset_BLOBDelayed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_IsDelayedUpdate_Proxy( 
    IURowset __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbDelayedUpdate);


void __RPC_STUB IURowset_IsDelayedUpdate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_IsReadOnly_Proxy( 
    IURowset __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbIsReadOnly);


void __RPC_STUB IURowset_IsReadOnly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_GetCountOfBLOBsPerRow_Proxy( 
    IURowset __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *plCountBLOBs);


void __RPC_STUB IURowset_GetCountOfBLOBsPerRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IURowset_get_ParentHandle_Proxy( 
    IURowset __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IURowset_get_ParentHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IURowset_put_ParentHandle_Proxy( 
    IURowset __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IURowset_put_ParentHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IURowset_get_Handle_Proxy( 
    IURowset __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IURowset_get_Handle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IURowset_get_Property_Proxy( 
    IURowset __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *pVal);


void __RPC_STUB IURowset_get_Property_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IURowset_get_BookmarkValue_Proxy( 
    IURowset __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *pVal);


void __RPC_STUB IURowset_get_BookmarkValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_GetBatchRecordsEx_Proxy( 
    IURowset __RPC_FAR * This,
    /* [defaultvalue][in] */ short sSubBatchSize,
    /* [defaultvalue][in] */ long lSkip);


void __RPC_STUB IURowset_GetBatchRecordsEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IURowset_GetBatchRecordsLast_Proxy( 
    IURowset __RPC_FAR * This,
    /* [defaultvalue][in] */ short sSubBatchSize);


void __RPC_STUB IURowset_GetBatchRecordsLast_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IURowset_INTERFACE_DEFINED__ */


#ifndef __IUDataReader_INTERFACE_DEFINED__
#define __IUDataReader_INTERFACE_DEFINED__

/* interface IUDataReader */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IUDataReader;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1A0B8C2A-742C-15D6-B1D1-1112B5EC1C5B")
    IUDataReader : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetUInt8( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ BYTE __RPC_FAR *pData) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetInt8( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ signed char __RPC_FAR *pData) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetUInt16( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ unsigned short __RPC_FAR *pusData) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetInt16( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ short __RPC_FAR *psData) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetUInt32( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ unsigned long __RPC_FAR *punData) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetInt32( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ long __RPC_FAR *pnData) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetFloat( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ float __RPC_FAR *pfData) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDouble( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ double __RPC_FAR *pdData) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetBool( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbData) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetVariant( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [retval][out] */ VARIANT __RPC_FAR *pvtData) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetUInt64( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ unsigned __int64 __RPC_FAR *pulData) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetInt64( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ __int64 __RPC_FAR *plData) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDecimal( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ DECIMAL __RPC_FAR *pdecData) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCurrency( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ CY __RPC_FAR *pcyData) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDateTime( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ DATE __RPC_FAR *pdData) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetBytes( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [in] */ unsigned long ulLen,
            /* [size_is][in] */ BYTE __RPC_FAR *pBuffer,
            /* [retval][out] */ unsigned long __RPC_FAR *plRtnSize) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetStringA( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [in] */ unsigned long cchar,
            /* [size_is][in] */ signed char __RPC_FAR *strA,
            /* [retval][out] */ unsigned long __RPC_FAR *plRtnChars) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetStringW( 
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [in] */ unsigned long cwchar,
            /* [size_is][in] */ LPWSTR strW,
            /* [retval][out] */ unsigned long __RPC_FAR *plRtnWChars) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_MatrixData( 
            /* [retval][out] */ VARIANT __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_InternalDataPointer( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUDataReaderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IUDataReader __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IUDataReader __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IUDataReader __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetUInt8 )( 
            IUDataReader __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ BYTE __RPC_FAR *pData);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInt8 )( 
            IUDataReader __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ signed char __RPC_FAR *pData);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetUInt16 )( 
            IUDataReader __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ unsigned short __RPC_FAR *pusData);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInt16 )( 
            IUDataReader __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ short __RPC_FAR *psData);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetUInt32 )( 
            IUDataReader __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ unsigned long __RPC_FAR *punData);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInt32 )( 
            IUDataReader __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ long __RPC_FAR *pnData);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFloat )( 
            IUDataReader __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ float __RPC_FAR *pfData);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDouble )( 
            IUDataReader __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ double __RPC_FAR *pdData);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBool )( 
            IUDataReader __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbData);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetVariant )( 
            IUDataReader __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [retval][out] */ VARIANT __RPC_FAR *pvtData);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetUInt64 )( 
            IUDataReader __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ unsigned __int64 __RPC_FAR *pulData);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInt64 )( 
            IUDataReader __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ __int64 __RPC_FAR *plData);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDecimal )( 
            IUDataReader __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ DECIMAL __RPC_FAR *pdecData);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCurrency )( 
            IUDataReader __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ CY __RPC_FAR *pcyData);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDateTime )( 
            IUDataReader __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
            /* [retval][out] */ DATE __RPC_FAR *pdData);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBytes )( 
            IUDataReader __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [in] */ unsigned long ulLen,
            /* [size_is][in] */ BYTE __RPC_FAR *pBuffer,
            /* [retval][out] */ unsigned long __RPC_FAR *plRtnSize);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStringA )( 
            IUDataReader __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [in] */ unsigned long cchar,
            /* [size_is][in] */ signed char __RPC_FAR *strA,
            /* [retval][out] */ unsigned long __RPC_FAR *plRtnChars);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStringW )( 
            IUDataReader __RPC_FAR * This,
            /* [in] */ short nRow,
            /* [in] */ long lCol,
            /* [in] */ unsigned long cwchar,
            /* [size_is][in] */ LPWSTR strW,
            /* [retval][out] */ unsigned long __RPC_FAR *plRtnWChars);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MatrixData )( 
            IUDataReader __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *pVal);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_InternalDataPointer )( 
            IUDataReader __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        END_INTERFACE
    } IUDataReaderVtbl;

    interface IUDataReader
    {
        CONST_VTBL struct IUDataReaderVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUDataReader_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUDataReader_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUDataReader_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUDataReader_GetUInt8(This,nRow,lCol,pbIsNull,pData)	\
    (This)->lpVtbl -> GetUInt8(This,nRow,lCol,pbIsNull,pData)

#define IUDataReader_GetInt8(This,nRow,lCol,pbIsNull,pData)	\
    (This)->lpVtbl -> GetInt8(This,nRow,lCol,pbIsNull,pData)

#define IUDataReader_GetUInt16(This,nRow,lCol,pbIsNull,pusData)	\
    (This)->lpVtbl -> GetUInt16(This,nRow,lCol,pbIsNull,pusData)

#define IUDataReader_GetInt16(This,nRow,lCol,pbIsNull,psData)	\
    (This)->lpVtbl -> GetInt16(This,nRow,lCol,pbIsNull,psData)

#define IUDataReader_GetUInt32(This,nRow,lCol,pbIsNull,punData)	\
    (This)->lpVtbl -> GetUInt32(This,nRow,lCol,pbIsNull,punData)

#define IUDataReader_GetInt32(This,nRow,lCol,pbIsNull,pnData)	\
    (This)->lpVtbl -> GetInt32(This,nRow,lCol,pbIsNull,pnData)

#define IUDataReader_GetFloat(This,nRow,lCol,pbIsNull,pfData)	\
    (This)->lpVtbl -> GetFloat(This,nRow,lCol,pbIsNull,pfData)

#define IUDataReader_GetDouble(This,nRow,lCol,pbIsNull,pdData)	\
    (This)->lpVtbl -> GetDouble(This,nRow,lCol,pbIsNull,pdData)

#define IUDataReader_GetBool(This,nRow,lCol,pbIsNull,pbData)	\
    (This)->lpVtbl -> GetBool(This,nRow,lCol,pbIsNull,pbData)

#define IUDataReader_GetVariant(This,nRow,lCol,pvtData)	\
    (This)->lpVtbl -> GetVariant(This,nRow,lCol,pvtData)

#define IUDataReader_GetUInt64(This,nRow,lCol,pbIsNull,pulData)	\
    (This)->lpVtbl -> GetUInt64(This,nRow,lCol,pbIsNull,pulData)

#define IUDataReader_GetInt64(This,nRow,lCol,pbIsNull,plData)	\
    (This)->lpVtbl -> GetInt64(This,nRow,lCol,pbIsNull,plData)

#define IUDataReader_GetDecimal(This,nRow,lCol,pbIsNull,pdecData)	\
    (This)->lpVtbl -> GetDecimal(This,nRow,lCol,pbIsNull,pdecData)

#define IUDataReader_GetCurrency(This,nRow,lCol,pbIsNull,pcyData)	\
    (This)->lpVtbl -> GetCurrency(This,nRow,lCol,pbIsNull,pcyData)

#define IUDataReader_GetDateTime(This,nRow,lCol,pbIsNull,pdData)	\
    (This)->lpVtbl -> GetDateTime(This,nRow,lCol,pbIsNull,pdData)

#define IUDataReader_GetBytes(This,nRow,lCol,ulLen,pBuffer,plRtnSize)	\
    (This)->lpVtbl -> GetBytes(This,nRow,lCol,ulLen,pBuffer,plRtnSize)

#define IUDataReader_GetStringA(This,nRow,lCol,cchar,strA,plRtnChars)	\
    (This)->lpVtbl -> GetStringA(This,nRow,lCol,cchar,strA,plRtnChars)

#define IUDataReader_GetStringW(This,nRow,lCol,cwchar,strW,plRtnWChars)	\
    (This)->lpVtbl -> GetStringW(This,nRow,lCol,cwchar,strW,plRtnWChars)

#define IUDataReader_get_MatrixData(This,pVal)	\
    (This)->lpVtbl -> get_MatrixData(This,pVal)

#define IUDataReader_get_InternalDataPointer(This,pVal)	\
    (This)->lpVtbl -> get_InternalDataPointer(This,pVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE IUDataReader_GetUInt8_Proxy( 
    IUDataReader __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
    /* [retval][out] */ BYTE __RPC_FAR *pData);


void __RPC_STUB IUDataReader_GetUInt8_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IUDataReader_GetInt8_Proxy( 
    IUDataReader __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
    /* [retval][out] */ signed char __RPC_FAR *pData);


void __RPC_STUB IUDataReader_GetInt8_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IUDataReader_GetUInt16_Proxy( 
    IUDataReader __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
    /* [retval][out] */ unsigned short __RPC_FAR *pusData);


void __RPC_STUB IUDataReader_GetUInt16_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IUDataReader_GetInt16_Proxy( 
    IUDataReader __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
    /* [retval][out] */ short __RPC_FAR *psData);


void __RPC_STUB IUDataReader_GetInt16_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IUDataReader_GetUInt32_Proxy( 
    IUDataReader __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
    /* [retval][out] */ unsigned long __RPC_FAR *punData);


void __RPC_STUB IUDataReader_GetUInt32_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IUDataReader_GetInt32_Proxy( 
    IUDataReader __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
    /* [retval][out] */ long __RPC_FAR *pnData);


void __RPC_STUB IUDataReader_GetInt32_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IUDataReader_GetFloat_Proxy( 
    IUDataReader __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
    /* [retval][out] */ float __RPC_FAR *pfData);


void __RPC_STUB IUDataReader_GetFloat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IUDataReader_GetDouble_Proxy( 
    IUDataReader __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
    /* [retval][out] */ double __RPC_FAR *pdData);


void __RPC_STUB IUDataReader_GetDouble_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IUDataReader_GetBool_Proxy( 
    IUDataReader __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbData);


void __RPC_STUB IUDataReader_GetBool_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IUDataReader_GetVariant_Proxy( 
    IUDataReader __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [retval][out] */ VARIANT __RPC_FAR *pvtData);


void __RPC_STUB IUDataReader_GetVariant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IUDataReader_GetUInt64_Proxy( 
    IUDataReader __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
    /* [retval][out] */ unsigned __int64 __RPC_FAR *pulData);


void __RPC_STUB IUDataReader_GetUInt64_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IUDataReader_GetInt64_Proxy( 
    IUDataReader __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
    /* [retval][out] */ __int64 __RPC_FAR *plData);


void __RPC_STUB IUDataReader_GetInt64_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IUDataReader_GetDecimal_Proxy( 
    IUDataReader __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
    /* [retval][out] */ DECIMAL __RPC_FAR *pdecData);


void __RPC_STUB IUDataReader_GetDecimal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IUDataReader_GetCurrency_Proxy( 
    IUDataReader __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
    /* [retval][out] */ CY __RPC_FAR *pcyData);


void __RPC_STUB IUDataReader_GetCurrency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IUDataReader_GetDateTime_Proxy( 
    IUDataReader __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [out] */ VARIANT_BOOL __RPC_FAR *pbIsNull,
    /* [retval][out] */ DATE __RPC_FAR *pdData);


void __RPC_STUB IUDataReader_GetDateTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IUDataReader_GetBytes_Proxy( 
    IUDataReader __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [in] */ unsigned long ulLen,
    /* [size_is][in] */ BYTE __RPC_FAR *pBuffer,
    /* [retval][out] */ unsigned long __RPC_FAR *plRtnSize);


void __RPC_STUB IUDataReader_GetBytes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IUDataReader_GetStringA_Proxy( 
    IUDataReader __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [in] */ unsigned long cchar,
    /* [size_is][in] */ signed char __RPC_FAR *strA,
    /* [retval][out] */ unsigned long __RPC_FAR *plRtnChars);


void __RPC_STUB IUDataReader_GetStringA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IUDataReader_GetStringW_Proxy( 
    IUDataReader __RPC_FAR * This,
    /* [in] */ short nRow,
    /* [in] */ long lCol,
    /* [in] */ unsigned long cwchar,
    /* [size_is][in] */ LPWSTR strW,
    /* [retval][out] */ unsigned long __RPC_FAR *plRtnWChars);


void __RPC_STUB IUDataReader_GetStringW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IUDataReader_get_MatrixData_Proxy( 
    IUDataReader __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *pVal);


void __RPC_STUB IUDataReader_get_MatrixData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IUDataReader_get_InternalDataPointer_Proxy( 
    IUDataReader __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IUDataReader_get_InternalDataPointer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IUDataReader_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_UDataSource;

#ifdef __cplusplus

class DECLSPEC_UUID("15CF3B4A-9E2D-47FE-BFC8-942C83CB94B6")
UDataSource;
#endif

EXTERN_C const CLSID CLSID_USession;

#ifdef __cplusplus

class DECLSPEC_UUID("A1FE27D7-FD8B-4029-AEB7-DFBC0321D9D0")
USession;
#endif

EXTERN_C const CLSID CLSID_UCommand;

#ifdef __cplusplus

class DECLSPEC_UUID("7BBFAE6E-1863-4FA5-BE86-977B5FC66E82")
UCommand;
#endif

EXTERN_C const CLSID CLSID_URowset;

#ifdef __cplusplus

class DECLSPEC_UUID("BBE0E397-6941-45A9-B1E8-D81BE1B74992")
URowset;
#endif
#endif /* __UDBLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
