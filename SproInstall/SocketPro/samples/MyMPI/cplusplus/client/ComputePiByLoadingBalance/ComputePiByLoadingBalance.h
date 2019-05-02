// ComputePiByLoadingBalance.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CComputePiByLoadingBalanceApp:
// See ComputePiByLoadingBalance.cpp for the implementation of this class
//

class CComputePiByLoadingBalanceApp : public CWinApp
{
public:
	CComputePiByLoadingBalanceApp();
	virtual ~CComputePiByLoadingBalanceApp();

// Overrides
public:
	virtual BOOL InitInstance();

protected:
	virtual int ExitInstance();


// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CComputePiByLoadingBalanceApp theApp;