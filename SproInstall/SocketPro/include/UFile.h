
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 5.03.0280 */
/* at Sat Apr 24 08:32:44 2004
 */
/* Compiler settings for E:\uskt\UFile\UFile.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32 (32b run), ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __UFile_h__
#define __UFile_h__

/* Forward Declarations */ 

#ifndef __IUObjBase_FWD_DEFINED__
#define __IUObjBase_FWD_DEFINED__
typedef interface IUObjBase IUObjBase;
#endif 	/* __IUObjBase_FWD_DEFINED__ */


#ifndef ___IURequestEvent_FWD_DEFINED__
#define ___IURequestEvent_FWD_DEFINED__
typedef interface _IURequestEvent _IURequestEvent;
#endif 	/* ___IURequestEvent_FWD_DEFINED__ */


#ifndef __IUFile_FWD_DEFINED__
#define __IUFile_FWD_DEFINED__
typedef interface IUFile IUFile;
#endif 	/* __IUFile_FWD_DEFINED__ */


#ifndef __UFile_FWD_DEFINED__
#define __UFile_FWD_DEFINED__

#ifdef __cplusplus
typedef class UFile UFile;
#else
typedef struct UFile UFile;
#endif /* __cplusplus */

#endif 	/* __UFile_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __UFILELib_LIBRARY_DEFINED__
#define __UFILELib_LIBRARY_DEFINED__

/* library UFILELib */
/* [helpstring][version][uuid] */ 


enum tagDesiredAccess
    {	daZero	= 0,
	daGenericRead	= 0x80000000,
	daGenericWrite	= 0x40000000,
	daGenericExecute	= 0x20000000,
	daGenericAll	= 0x10000000
    };

enum tagShareMode
    {	smFileShareRead	= 1,
	smFileShareWrite	= 2,
	smFileShareDelete	= 4
    };

enum tagCreationDisposition
    {	cdCreateNew	= 1,
	cdCreateAlways	= 2,
	cdOpenExisting	= 3,
	cdOpenAlways	= 4,
	cdTruncateExisting	= 5
    };

enum tagFileAttribute
    {	faReadOnly	= 0x1,
	faHidden	= 0x2,
	faSystem	= 0x4,
	faDirectory	= 0x10,
	faArchive	= 0x20,
	faEncrypted	= 0x40,
	faNormal	= 0x80,
	faTemporary	= 0x100,
	faSparseFile	= 0x200,
	faReparsePoint	= 0x400,
	faCompressed	= 0x800,
	faOffLine	= 0x1000,
	faNotContentIndexed	= 0x2000
    };

enum tagFileFlag
    {	ffWriteThrough	= 0x80000000,
	ffNoBuffering	= 0x20000000,
	ffRandomAccess	= 0x10000000,
	ffSequentialScan	= 0x8000000
    };

enum tagMoveMethod
    {	mmFileBegin	= 0,
	mmFileCurrent	= 1,
	mmFileEnd	= 2
    };

enum tagDriveType
    {	dtUnknown	= 0,
	dtNoRootDir	= dtUnknown + 1,
	dtRemovable	= dtNoRootDir + 1,
	dtFixed	= dtRemovable + 1,
	dtRemote	= dtFixed + 1,
	dtCDROM	= dtRemote + 1,
	dtRAMDisk	= dtCDROM + 1
    };

enum tagFileRequestID
    {	idClose	= 46,
	idFindClose	= idClose + 1,
	idSetFileAttributes	= idFindClose + 1,
	idGetFileAttributes	= idSetFileAttributes + 1,
	idDeleteFile	= idGetFileAttributes + 1,
	idCreateDirectory	= idDeleteFile + 1,
	idRemoveDirectory	= idCreateDirectory + 1,
	idGetCurrentDirectory	= idRemoveDirectory + 1,
	idFindFile	= idGetCurrentDirectory + 1,
	idFindFirstFile	= idFindFile + 1,
	idFindNextFile	= idFindFirstFile + 1,
	idCreateFile	= idFindNextFile + 1,
	idWriteFile	= idCreateFile + 1,
	idReadFile	= idWriteFile + 1,
	idMoveFile	= idReadFile + 1,
	idCopyFile	= idMoveFile + 1,
	idSetFilePointer	= idCopyFile + 1,
	idSetCurrentDirectory	= idSetFilePointer + 1,
	idFlushFileBuffers	= idSetCurrentDirectory + 1,
	idGetDiskFreeSpaceEx	= idFlushFileBuffers + 1,
	idGetDriveType	= idGetDiskFreeSpaceEx + 1,
	idGetFile	= idGetDriveType + 1,
	idSendFile	= idGetFile + 1,
	idGetFileSize	= idSendFile + 1,
	idLockFile	= idGetFileSize + 1,
	idUnlockFile	= idLockFile + 1,
	idSetEndOfFile	= idUnlockFile + 1,
	idSearchPath	= idSetEndOfFile + 1,
	idFindAll	= idSearchPath + 1,
	idSetRootDirectory	= idFindAll + 1,
	idGetRootDirectory	= idSetRootDirectory + 1,
	idSendBytesToClient	= idGetRootDirectory + 1,
	idSendBytesToServer	= idSendBytesToClient + 1
    };

