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

#ifndef __U_SOCKETPRO_SVS_H____
#define __U_SOCKETPRO_SVS_H____

#include "uscktpro.h"

#ifdef __cplusplus
extern "C" {
#endif

bool WINAPI InitServerLibrary(int nParam);
void WINAPI UninitServerLibrary();
unsigned short WINAPI GetNumOfServices();
unsigned long WINAPI GetAServiceID(unsigned short usIndex);
CSvsContext WINAPI GetOneSvsContext(unsigned long ulSvsID);
unsigned short WINAPI GetNumOfSlowRequests(unsigned long ulSvsID);
unsigned short WINAPI GetOneSlowRequestID(unsigned long ulSvsID, unsigned long ulIndex);

#ifdef __cplusplus
}
#endif

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