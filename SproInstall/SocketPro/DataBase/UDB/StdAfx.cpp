// stdafx.cpp : source file that includes just the standard includes
//  stdafx.pch will be the pre-compiled header
//  stdafx.obj will contain the pre-compiled type information

// This is a part of the SocketPro package.
// Copyright (C) 2000-2008 UDAParts 
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

#include "stdafx.h"

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>

#include "UDB.h"

bool IsDataTypeSupported(unsigned short usDBType)
{
	switch(usDBType)
	{
	case sdVT_EMPTY:
	case sdVT_I2:
	case sdVT_I4:
	case sdVT_R4:
	case sdVT_R8:
	case sdVT_CY:
	case sdVT_DATE:
	case sdVT_BOOL:
	case sdVT_VARIANT:
	case sdVT_DECIMAL:
	case sdVT_I1:
	case sdVT_UI1:
	case sdVT_UI2:
	case sdVT_UI4:
	case sdVT_I8:
	case sdVT_UI8:
	case sdVT_BYTES:
	case sdVT_STR:
	case sdVT_WSTR:
		return true;
		break;
	default:
		break;
	}
	return false;
}

// This is a part of the SocketPro package.
// Copyright (C) 2000-2008 UDAParts 
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