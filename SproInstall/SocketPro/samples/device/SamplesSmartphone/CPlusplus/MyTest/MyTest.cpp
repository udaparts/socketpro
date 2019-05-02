// MyTest.cpp : Defines the entry point for the application.
//

#include <windows.h>
#include <windowsx.h>
#include <aygshell.h>
#include "resource.h"

#define _ATL_DLL

#include "TOne.h"

HINSTANCE g_hInst = NULL;  // Local copy of hInstance

#define ARRAYSIZE(a)   (sizeof(a)/sizeof(*a))

const TCHAR* g_szAppWndClass = TEXT("HelloApp");

TCHAR g_szMessage[30];

/**************************************************************************************

   OnCreate

 **************************************************************************************/
LRESULT OnCreate(
    HWND hwnd,
    CREATESTRUCT* lParam
    )
{
    // create the menu bar
    SHMENUBARINFO mbi;
    ZeroMemory(&mbi, sizeof(SHMENUBARINFO));
    mbi.cbSize = sizeof(SHMENUBARINFO);
    mbi.hwndParent = hwnd;
    mbi.nToolBarId = IDR_HELLO_MENUBAR;
    mbi.hInstRes = g_hInst;
    if(!SHCreateMenuBar(&mbi))
    {
        // Couldn't create the menu bar.  Fail creation of the window.
        return(-1);
    }

    // Get our message text.
    if(0 == LoadString(g_hInst, IDS_HELLO_MESSAGE, g_szMessage, ARRAYSIZE(g_szMessage)))
    {
        // Couldn't load the string.  Fail creation of the window.
        return(-1);
    }

    // Do other window creation related things here.

    return(0); // continue creation of the window
}

/**************************************************************************************

   WndProc

 **************************************************************************************/
LRESULT CALLBACK WndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wp,
    LPARAM lp
    )
{
    LRESULT lResult = TRUE;

    switch(msg)
    {
        case WM_CREATE:
		{
            lResult = OnCreate(hwnd, (CREATESTRUCT*)lp);
		}
            break;

        case WM_COMMAND:
            switch (wp)
            {
                case IDOK:
                    DestroyWindow(hwnd);
                    break;
                default:
                    goto DoDefault;
            }
            break;

        case WM_PAINT:
            {
                HDC hdc;
                PAINTSTRUCT ps;
                RECT rect;

                hdc = BeginPaint(hwnd, &ps);
                GetClientRect(hwnd, &rect);

                DrawText(hdc, g_szMessage, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
                EndPaint (hwnd, &ps);
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        DoDefault:
        default:
            lResult = DefWindowProc(hwnd, msg, wp, lp);
            break;
    }

    return(lResult);
}

/****************************************************************************

   ActivatePreviousInstance

  ****************************************************************************/
HRESULT ActivatePreviousInstance(
    const TCHAR* pszClass,
    const TCHAR* pszTitle,
    BOOL* pfActivated
    )
{
    HRESULT hr = S_OK;
    int cTries;
    HANDLE hMutex = NULL;

    *pfActivated = FALSE;
    cTries = 5;
    while(cTries > 0)
    {
        hMutex = CreateMutex(NULL, FALSE, pszClass); // NOTE: We don't want to own the object.
        if(NULL == hMutex)
        {
            // Something bad happened, fail.
            hr = E_FAIL;
            goto Exit;
        }

        if(GetLastError() == ERROR_ALREADY_EXISTS)
        {
            HWND hwnd;

            CloseHandle(hMutex);
            hMutex = NULL;

            // There is already an instance of this app
            // running.  Try to bring it to the foreground.

            hwnd = FindWindow(pszClass, pszTitle);
            if(NULL == hwnd)
            {
                // It's possible that the other window is in the process of being created...
                Sleep(500);
                hwnd = FindWindow(pszClass, pszTitle);
            }

            if(NULL != hwnd) 
            {
                // Set the previous instance as the foreground window

                // The "| 0x01" in the code below activates
                // the correct owned window of the
                // previous instance's main window.
                SetForegroundWindow((HWND) (((ULONG) hwnd) | 0x01));

                // We are done.
                *pfActivated = TRUE;
                break;
            }

            // It's possible that the instance we found isn't coming up,
            // but rather is going down.  Try again.
            cTries--;
        }
        else
        {
            // We were the first one to create the mutex
            // so that makes us the main instance.  'leak'
            // the mutex in this function so it gets cleaned
            // up by the OS when this instance exits.
            break;
        }
    }

    if(cTries <= 0)
    {
        // Someone else owns the mutex but we cannot find
        // their main window to activate.
        hr = E_FAIL;
        goto Exit;
    }

Exit:
    return(hr);
}


/*****************************************************************************

  WinMain

  ***************************************************************************/

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine,
    int nCmdShow
    )
{
    MSG msg;
    HWND hwnd = NULL;
    BOOL fActivated;
    WNDCLASS wc;
    HWND hwndMain;
    TCHAR szAppTitle[20];

	HRESULT hr = ::CoInitializeEx(0, COINIT_MULTITHREADED /*COINIT_APARTMENTTHREADED*/);

	CSocketPool<CTOne> poolTOne;
	if(poolTOne.StartSocketPool(CComBSTR("192.168.1.100"), 20901, CComBSTR("SocketPro"), CComBSTR("PassOne"), 1))
	{
		CTOne *pTone = poolTOne.Lock(0);
		if(pTone != NULL)
		{
			int nOne;
			int nTwo;
			int nThree;

			CComVariant vtInput(L"This is a test");
			CComVariant vtOut;
			vtOut = pTone->Echo(vtInput);
			ATLASSERT(vtOut == vtInput);

			pTone->GetAllCounts(nOne, nTwo, nThree);
			poolTOne.Unlock(pTone);
		}
	}

    g_hInst = hInstance;

    if(0 == LoadString(g_hInst, IDS_HELLO_TITLE, szAppTitle, ARRAYSIZE(szAppTitle)))
    {
        return(0);
    }

    if(FAILED(ActivatePreviousInstance(g_szAppWndClass, szAppTitle, &fActivated)) ||
            fActivated)
    {
        return(0);
    }

    // Register our main window's class.
    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_HREDRAW | CS_VREDRAW ;
    wc.lpfnWndProc = (WNDPROC)WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hIcon = NULL;
    wc.hInstance = g_hInst;
    wc.hCursor = NULL;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = g_szAppWndClass;
    if(!RegisterClass(&wc))
    {
        return(0);
    }

    // Create the main window.    
    hwndMain = CreateWindow(g_szAppWndClass, szAppTitle,
            WS_CLIPCHILDREN, // Setting this to 0 gives a default style we don't want.  Use a benign style bit instead.
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, NULL, g_hInst, NULL );
    if(!hwndMain)
    {
        return(0);
    }

    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);

    // Pump messages until a PostQuitMessage.
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage (&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

// end MyTest.cpp
