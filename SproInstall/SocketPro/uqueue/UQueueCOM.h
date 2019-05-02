
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 5.03.0280 */
/* at Thu Oct 07 22:11:57 2004
 */
/* Compiler settings for E:\uskt\uqueue\UQueue.idl:
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

#ifndef __UQueue_h__
#define __UQueue_h__

/* Forward Declarations */ 

#ifndef __IUObjBase_FWD_DEFINED__
#define __IUObjBase_FWD_DEFINED__
typedef interface IUObjBase IUObjBase;
#endif 	/* __IUObjBase_FWD_DEFINED__ */


#ifndef ___IURequestEvent_FWD_DEFINED__
#define ___IURequestEvent_FWD_DEFINED__
typedef interface _IURequestEvent _IURequestEvent;
#endif 	/* ___IURequestEvent_FWD_DEFINED__ */


#ifndef __IUQueue_FWD_DEFINED__
#define __IUQueue_FWD_DEFINED__
typedef interface IUQueue IUQueue;
#endif 	/* __IUQueue_FWD_DEFINED__ */


#ifndef __UQueue_FWD_DEFINED__
#define __UQueue_FWD_DEFINED__

#ifdef __cplusplus
typedef class UQueue UQueue;
#else
typedef struct UQueue UQueue;
#endif /* __cplusplus */

#endif 	/* __UQueue_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __UQUEUELib_LIBRARY_DEFINED__
#define __UQUEUELib_LIBRARY_DEFINED__

/* library UQUEUELib */
/* [helpstring][version][uuid] */ 


enum tagUQueueError
    {	ueUnknown	= -1,
	ueSocketNotAttached	= 0x81000001,
	ueNewSizeLargerThanBufferSize	= 0x81000002,
	ueAllocatingMemoryFailed	= 0x81000003,
	ueDataSizeWrong	= 0x81000004,
	ueNotBytes	= 0x81000005
    };

EXTERN_C const IID LIBID_UQUEUELib;

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


#ifndef __IUQueue_INTERFACE_DEFINED__
#define __IUQueue_INTERFACE_DEFINED__

