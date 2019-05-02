
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


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

MIDL_DEFINE_GUID(IID, LIBID_UFILELib,0x4071FB2E,0xB7FC,0x4A14,0x97,0x19,0x3F,0xDF,0x4C,0xCA,0x47,0xF2);


MIDL_DEFINE_GUID(IID, IID_IUObjBase,0x20F08C2E,0x792C,0x10D6,0xB1,0xD1,0x00,0x10,0xB5,0xEC,0x08,0x5B);


MIDL_DEFINE_GUID(IID, DIID__IURequestEvent,0xF8000F37,0x147E,0x4C43,0xB8,0x89,0x1D,0x4C,0x9A,0x61,0x20,0x09);


MIDL_DEFINE_GUID(IID, IID_IUFile,0xDE98E3D3,0x699B,0x4AD1,0x82,0x87,0x34,0x81,0x97,0x2C,0xEB,0xB9);


MIDL_DEFINE_GUID(CLSID, CLSID_UFile,0xA15E2D3F,0x7739,0x4063,0xB8,0xFC,0x7D,0x91,0xD6,0x50,0xFA,0xA4);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif /* !defined(_M_IA64) && !defined(_M_AXP64)*/


#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 5.03.0280 */
/* at Sat Apr 24 08:32:44 2004
 */
/* Compiler settings for E:\uskt\UFile\UFile.idl:
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

MIDL_DEFINE_GUID(IID, LIBID_UFILELib,0x4071FB2E,0xB7FC,0x4A14,0x97,0x19,0x3F,0xDF,0x4C,0xCA,0x47,0xF2);


MIDL_DEFINE_GUID(IID, IID_IUObjBase,0x20F08C2E,0x792C,0x10D6,0xB1,0xD1,0x00,0x10,0xB5,0xEC,0x08,0x5B);


MIDL_DEFINE_GUID(IID, DIID__IURequestEvent,0xF8000F37,0x147E,0x4C43,0xB8,0x89,0x1D,0x4C,0x9A,0x61,0x20,0x09);


MIDL_DEFINE_GUID(IID, IID_IUFile,0xDE98E3D3,0x699B,0x4AD1,0x82,0x87,0x34,0x81,0x97,0x2C,0xEB,0xB9);


MIDL_DEFINE_GUID(CLSID, CLSID_UFile,0xA15E2D3F,0x7739,0x4063,0xB8,0xFC,0x7D,0x91,0xD6,0x50,0xFA,0xA4);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif /* defined(_M_IA64) || defined(_M_AXP64)*/