EXTERN_C const IID LIBID_UFILELib;

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


#ifndef __IUFile_INTERFACE_DEFINED__
#define __IUFile_INTERFACE_DEFINED__

/* interface IUFile */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IUFile;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DE98E3D3-699B-4AD1-8287-3481972CEBB9")
    IUFile : public IUObjBase
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FindClose( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetFileAttributes( 
            /* [in] */ BSTR bstrFileName,
            /* [in] */ long lFileAttributes) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DeleteFile( 
            /* [in] */ BSTR bstrFileName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CreateDirectory( 
            /* [in] */ BSTR bstrDirectory) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RemoveDirectory( 
            /* [in] */ BSTR bstrDirectory) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCurrentDirectory( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FindFile( 
            /* [in] */ BSTR bstrFileName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FindFirstFile( 
            /* [in] */ BSTR bstrFileName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FindNextFile( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CreateFile( 
            /* [in] */ BSTR bstrFileName,
            /* [in] */ long lDesiredAccess,
            /* [in] */ long lShareMode,
            /* [in] */ long lCreationDisposition,
            /* [in] */ long lFlagsAndAttributes) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE WriteFile( 
            /* [in] */ VARIANT vtData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ReadFile( 
            /* [in] */ long lLen) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE MoveFile( 
            /* [in] */ BSTR bstrExistingFileName,
            /* [in] */ BSTR bstrNewFileName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CopyFile( 
            /* [in] */ BSTR bstrExistingFileName,
            /* [in] */ BSTR bstrNewFileName,
            /* [defaultvalue][in] */ VARIANT_BOOL bFailIfExists = -1) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetFilePointer( 
            /* [in] */ long lDistanceToMove,
            /* [in] */ long lMoveMethod) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetCurrentDirectory( 
            /* [in] */ BSTR bstrCurrentDirectory) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FlushFileBuffers( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetDiskFreeSpaceEx( 
            /* [defaultvalue][in] */ BSTR bstrDirectoryName = L"") = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetDriveType( 
            /* [defaultvalue][in] */ BSTR bstrRootPathName = L"") = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetFile( 
            /* [in] */ BSTR bstrSrcFileName,
            /* [in] */ BSTR bstrDesFileName,
            /* [defaultvalue][in] */ VARIANT_BOOL bFailIfExists = -1) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SendFile( 
            /* [in] */ BSTR bstrSrcFileName,
            /* [in] */ BSTR bstrDesFileName,
            /* [defaultvalue][in] */ VARIANT_BOOL bFailIfExists = -1) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetFileSize( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE LockFile( 
            /* [in] */ long lFileOffsetLow,
            /* [in] */ long lFileOffsetHigh,
            /* [in] */ long lNumberOfBytesToLockLow,
            /* [in] */ long lNumberOfBytesToLockHigh) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE UnlockFile( 
            /* [in] */ long lFileOffsetLow,
            /* [in] */ long lFileOffsetHigh,
            /* [in] */ long lNumberOfBytesToLockLow,
            /* [in] */ long lNumberOfBytesToLockHigh) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetEndOfFile( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SearchPath( 
            /* [in] */ BSTR bstrFileName,
            /* [defaultvalue][in] */ BSTR bstrPath = L"",
            /* [defaultvalue][in] */ BSTR bstrExtension = L"") = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FindAll( 
            /* [in] */ BSTR bstrFileName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetRootDirectory( 
            /* [in] */ BSTR bstrRootDirectory) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetRootDirectory( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetLastWriteTime( 
            /* [out] */ short __RPC_FAR *pnYear,
            /* [out] */ short __RPC_FAR *pnMonth,
            /* [out] */ short __RPC_FAR *pnDayOfWeek,
            /* [out] */ short __RPC_FAR *pnDay,
            /* [out] */ short __RPC_FAR *pnHour,
            /* [out] */ short __RPC_FAR *pnMinute,
            /* [out] */ short __RPC_FAR *pnSecond,
            /* [out] */ short __RPC_FAR *pnMilliseconds) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetLastAccessTime( 
            /* [out] */ short __RPC_FAR *pnYear,
            /* [out] */ short __RPC_FAR *pnMonth,
            /* [out] */ short __RPC_FAR *pnDayOfWeek,
            /* [out] */ short __RPC_FAR *pnDay,
            /* [out] */ short __RPC_FAR *pnHour,
            /* [out] */ short __RPC_FAR *pnMinute,
            /* [out] */ short __RPC_FAR *pnSecond,
            /* [out] */ short __RPC_FAR *pnMilliseconds) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCreationTime( 
            /* [out] */ short __RPC_FAR *pnYear,
            /* [out] */ short __RPC_FAR *pnMonth,
            /* [out] */ short __RPC_FAR *pnDayOfWeek,
            /* [out] */ short __RPC_FAR *pnDay,
            /* [out] */ short __RPC_FAR *pnHour,
            /* [out] */ short __RPC_FAR *pnMinute,
            /* [out] */ short __RPC_FAR *pnSecond,
            /* [out] */ short __RPC_FAR *pnMilliseconds) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetSpaceData( 
            /* [out] */ long __RPC_FAR *plFreeBytesAvailableToCallerLowPart,
            /* [defaultvalue][out] */ long __RPC_FAR *plTotalNumberOfBytesLowPart = 0,
            /* [defaultvalue][out] */ long __RPC_FAR *plTotalNumberOfFreeBytesLowPart = 0,
            /* [defaultvalue][out] */ long __RPC_FAR *plFreeBytesAvailableToCallerHighPart = 0,
            /* [defaultvalue][out] */ long __RPC_FAR *plTotalNumberOfBytesHighPart = 0,
            /* [defaultvalue][out] */ long __RPC_FAR *plTotalNumberOfFreeBytesHighPart = 0) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetFileAttributes( 
            /* [in] */ BSTR bstrFileName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetFilePart( 
            /* [defaultvalue][out] */ BSTR __RPC_FAR *pbstrFullFile,
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FileSize( 
            /* [defaultvalue][out] */ long __RPC_FAR *plFileSizeHigh,
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DriveType( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FilePointer( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FileNameOrDirectory( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FileAttributes( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RootDirectory( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Data( 
            /* [retval][out] */ VARIANT __RPC_FAR *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUFileVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IUFile __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IUFile __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IUFile __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IUFile __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IUFile __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IUFile __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IUFile __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AttachSocket )( 
            IUFile __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pIUnknownToUSocket);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Rtn )( 
            IUFile __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plResult);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ErrorMsg )( 
            IUFile __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Close )( 
            IUFile __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindClose )( 
            IUFile __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetFileAttributes )( 
            IUFile __RPC_FAR * This,
            /* [in] */ BSTR bstrFileName,
            /* [in] */ long lFileAttributes);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteFile )( 
            IUFile __RPC_FAR * This,
            /* [in] */ BSTR bstrFileName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateDirectory )( 
            IUFile __RPC_FAR * This,
            /* [in] */ BSTR bstrDirectory);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveDirectory )( 
            IUFile __RPC_FAR * This,
            /* [in] */ BSTR bstrDirectory);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCurrentDirectory )( 
            IUFile __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindFile )( 
            IUFile __RPC_FAR * This,
            /* [in] */ BSTR bstrFileName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindFirstFile )( 
            IUFile __RPC_FAR * This,
            /* [in] */ BSTR bstrFileName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindNextFile )( 
            IUFile __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateFile )( 
            IUFile __RPC_FAR * This,
            /* [in] */ BSTR bstrFileName,
            /* [in] */ long lDesiredAccess,
            /* [in] */ long lShareMode,
            /* [in] */ long lCreationDisposition,
            /* [in] */ long lFlagsAndAttributes);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WriteFile )( 
            IUFile __RPC_FAR * This,
            /* [in] */ VARIANT vtData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReadFile )( 
            IUFile __RPC_FAR * This,
            /* [in] */ long lLen);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveFile )( 
            IUFile __RPC_FAR * This,
            /* [in] */ BSTR bstrExistingFileName,
            /* [in] */ BSTR bstrNewFileName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CopyFile )( 
            IUFile __RPC_FAR * This,
            /* [in] */ BSTR bstrExistingFileName,
            /* [in] */ BSTR bstrNewFileName,
            /* [defaultvalue][in] */ VARIANT_BOOL bFailIfExists);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetFilePointer )( 
            IUFile __RPC_FAR * This,
            /* [in] */ long lDistanceToMove,
            /* [in] */ long lMoveMethod);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCurrentDirectory )( 
            IUFile __RPC_FAR * This,
            /* [in] */ BSTR bstrCurrentDirectory);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FlushFileBuffers )( 
            IUFile __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDiskFreeSpaceEx )( 
            IUFile __RPC_FAR * This,
            /* [defaultvalue][in] */ BSTR bstrDirectoryName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDriveType )( 
            IUFile __RPC_FAR * This,
            /* [defaultvalue][in] */ BSTR bstrRootPathName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFile )( 
            IUFile __RPC_FAR * This,
            /* [in] */ BSTR bstrSrcFileName,
            /* [in] */ BSTR bstrDesFileName,
            /* [defaultvalue][in] */ VARIANT_BOOL bFailIfExists);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendFile )( 
            IUFile __RPC_FAR * This,
            /* [in] */ BSTR bstrSrcFileName,
            /* [in] */ BSTR bstrDesFileName,
            /* [defaultvalue][in] */ VARIANT_BOOL bFailIfExists);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFileSize )( 
            IUFile __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LockFile )( 
            IUFile __RPC_FAR * This,
            /* [in] */ long lFileOffsetLow,
            /* [in] */ long lFileOffsetHigh,
            /* [in] */ long lNumberOfBytesToLockLow,
            /* [in] */ long lNumberOfBytesToLockHigh);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UnlockFile )( 
            IUFile __RPC_FAR * This,
            /* [in] */ long lFileOffsetLow,
            /* [in] */ long lFileOffsetHigh,
            /* [in] */ long lNumberOfBytesToLockLow,
            /* [in] */ long lNumberOfBytesToLockHigh);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetEndOfFile )( 
            IUFile __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SearchPath )( 
            IUFile __RPC_FAR * This,
            /* [in] */ BSTR bstrFileName,
            /* [defaultvalue][in] */ BSTR bstrPath,
            /* [defaultvalue][in] */ BSTR bstrExtension);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindAll )( 
            IUFile __RPC_FAR * This,
            /* [in] */ BSTR bstrFileName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetRootDirectory )( 
            IUFile __RPC_FAR * This,
            /* [in] */ BSTR bstrRootDirectory);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRootDirectory )( 
            IUFile __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLastWriteTime )( 
            IUFile __RPC_FAR * This,
            /* [out] */ short __RPC_FAR *pnYear,
            /* [out] */ short __RPC_FAR *pnMonth,
            /* [out] */ short __RPC_FAR *pnDayOfWeek,
            /* [out] */ short __RPC_FAR *pnDay,
            /* [out] */ short __RPC_FAR *pnHour,
            /* [out] */ short __RPC_FAR *pnMinute,
            /* [out] */ short __RPC_FAR *pnSecond,
            /* [out] */ short __RPC_FAR *pnMilliseconds);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLastAccessTime )( 
            IUFile __RPC_FAR * This,
            /* [out] */ short __RPC_FAR *pnYear,
            /* [out] */ short __RPC_FAR *pnMonth,
            /* [out] */ short __RPC_FAR *pnDayOfWeek,
            /* [out] */ short __RPC_FAR *pnDay,
            /* [out] */ short __RPC_FAR *pnHour,
            /* [out] */ short __RPC_FAR *pnMinute,
            /* [out] */ short __RPC_FAR *pnSecond,
            /* [out] */ short __RPC_FAR *pnMilliseconds);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCreationTime )( 
            IUFile __RPC_FAR * This,
            /* [out] */ short __RPC_FAR *pnYear,
            /* [out] */ short __RPC_FAR *pnMonth,
            /* [out] */ short __RPC_FAR *pnDayOfWeek,
            /* [out] */ short __RPC_FAR *pnDay,
            /* [out] */ short __RPC_FAR *pnHour,
            /* [out] */ short __RPC_FAR *pnMinute,
            /* [out] */ short __RPC_FAR *pnSecond,
            /* [out] */ short __RPC_FAR *pnMilliseconds);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSpaceData )( 
            IUFile __RPC_FAR * This,
            /* [out] */ long __RPC_FAR *plFreeBytesAvailableToCallerLowPart,
            /* [defaultvalue][out] */ long __RPC_FAR *plTotalNumberOfBytesLowPart,
            /* [defaultvalue][out] */ long __RPC_FAR *plTotalNumberOfFreeBytesLowPart,
            /* [defaultvalue][out] */ long __RPC_FAR *plFreeBytesAvailableToCallerHighPart,
            /* [defaultvalue][out] */ long __RPC_FAR *plTotalNumberOfBytesHighPart,
            /* [defaultvalue][out] */ long __RPC_FAR *plTotalNumberOfFreeBytesHighPart);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFileAttributes )( 
            IUFile __RPC_FAR * This,
            /* [in] */ BSTR bstrFileName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Cancel )( 
            IUFile __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFilePart )( 
            IUFile __RPC_FAR * This,
            /* [defaultvalue][out] */ BSTR __RPC_FAR *pbstrFullFile,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FileSize )( 
            IUFile __RPC_FAR * This,
            /* [defaultvalue][out] */ long __RPC_FAR *plFileSizeHigh,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DriveType )( 
            IUFile __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FilePointer )( 
            IUFile __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FileNameOrDirectory )( 
            IUFile __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FileAttributes )( 
            IUFile __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_RootDirectory )( 
            IUFile __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Data )( 
            IUFile __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *pVal);
        
        END_INTERFACE
    } IUFileVtbl;

    interface IUFile
    {
        CONST_VTBL struct IUFileVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUFile_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUFile_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUFile_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUFile_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IUFile_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IUFile_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IUFile_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IUFile_AttachSocket(This,pIUnknownToUSocket)	\
    (This)->lpVtbl -> AttachSocket(This,pIUnknownToUSocket)

#define IUFile_get_Rtn(This,plResult)	\
    (This)->lpVtbl -> get_Rtn(This,plResult)

#define IUFile_get_ErrorMsg(This,pVal)	\
    (This)->lpVtbl -> get_ErrorMsg(This,pVal)


#define IUFile_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IUFile_FindClose(This)	\
    (This)->lpVtbl -> FindClose(This)

#define IUFile_SetFileAttributes(This,bstrFileName,lFileAttributes)	\
    (This)->lpVtbl -> SetFileAttributes(This,bstrFileName,lFileAttributes)

#define IUFile_DeleteFile(This,bstrFileName)	\
    (This)->lpVtbl -> DeleteFile(This,bstrFileName)

#define IUFile_CreateDirectory(This,bstrDirectory)	\
    (This)->lpVtbl -> CreateDirectory(This,bstrDirectory)

#define IUFile_RemoveDirectory(This,bstrDirectory)	\
    (This)->lpVtbl -> RemoveDirectory(This,bstrDirectory)

#define IUFile_GetCurrentDirectory(This)	\
    (This)->lpVtbl -> GetCurrentDirectory(This)

#define IUFile_FindFile(This,bstrFileName)	\
    (This)->lpVtbl -> FindFile(This,bstrFileName)

#define IUFile_FindFirstFile(This,bstrFileName)	\
    (This)->lpVtbl -> FindFirstFile(This,bstrFileName)

#define IUFile_FindNextFile(This)	\
    (This)->lpVtbl -> FindNextFile(This)

#define IUFile_CreateFile(This,bstrFileName,lDesiredAccess,lShareMode,lCreationDisposition,lFlagsAndAttributes)	\
    (This)->lpVtbl -> CreateFile(This,bstrFileName,lDesiredAccess,lShareMode,lCreationDisposition,lFlagsAndAttributes)

#define IUFile_WriteFile(This,vtData)	\
    (This)->lpVtbl -> WriteFile(This,vtData)

#define IUFile_ReadFile(This,lLen)	\
    (This)->lpVtbl -> ReadFile(This,lLen)

#define IUFile_MoveFile(This,bstrExistingFileName,bstrNewFileName)	\
    (This)->lpVtbl -> MoveFile(This,bstrExistingFileName,bstrNewFileName)

#define IUFile_CopyFile(This,bstrExistingFileName,bstrNewFileName,bFailIfExists)	\
    (This)->lpVtbl -> CopyFile(This,bstrExistingFileName,bstrNewFileName,bFailIfExists)

#define IUFile_SetFilePointer(This,lDistanceToMove,lMoveMethod)	\
    (This)->lpVtbl -> SetFilePointer(This,lDistanceToMove,lMoveMethod)

#define IUFile_SetCurrentDirectory(This,bstrCurrentDirectory)	\
    (This)->lpVtbl -> SetCurrentDirectory(This,bstrCurrentDirectory)

#define IUFile_FlushFileBuffers(This)	\
    (This)->lpVtbl -> FlushFileBuffers(This)

#define IUFile_GetDiskFreeSpaceEx(This,bstrDirectoryName)	\
    (This)->lpVtbl -> GetDiskFreeSpaceEx(This,bstrDirectoryName)

#define IUFile_GetDriveType(This,bstrRootPathName)	\
    (This)->lpVtbl -> GetDriveType(This,bstrRootPathName)

#define IUFile_GetFile(This,bstrSrcFileName,bstrDesFileName,bFailIfExists)	\
    (This)->lpVtbl -> GetFile(This,bstrSrcFileName,bstrDesFileName,bFailIfExists)

#define IUFile_SendFile(This,bstrSrcFileName,bstrDesFileName,bFailIfExists)	\
    (This)->lpVtbl -> SendFile(This,bstrSrcFileName,bstrDesFileName,bFailIfExists)

#define IUFile_GetFileSize(This)	\
    (This)->lpVtbl -> GetFileSize(This)

#define IUFile_LockFile(This,lFileOffsetLow,lFileOffsetHigh,lNumberOfBytesToLockLow,lNumberOfBytesToLockHigh)	\
    (This)->lpVtbl -> LockFile(This,lFileOffsetLow,lFileOffsetHigh,lNumberOfBytesToLockLow,lNumberOfBytesToLockHigh)

#define IUFile_UnlockFile(This,lFileOffsetLow,lFileOffsetHigh,lNumberOfBytesToLockLow,lNumberOfBytesToLockHigh)	\
    (This)->lpVtbl -> UnlockFile(This,lFileOffsetLow,lFileOffsetHigh,lNumberOfBytesToLockLow,lNumberOfBytesToLockHigh)

#define IUFile_SetEndOfFile(This)	\
    (This)->lpVtbl -> SetEndOfFile(This)

#define IUFile_SearchPath(This,bstrFileName,bstrPath,bstrExtension)	\
    (This)->lpVtbl -> SearchPath(This,bstrFileName,bstrPath,bstrExtension)

#define IUFile_FindAll(This,bstrFileName)	\
    (This)->lpVtbl -> FindAll(This,bstrFileName)

#define IUFile_SetRootDirectory(This,bstrRootDirectory)	\
    (This)->lpVtbl -> SetRootDirectory(This,bstrRootDirectory)

#define IUFile_GetRootDirectory(This)	\
    (This)->lpVtbl -> GetRootDirectory(This)

#define IUFile_GetLastWriteTime(This,pnYear,pnMonth,pnDayOfWeek,pnDay,pnHour,pnMinute,pnSecond,pnMilliseconds)	\
    (This)->lpVtbl -> GetLastWriteTime(This,pnYear,pnMonth,pnDayOfWeek,pnDay,pnHour,pnMinute,pnSecond,pnMilliseconds)

#define IUFile_GetLastAccessTime(This,pnYear,pnMonth,pnDayOfWeek,pnDay,pnHour,pnMinute,pnSecond,pnMilliseconds)	\
    (This)->lpVtbl -> GetLastAccessTime(This,pnYear,pnMonth,pnDayOfWeek,pnDay,pnHour,pnMinute,pnSecond,pnMilliseconds)

#define IUFile_GetCreationTime(This,pnYear,pnMonth,pnDayOfWeek,pnDay,pnHour,pnMinute,pnSecond,pnMilliseconds)	\
    (This)->lpVtbl -> GetCreationTime(This,pnYear,pnMonth,pnDayOfWeek,pnDay,pnHour,pnMinute,pnSecond,pnMilliseconds)

#define IUFile_GetSpaceData(This,plFreeBytesAvailableToCallerLowPart,plTotalNumberOfBytesLowPart,plTotalNumberOfFreeBytesLowPart,plFreeBytesAvailableToCallerHighPart,plTotalNumberOfBytesHighPart,plTotalNumberOfFreeBytesHighPart)	\
    (This)->lpVtbl -> GetSpaceData(This,plFreeBytesAvailableToCallerLowPart,plTotalNumberOfBytesLowPart,plTotalNumberOfFreeBytesLowPart,plFreeBytesAvailableToCallerHighPart,plTotalNumberOfBytesHighPart,plTotalNumberOfFreeBytesHighPart)

#define IUFile_GetFileAttributes(This,bstrFileName)	\
    (This)->lpVtbl -> GetFileAttributes(This,bstrFileName)

#define IUFile_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)

#define IUFile_GetFilePart(This,pbstrFullFile,pVal)	\
    (This)->lpVtbl -> GetFilePart(This,pbstrFullFile,pVal)

#define IUFile_FileSize(This,plFileSizeHigh,pVal)	\
    (This)->lpVtbl -> FileSize(This,plFileSizeHigh,pVal)

#define IUFile_get_DriveType(This,pVal)	\
    (This)->lpVtbl -> get_DriveType(This,pVal)

#define IUFile_get_FilePointer(This,pVal)	\
    (This)->lpVtbl -> get_FilePointer(This,pVal)

#define IUFile_get_FileNameOrDirectory(This,pVal)	\
    (This)->lpVtbl -> get_FileNameOrDirectory(This,pVal)

#define IUFile_get_FileAttributes(This,pVal)	\
    (This)->lpVtbl -> get_FileAttributes(This,pVal)

#define IUFile_get_RootDirectory(This,pVal)	\
    (This)->lpVtbl -> get_RootDirectory(This,pVal)

#define IUFile_get_Data(This,pVal)	\
    (This)->lpVtbl -> get_Data(This,pVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_Close_Proxy( 
    IUFile __RPC_FAR * This);


void __RPC_STUB IUFile_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_FindClose_Proxy( 
    IUFile __RPC_FAR * This);


void __RPC_STUB IUFile_FindClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_SetFileAttributes_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ BSTR bstrFileName,
    /* [in] */ long lFileAttributes);


void __RPC_STUB IUFile_SetFileAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_DeleteFile_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ BSTR bstrFileName);


