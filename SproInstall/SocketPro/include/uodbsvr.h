// This is a part of the SocketPro package.
// Copyright (C) 2000-2004 UDAParts 
// All rights reserved.
//
// This source code is only intended as a supplement to the
// SocketPro package and related electronic documentation provided with the package.
// See these sources for detailed information regarding this
// UDAParts product.

// Please don't disclose any source code of the software to any person or entity,
//
// Please don't decompile, disassemble, or reverse engineer any object code of 
// any portion of the software.
//  
// http://www.udaparts.com/index.htm
// support@udaparts.com

#ifndef __U_OLEDB_SVR_H____
#define __U_OLEDB_SVR_H____

#include "usvrdll.h"


#ifdef __cplusplus
extern "C" {
#endif

bool WINAPI StartOneGlobalSession(const wchar_t* strConnectionStringToAnOLEDBProvider);
void WINAPI SetGlobalOLEDBConnectionString(const wchar_t* strOLEDBConnection);

//not implemented for the following two
void WINAPI SetOLEDBReadOnly(unsigned int hSocket, bool bReadOnly = true);
void WINAPI SetUseStorageObjectForBLOB(bool bStorageObjectForBLOB = true);
void WINAPI SetMaxRequestSize(unsigned long ulMaxSize);

#ifdef __cplusplus
}
#endif

#define WM_CLOSE_OLEDB_OBJECT_WITHIN_WORKER_THREAD	(WM_USER + 0x0402)

#endif

// This is a part of the SocketPro package.
// Copyright (C) 2000-2004 UDAParts 
// All rights reserved.
//
// This source code is only intended as a supplement to the
// SocketPro package and related electronic documentation provided with the package.
// See these sources for detailed information regarding this
// UDAParts product.

// Please don't disclose any source code of the software to any person or entity,
//
// Please don't decompile, disassemble, or reverse engineer any object code of 
// any portion of the software.
//  
// http://www.udaparts.com/index.htm
// support@udaparts.com