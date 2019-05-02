#ifndef _WIN_CRASH_HANDLERS_H
#define _WIN_CRASH_HANDLERS_H

#include <windows.h>

class CCrashHandler {
public:

    // Constructor
    CCrashHandler();

    // Destructor
    virtual ~CCrashHandler();

    // Sets exception handlers that work on per-process basis
    static void SetProcessExceptionHandlers();

    // Installs C++ exception handlers that function on per-thread basis
    static void SetThreadExceptionHandlers();

    // Collects current process state.
    static void GetExceptionPointers(
            DWORD dwExceptionCode,
            EXCEPTION_POINTERS** pExceptionPointers);

    // This method creates minidump of the process
    static void CreateMiniDump(EXCEPTION_POINTERS* pExcPtrs);

    /* Exception handler functions. */

    static LONG WINAPI SehHandler(PEXCEPTION_POINTERS pExceptionPtrs);
    static void __cdecl TerminateHandler();
    static void __cdecl UnexpectedHandler();

    static void __cdecl PureCallHandler();

    static void __cdecl InvalidParameterHandler(const wchar_t* expression,
            const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved);

    static int __cdecl NewHandler(size_t);

    static void SigabrtHandler(int);
    static void SigfpeHandler(int /*code*/, int subcode);
    static void SigintHandler(int);
    static void SigillHandler(int);
    static void SigsegvHandler(int);
    static void SigtermHandler(int);
};

#endif