void __RPC_STUB IUFile_DeleteFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_CreateDirectory_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ BSTR bstrDirectory);


void __RPC_STUB IUFile_CreateDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_RemoveDirectory_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ BSTR bstrDirectory);


void __RPC_STUB IUFile_RemoveDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_GetCurrentDirectory_Proxy( 
    IUFile __RPC_FAR * This);


void __RPC_STUB IUFile_GetCurrentDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_FindFile_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ BSTR bstrFileName);


void __RPC_STUB IUFile_FindFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_FindFirstFile_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ BSTR bstrFileName);


void __RPC_STUB IUFile_FindFirstFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_FindNextFile_Proxy( 
    IUFile __RPC_FAR * This);


void __RPC_STUB IUFile_FindNextFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_CreateFile_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ BSTR bstrFileName,
    /* [in] */ long lDesiredAccess,
    /* [in] */ long lShareMode,
    /* [in] */ long lCreationDisposition,
    /* [in] */ long lFlagsAndAttributes);


void __RPC_STUB IUFile_CreateFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_WriteFile_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ VARIANT vtData);


void __RPC_STUB IUFile_WriteFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_ReadFile_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ long lLen);


void __RPC_STUB IUFile_ReadFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_MoveFile_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ BSTR bstrExistingFileName,
    /* [in] */ BSTR bstrNewFileName);


