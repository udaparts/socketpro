#include "udemo.h"
#include <atlbase.h>


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)  
{
    switch(fdwReason) 
    { 
    case DLL_PROCESS_ATTACH:
		{
			ATLTRACE("DLL_PROCESS_ATTACH called, udemo.dll\n");
		}
        break;
    case DLL_THREAD_ATTACH:
		{
			ATLTRACE("DLL_THREAD_ATTACH called, udemo.dll\n");
		}
        break;
    case DLL_THREAD_DETACH:
		{
			ATLTRACE("DLL_THREAD_DETACH called, udemo.dll\n");
		}
        break;
    case DLL_PROCESS_DETACH:
		{
			ATLTRACE("DLL_PROCESS_DETACH called, udemo.dll\n");
		}
        break;
	default:
		ATLASSERT(FALSE);
		break;
    }
    return TRUE;  
}