/* interface IUQueue */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IUQueue;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BBC93725-730D-499B-92E9-7AF0EA3589EB")
    IUQueue : public IUObjBase
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SendRequest( 
            /* [in] */ short sRequestID) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ReallocBuffer( 
            /* [in] */ long lNewSize) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SpeakEx( 
            /* [defaultvalue][in] */ long lGroups = -1) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SpeakToEx( 
            /* [in] */ BSTR bstrIPAddr,
            /* [in] */ long lPort) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Push( 
            /* [in] */ VARIANT vtData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Pop( 
            /* [retval][out] */ VARIANT __RPC_FAR *pvtData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PushInteger( 
            /* [in] */ short sData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PopInteger( 
            /* [retval][out] */ short __RPC_FAR *psData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PushLong( 
            /* [in] */ long lData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PopLong( 
            /* [retval][out] */ long __RPC_FAR *plData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PushString( 
            /* [in] */ BSTR bstrData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PopString( 
            /* [in] */ long lLenInByte,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PushFloat( 
            /* [in] */ float fData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PopFloat( 
            /* [retval][out] */ float __RPC_FAR *pfData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PushDate( 
            /* [in] */ DATE dtData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PopDate( 
            /* [retval][out] */ DATE __RPC_FAR *pdtData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PushDouble( 
            /* [in] */ double ddata) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PopDouble( 
            /* [retval][out] */ double __RPC_FAR *pdData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PushBoolean( 
            /* [in] */ VARIANT_BOOL bData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PopBoolean( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PushCurrency( 
            /* [in] */ CURRENCY cyData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PopCurrency( 
            /* [retval][out] */ CURRENCY __RPC_FAR *pcyData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PushBytes( 
            /* [in] */ VARIANT vtBytes) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PopBytes( 
            /* [in] */ long lLen,
            /* [retval][out] */ VARIANT __RPC_FAR *pvtBytes) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Size( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Size( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BufferSize( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PushDecimal( 
            /* [in] */ DECIMAL dcData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PopDecimal( 
            /* [retval][out] */ DECIMAL __RPC_FAR *pdcData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PushByte( 
            /* [in] */ BYTE bData) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PopByte( 
            /* [retval][out] */ BYTE __RPC_FAR *pbData) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUQueueVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IUQueue __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IUQueue __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IUQueue __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AttachSocket )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pIUnknownToUSocket);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Rtn )( 
            IUQueue __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plResult);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ErrorMsg )( 
            IUQueue __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendRequest )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ short sRequestID);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReallocBuffer )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ long lNewSize);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SpeakEx )( 
            IUQueue __RPC_FAR * This,
            /* [defaultvalue][in] */ long lGroups);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SpeakToEx )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ BSTR bstrIPAddr,
            /* [in] */ long lPort);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Push )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ VARIANT vtData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Pop )( 
            IUQueue __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *pvtData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PushInteger )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ short sData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PopInteger )( 
            IUQueue __RPC_FAR * This,
            /* [retval][out] */ short __RPC_FAR *psData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PushLong )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ long lData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PopLong )( 
            IUQueue __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *plData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PushString )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ BSTR bstrData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PopString )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ long lLenInByte,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PushFloat )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ float fData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PopFloat )( 
            IUQueue __RPC_FAR * This,
            /* [retval][out] */ float __RPC_FAR *pfData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PushDate )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ DATE dtData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PopDate )( 
            IUQueue __RPC_FAR * This,
            /* [retval][out] */ DATE __RPC_FAR *pdtData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PushDouble )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ double ddata);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PopDouble )( 
            IUQueue __RPC_FAR * This,
            /* [retval][out] */ double __RPC_FAR *pdData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PushBoolean )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL bData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PopBoolean )( 
            IUQueue __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PushCurrency )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ CURRENCY cyData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PopCurrency )( 
            IUQueue __RPC_FAR * This,
            /* [retval][out] */ CURRENCY __RPC_FAR *pcyData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PushBytes )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ VARIANT vtBytes);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PopBytes )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ long lLen,
            /* [retval][out] */ VARIANT __RPC_FAR *pvtBytes);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Size )( 
            IUQueue __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Size )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_BufferSize )( 
            IUQueue __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PushDecimal )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ DECIMAL dcData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PopDecimal )( 
            IUQueue __RPC_FAR * This,
            /* [retval][out] */ DECIMAL __RPC_FAR *pdcData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PushByte )( 
            IUQueue __RPC_FAR * This,
            /* [in] */ BYTE bData);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PopByte )( 
            IUQueue __RPC_FAR * This,
            /* [retval][out] */ BYTE __RPC_FAR *pbData);
        
        END_INTERFACE
    } IUQueueVtbl;

    interface IUQueue
    {
        CONST_VTBL struct IUQueueVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUQueue_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUQueue_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUQueue_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUQueue_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IUQueue_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IUQueue_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IUQueue_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IUQueue_AttachSocket(This,pIUnknownToUSocket)	\
    (This)->lpVtbl -> AttachSocket(This,pIUnknownToUSocket)

#define IUQueue_get_Rtn(This,plResult)	\
    (This)->lpVtbl -> get_Rtn(This,plResult)

#define IUQueue_get_ErrorMsg(This,pVal)	\
    (This)->lpVtbl -> get_ErrorMsg(This,pVal)


#define IUQueue_SendRequest(This,sRequestID)	\
    (This)->lpVtbl -> SendRequest(This,sRequestID)

#define IUQueue_ReallocBuffer(This,lNewSize)	\
    (This)->lpVtbl -> ReallocBuffer(This,lNewSize)

#define IUQueue_SpeakEx(This,lGroups)	\
    (This)->lpVtbl -> SpeakEx(This,lGroups)

#define IUQueue_SpeakToEx(This,bstrIPAddr,lPort)	\
    (This)->lpVtbl -> SpeakToEx(This,bstrIPAddr,lPort)

#define IUQueue_Push(This,vtData)	\
    (This)->lpVtbl -> Push(This,vtData)

#define IUQueue_Pop(This,pvtData)	\
    (This)->lpVtbl -> Pop(This,pvtData)

#define IUQueue_PushInteger(This,sData)	\
    (This)->lpVtbl -> PushInteger(This,sData)

#define IUQueue_PopInteger(This,psData)	\
    (This)->lpVtbl -> PopInteger(This,psData)

#define IUQueue_PushLong(This,lData)	\
    (This)->lpVtbl -> PushLong(This,lData)

#define IUQueue_PopLong(This,plData)	\
    (This)->lpVtbl -> PopLong(This,plData)

#define IUQueue_PushString(This,bstrData)	\
    (This)->lpVtbl -> PushString(This,bstrData)

#define IUQueue_PopString(This,lLenInByte,pbstrData)	\
    (This)->lpVtbl -> PopString(This,lLenInByte,pbstrData)

#define IUQueue_PushFloat(This,fData)	\
    (This)->lpVtbl -> PushFloat(This,fData)

#define IUQueue_PopFloat(This,pfData)	\
    (This)->lpVtbl -> PopFloat(This,pfData)

#define IUQueue_PushDate(This,dtData)	\
    (This)->lpVtbl -> PushDate(This,dtData)

#define IUQueue_PopDate(This,pdtData)	\
    (This)->lpVtbl -> PopDate(This,pdtData)

#define IUQueue_PushDouble(This,ddata)	\
    (This)->lpVtbl -> PushDouble(This,ddata)

#define IUQueue_PopDouble(This,pdData)	\
    (This)->lpVtbl -> PopDouble(This,pdData)

#define IUQueue_PushBoolean(This,bData)	\
    (This)->lpVtbl -> PushBoolean(This,bData)

#define IUQueue_PopBoolean(This,pbData)	\
    (This)->lpVtbl -> PopBoolean(This,pbData)

#define IUQueue_PushCurrency(This,cyData)	\
    (This)->lpVtbl -> PushCurrency(This,cyData)

#define IUQueue_PopCurrency(This,pcyData)	\
    (This)->lpVtbl -> PopCurrency(This,pcyData)

#define IUQueue_PushBytes(This,vtBytes)	\
    (This)->lpVtbl -> PushBytes(This,vtBytes)

#define IUQueue_PopBytes(This,lLen,pvtBytes)	\
    (This)->lpVtbl -> PopBytes(This,lLen,pvtBytes)

#define IUQueue_get_Size(This,pVal)	\
    (This)->lpVtbl -> get_Size(This,pVal)

#define IUQueue_put_Size(This,newVal)	\
    (This)->lpVtbl -> put_Size(This,newVal)

#define IUQueue_get_BufferSize(This,pVal)	\
    (This)->lpVtbl -> get_BufferSize(This,pVal)

#define IUQueue_PushDecimal(This,dcData)	\
    (This)->lpVtbl -> PushDecimal(This,dcData)

#define IUQueue_PopDecimal(This,pdcData)	\
    (This)->lpVtbl -> PopDecimal(This,pdcData)

#define IUQueue_PushByte(This,bData)	\
    (This)->lpVtbl -> PushByte(This,bData)

#define IUQueue_PopByte(This,pbData)	\
    (This)->lpVtbl -> PopByte(This,pbData)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_SendRequest_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [in] */ short sRequestID);


