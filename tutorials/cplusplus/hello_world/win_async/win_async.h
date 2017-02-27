
// win_async.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Cwin_asyncApp:
// See win_async.cpp for the implementation of this class
//

class Cwin_asyncApp : public CWinApp
{
public:
	Cwin_asyncApp();
	virtual ~Cwin_asyncApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern Cwin_asyncApp theApp;