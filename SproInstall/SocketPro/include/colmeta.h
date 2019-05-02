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

#ifndef	__UDB_COLUMN_META_DATA_H__
#define __UDB_COLUMN_META_DATA_H__

#pragma pack(push,1)

struct CCellMeta
{
	unsigned long	m_ulOrdinal;
	unsigned long	m_ulBoundMaxLen;
	unsigned long	m_ulRawMaxLen;
	unsigned short	m_usBoundDBType;
	unsigned short	m_usRawDBType;
	unsigned long	m_ulFlags;
};

struct CColMeta : public CCellMeta
{
	unsigned long	m_ulBytePos;
	unsigned char	m_bBitPos;
};

#pragma pack(pop)

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