void __RPC_STUB IUFile_MoveFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_CopyFile_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ BSTR bstrExistingFileName,
    /* [in] */ BSTR bstrNewFileName,
    /* [defaultvalue][in] */ VARIANT_BOOL bFailIfExists);


void __RPC_STUB IUFile_CopyFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_SetFilePointer_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ long lDistanceToMove,
    /* [in] */ long lMoveMethod);


void __RPC_STUB IUFile_SetFilePointer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_SetCurrentDirectory_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ BSTR bstrCurrentDirectory);


void __RPC_STUB IUFile_SetCurrentDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_FlushFileBuffers_Proxy( 
    IUFile __RPC_FAR * This);


void __RPC_STUB IUFile_FlushFileBuffers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_GetDiskFreeSpaceEx_Proxy( 
    IUFile __RPC_FAR * This,
    /* [defaultvalue][in] */ BSTR bstrDirectoryName);


void __RPC_STUB IUFile_GetDiskFreeSpaceEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_GetDriveType_Proxy( 
    IUFile __RPC_FAR * This,
    /* [defaultvalue][in] */ BSTR bstrRootPathName);


void __RPC_STUB IUFile_GetDriveType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_GetFile_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ BSTR bstrSrcFileName,
    /* [in] */ BSTR bstrDesFileName,
    /* [defaultvalue][in] */ VARIANT_BOOL bFailIfExists);


