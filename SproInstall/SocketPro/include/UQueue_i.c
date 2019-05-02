
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


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

#if !defined(_M_IA64) && !defined(_M_AXP64)

#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, LIBID_UQUEUELib,0xAE23C26C,0xCFA8,0x49A4,0x8D,0x76,0x08,0xE6,0x1C,0x9C,0x0A,0xC9);


MIDL_DEFINE_GUID(IID, IID_IUObjBase,0x20F08C2E,0x792C,0x10D6,0xB1,0xD1,0x00,0x10,0xB5,0xEC,0x08,0x5B);


MIDL_DEFINE_GUID(IID, DIID__IURequestEvent,0xF8000F37,0x147E,0x4C43,0xB8,0x89,0x1D,0x4C,0x9A,0x61,0x20,0x09);


MIDL_DEFINE_GUID(IID, IID_IUQueue,0xBBC93725,0x730D,0x499B,0x92,0xE9,0x7A,0xF0,0xEA,0x35,0x89,0xEB);


MIDL_DEFINE_GUID(CLSID, CLSID_UQueue,0x22ADED25,0xBB1D,0x49BF,0x8C,0x96,0x40,0x75,0x7F,0xB7,0xA4,0x17);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif /* !defined(_M_IA64) && !defined(_M_AXP64)*/


#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 5.03.0280 */
/* at Thu Oct 07 22:11:57 2004
 */
/* Compiler settings for E:\uskt\uqueue\UQueue.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win64 (32b run,appending), ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#if defined(_M_IA64) || defined(_M_AXP64)

#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, LIBID_UQUEUELib,0xAE23C26C,0xCFA8,0x49A4,0x8D,0x76,0x08,0xE6,0x1C,0x9C,0x0A,0xC9);


MIDL_DEFINE_GUID(IID, IID_IUObjBase,0x20F08C2E,0x792C,0x10D6,0xB1,0xD1,0x00,0x10,0xB5,0xEC,0x08,0x5B);


MIDL_DEFINE_GUID(IID, DIID__IURequestEvent,0xF8000F37,0x147E,0x4C43,0xB8,0x89,0x1D,0x4C,0x9A,0x61,0x20,0x09);


MIDL_DEFINE_GUID(IID, IID_IUQueue,0xBBC93725,0x730D,0x499B,0x92,0xE9,0x7A,0xF0,0xEA,0x35,0x89,0xEB);


MIDL_DEFINE_GUID(CLSID, CLSID_UQueue,0x22ADED25,0xBB1D,0x49BF,0x8C,0x96,0x40,0x75,0x7F,0xB7,0xA4,0x17);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif /* defined(_M_IA64) || defined(_M_AXP64)*/

