// This is a part of the SocketPro package.
// Copyright (C) 2000-2010 UDAParts 
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

#ifndef ___U_FILE_SERVER_H_____
#define ___U_FILE_SERVER_H_____

#define GET_FILE_BATCH_SIZE		(20*1460)
#include "usvrdll.h"

#ifdef __cplusplus
extern "C" {
#endif

void WINAPI SetWinFileReadOnly(unsigned int hSocket, bool bReadOnly = true);
bool WINAPI SetRootDirectory(unsigned int hSocket, const wchar_t *strRootDir);
void WINAPI SetMaxRequestSize(unsigned long ulMaxSize);

#ifdef __cplusplus
}
#endif

#endif

// This is a part of the SocketPro package.
// Copyright (C) 2000-2010 UDAParts 
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