void __RPC_STUB IUFile_GetFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_SendFile_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ BSTR bstrSrcFileName,
    /* [in] */ BSTR bstrDesFileName,
    /* [defaultvalue][in] */ VARIANT_BOOL bFailIfExists);


void __RPC_STUB IUFile_SendFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_GetFileSize_Proxy( 
    IUFile __RPC_FAR * This);


void __RPC_STUB IUFile_GetFileSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_LockFile_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ long lFileOffsetLow,
    /* [in] */ long lFileOffsetHigh,
    /* [in] */ long lNumberOfBytesToLockLow,
    /* [in] */ long lNumberOfBytesToLockHigh);


void __RPC_STUB IUFile_LockFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_UnlockFile_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ long lFileOffsetLow,
    /* [in] */ long lFileOffsetHigh,
    /* [in] */ long lNumberOfBytesToLockLow,
    /* [in] */ long lNumberOfBytesToLockHigh);


void __RPC_STUB IUFile_UnlockFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_SetEndOfFile_Proxy( 
    IUFile __RPC_FAR * This);


void __RPC_STUB IUFile_SetEndOfFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_SearchPath_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ BSTR bstrFileName,
    /* [defaultvalue][in] */ BSTR bstrPath,
    /* [defaultvalue][in] */ BSTR bstrExtension);