void __RPC_STUB IUQueue_SendRequest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_ReallocBuffer_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [in] */ long lNewSize);


void __RPC_STUB IUQueue_ReallocBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_SpeakEx_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [defaultvalue][in] */ long lGroups);


void __RPC_STUB IUQueue_SpeakEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_SpeakToEx_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [in] */ BSTR bstrIPAddr,
    /* [in] */ long lPort);


void __RPC_STUB IUQueue_SpeakToEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_Push_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [in] */ VARIANT vtData);


void __RPC_STUB IUQueue_Push_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_Pop_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *pvtData);


void __RPC_STUB IUQueue_Pop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PushInteger_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [in] */ short sData);


void __RPC_STUB IUQueue_PushInteger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PopInteger_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [retval][out] */ short __RPC_FAR *psData);


void __RPC_STUB IUQueue_PopInteger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PushLong_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [in] */ long lData);


void __RPC_STUB IUQueue_PushLong_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PopLong_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *plData);


void __RPC_STUB IUQueue_PopLong_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PushString_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [in] */ BSTR bstrData);


void __RPC_STUB IUQueue_PushString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PopString_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [in] */ long lLenInByte,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrData);


void __RPC_STUB IUQueue_PopString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PushFloat_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [in] */ float fData);


void __RPC_STUB IUQueue_PushFloat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PopFloat_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [retval][out] */ float __RPC_FAR *pfData);


void __RPC_STUB IUQueue_PopFloat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PushDate_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [in] */ DATE dtData);


void __RPC_STUB IUQueue_PushDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PopDate_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [retval][out] */ DATE __RPC_FAR *pdtData);


void __RPC_STUB IUQueue_PopDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PushDouble_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [in] */ double ddata);


void __RPC_STUB IUQueue_PushDouble_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PopDouble_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [retval][out] */ double __RPC_FAR *pdData);


void __RPC_STUB IUQueue_PopDouble_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PushBoolean_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL bData);


void __RPC_STUB IUQueue_PushBoolean_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PopBoolean_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pbData);


void __RPC_STUB IUQueue_PopBoolean_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PushCurrency_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [in] */ CURRENCY cyData);


void __RPC_STUB IUQueue_PushCurrency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PopCurrency_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [retval][out] */ CURRENCY __RPC_FAR *pcyData);


void __RPC_STUB IUQueue_PopCurrency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PushBytes_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [in] */ VARIANT vtBytes);


void __RPC_STUB IUQueue_PushBytes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PopBytes_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [in] */ long lLen,
    /* [retval][out] */ VARIANT __RPC_FAR *pvtBytes);


void __RPC_STUB IUQueue_PopBytes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUQueue_get_Size_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IUQueue_get_Size_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IUQueue_put_Size_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IUQueue_put_Size_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IUQueue_get_BufferSize_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IUQueue_get_BufferSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PushDecimal_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [in] */ DECIMAL dcData);


void __RPC_STUB IUQueue_PushDecimal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PopDecimal_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [retval][out] */ DECIMAL __RPC_FAR *pdcData);


void __RPC_STUB IUQueue_PopDecimal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PushByte_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [in] */ BYTE bData);


void __RPC_STUB IUQueue_PushByte_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IUQueue_PopByte_Proxy( 
    IUQueue __RPC_FAR * This,
    /* [retval][out] */ BYTE __RPC_FAR *pbData);


void __RPC_STUB IUQueue_PopByte_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IUQueue_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_UQueue;

#ifdef __cplusplus

class DECLSPEC_UUID("22ADED25-BB1D-49BF-8C96-40757FB7A417")
UQueue;
#endif
#endif /* __UQUEUELib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


