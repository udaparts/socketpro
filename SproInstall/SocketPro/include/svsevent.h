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

#ifndef __U_SOCKET_EVENT_SERVICE_1_H__
#define __U_SOCKET_EVENT_SERVICE_1_H__

#define	IDC_SRCSERVICEEVENT		1

static _ATL_FUNC_INFO OnSvsRequestProcessedInfo = {CC_STDCALL, VT_I4, 5, {VT_I4, VT_I2, VT_I4, VT_I4, VT_I2}};

template <class CContainer>
class CClientServiceEvent : public IDispEventSimpleImpl<IDC_SRCSERVICEEVENT, CClientServiceEvent<CContainer>, &__uuidof(_IURequestEvent)>							
{
public:
BEGIN_SINK_MAP(CClientServiceEvent)
	SINK_ENTRY_INFO(IDC_SRCSERVICEEVENT, __uuidof(_IURequestEvent), 1, OnSvsRequestProcessed, &OnSvsRequestProcessedInfo)
END_SINK_MAP()
	virtual HRESULT __stdcall OnSvsRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag)
	{
		return m_pContainer->OnSvsRequestProcessed(hSocket, nRequestID, lLen, lLenInBuffer, sFlag);
	}
public:
	CContainer	*m_pContainer;
};

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