void __RPC_STUB IUFile_SearchPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_FindAll_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ BSTR bstrFileName);


void __RPC_STUB IUFile_FindAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_SetRootDirectory_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ BSTR bstrRootDirectory);


void __RPC_STUB IUFile_SetRootDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_GetRootDirectory_Proxy( 
    IUFile __RPC_FAR * This);


void __RPC_STUB IUFile_GetRootDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_GetLastWriteTime_Proxy( 
    IUFile __RPC_FAR * This,
    /* [out] */ short __RPC_FAR *pnYear,
    /* [out] */ short __RPC_FAR *pnMonth,
    /* [out] */ short __RPC_FAR *pnDayOfWeek,
    /* [out] */ short __RPC_FAR *pnDay,
    /* [out] */ short __RPC_FAR *pnHour,
    /* [out] */ short __RPC_FAR *pnMinute,
    /* [out] */ short __RPC_FAR *pnSecond,
    /* [out] */ short __RPC_FAR *pnMilliseconds);


void __RPC_STUB IUFile_GetLastWriteTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_GetLastAccessTime_Proxy( 
    IUFile __RPC_FAR * This,
    /* [out] */ short __RPC_FAR *pnYear,
    /* [out] */ short __RPC_FAR *pnMonth,
    /* [out] */ short __RPC_FAR *pnDayOfWeek,
    /* [out] */ short __RPC_FAR *pnDay,
    /* [out] */ short __RPC_FAR *pnHour,
    /* [out] */ short __RPC_FAR *pnMinute,
    /* [out] */ short __RPC_FAR *pnSecond,
    /* [out] */ short __RPC_FAR *pnMilliseconds);


void __RPC_STUB IUFile_GetLastAccessTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_GetCreationTime_Proxy( 
    IUFile __RPC_FAR * This,
    /* [out] */ short __RPC_FAR *pnYear,
    /* [out] */ short __RPC_FAR *pnMonth,
    /* [out] */ short __RPC_FAR *pnDayOfWeek,
    /* [out] */ short __RPC_FAR *pnDay,
    /* [out] */ short __RPC_FAR *pnHour,
    /* [out] */ short __RPC_FAR *pnMinute,
    /* [out] */ short __RPC_FAR *pnSecond,
    /* [out] */ short __RPC_FAR *pnMilliseconds);


void __RPC_STUB IUFile_GetCreationTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_GetSpaceData_Proxy( 
    IUFile __RPC_FAR * This,
    /* [out] */ long __RPC_FAR *plFreeBytesAvailableToCallerLowPart,
    /* [defaultvalue][out] */ long __RPC_FAR *plTotalNumberOfBytesLowPart,
    /* [defaultvalue][out] */ long __RPC_FAR *plTotalNumberOfFreeBytesLowPart,
    /* [defaultvalue][out] */ long __RPC_FAR *plFreeBytesAvailableToCallerHighPart,
    /* [defaultvalue][out] */ long __RPC_FAR *plTotalNumberOfBytesHighPart,
    /* [defaultvalue][out] */ long __RPC_FAR *plTotalNumberOfFreeBytesHighPart);


void __RPC_STUB IUFile_GetSpaceData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_GetFileAttributes_Proxy( 
    IUFile __RPC_FAR * This,
    /* [in] */ BSTR bstrFileName);


void __RPC_STUB IUFile_GetFileAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_Cancel_Proxy( 
    IUFile __RPC_FAR * This);


void __RPC_STUB IUFile_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_GetFilePart_Proxy( 
    IUFile __RPC_FAR * This,
    /* [defaultvalue][out] */ BSTR __RPC_FAR *pbstrFullFile,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IUFile_GetFilePart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUFile_FileSize_Proxy( 
    IUFile __RPC_FAR * This,
    /* [defaultvalue][out] */ long __RPC_FAR *plFileSizeHigh,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IUFile_FileSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUFile_get_DriveType_Proxy( 
    IUFile __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IUFile_get_DriveType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUFile_get_FilePointer_Proxy( 
    IUFile __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IUFile_get_FilePointer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUFile_get_FileNameOrDirectory_Proxy( 
    IUFile __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IUFile_get_FileNameOrDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUFile_get_FileAttributes_Proxy( 
    IUFile __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IUFile_get_FileAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUFile_get_RootDirectory_Proxy( 
    IUFile __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IUFile_get_RootDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUFile_get_Data_Proxy( 
    IUFile __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *pVal);


void __RPC_STUB IUFile_get_Data_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IUFile_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_UFile;

#ifdef __cplusplus

class DECLSPEC_UUID("A15E2D3F-7739-4063-B8FC-7D91D650FAA4")
UFile;
#endif
#endif /* __UFILELib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


