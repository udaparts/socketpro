<html>
<body>
<pre>
<h1>Build Log</h1>
<h3>
--------------------Configuration: CeDB - Win32 (WCE emulator) Release--------------------
</h3>
<h3>Command Lines</h3>
Creating temporary file "E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP113.tmp" with contents
[
/nologo /W3 /D "_i386_" /D UNDER_CE=420 /D _WIN32_WCE=420 /D "WIN32_PLATFORM_PSPC=400" /D "UNICODE" /D "_UNICODE" /D "_X86_" /D "x86" /D "NDEBUG" /D "_WIN32_WCE_CEPC" /D "_AFXDLL" /FR"emulatorRel/" /Fp"emulatorRel/CeDB.pch" /Yu"stdafx.h" /Fo"emulatorRel/" /Gs8192 /GF /O2 /c 
"E:\uskt\install\samplesCE\CeDB\CeDB.cpp"
"E:\uskt\install\samplesCE\CeDB\CeDBDlg.cpp"
"E:\uskt\install\include\sprowrap.cpp"
]
Creating command line "cl.exe @E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP113.tmp" 
Creating temporary file "E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP114.tmp" with contents
[
/nologo /W3 /D "_i386_" /D UNDER_CE=420 /D _WIN32_WCE=420 /D "WIN32_PLATFORM_PSPC=400" /D "UNICODE" /D "_UNICODE" /D "_X86_" /D "x86" /D "NDEBUG" /D "_WIN32_WCE_CEPC" /D "_AFXDLL" /FR"emulatorRel/" /Fp"emulatorRel/CeDB.pch" /Yc"stdafx.h" /Fo"emulatorRel/" /Gs8192 /GF /O2 /c 
"E:\uskt\install\samplesCE\CeDB\StdAfx.cpp"
]
Creating command line "cl.exe @E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP114.tmp" 
Creating temporary file "E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP115.tmp" with contents
[
/nologo /base:"0x00010000" /stack:0x20000,0x2000 /entry:"wWinMainCRTStartup" /incremental:no /pdb:"emulatorRel/CeDB.pdb" /out:"emulatorRel/CeDB.exe" /subsystem:windowsce,4.20 /MACHINE:IX86 
.\emulatorRel\CeDB.obj
.\emulatorRel\CeDBDlg.obj
.\emulatorRel\sprowrap.obj
.\emulatorRel\StdAfx.obj
.\emulatorRel\CeDB.res
]
Creating command line "link.exe @E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP115.tmp"
<h3>Output Window</h3>
Compiling...
StdAfx.cpp
Compiling...
CeDB.cpp
CeDBDlg.cpp
sprowrap.cpp
Generating Code...
Linking...





<h3>Results</h3>
CeDB.exe - 0 error(s), 0 warning(s)
<h3>
--------------------Configuration: CeDB - Win32 (WCE emulator) Debug--------------------
</h3>
<h3>Command Lines</h3>
Creating temporary file "E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP118.tmp" with contents
[
/nologo /W3 /Zi /Od /D "DEBUG" /D "_i386_" /D UNDER_CE=420 /D _WIN32_WCE=420 /D "WIN32_PLATFORM_PSPC=400" /D "UNICODE" /D "_UNICODE" /D "_X86_" /D "x86" /D "_WIN32_WCE_CEPC" /D "_AFXDLL" /FR"emulatorDbg/" /Fp"emulatorDbg/CeDB.pch" /Yu"stdafx.h" /Fo"emulatorDbg/" /Fd"emulatorDbg/" /Gs8192 /GF /c 
"E:\uskt\install\samplesCE\CeDB\CeDB.cpp"
"E:\uskt\install\samplesCE\CeDB\CeDBDlg.cpp"
"E:\uskt\install\include\sprowrap.cpp"
]
Creating command line "cl.exe @E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP118.tmp" 
Creating temporary file "E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP119.tmp" with contents
[
/nologo /W3 /Zi /Od /D "DEBUG" /D "_i386_" /D UNDER_CE=420 /D _WIN32_WCE=420 /D "WIN32_PLATFORM_PSPC=400" /D "UNICODE" /D "_UNICODE" /D "_X86_" /D "x86" /D "_WIN32_WCE_CEPC" /D "_AFXDLL" /FR"emulatorDbg/" /Fp"emulatorDbg/CeDB.pch" /Yc"stdafx.h" /Fo"emulatorDbg/" /Fd"emulatorDbg/" /Gs8192 /GF /c 
"E:\uskt\install\samplesCE\CeDB\StdAfx.cpp"
]
Creating command line "cl.exe @E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP119.tmp" 
Creating temporary file "E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP11A.tmp" with contents
[
/nologo /base:"0x00010000" /stack:0x20000,0x2000 /entry:"wWinMainCRTStartup" /incremental:yes /pdb:"emulatorDbg/CeDB.pdb" /debug /out:"emulatorDbg/CeDB.exe" /subsystem:windowsce,4.20 /MACHINE:IX86 
.\emulatorDbg\CeDB.obj
.\emulatorDbg\CeDBDlg.obj
.\emulatorDbg\sprowrap.obj
.\emulatorDbg\StdAfx.obj
.\emulatorDbg\CeDB.res
]
Creating command line "link.exe @E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP11A.tmp"
<h3>Output Window</h3>
Compiling...
StdAfx.cpp
Compiling...
CeDB.cpp
CeDBDlg.cpp
sprowrap.cpp
Generating Code...
Linking...





<h3>Results</h3>
CeDB.exe - 0 error(s), 0 warning(s)
<h3>
--------------------Configuration: CeDB - Win32 (WCE ARMV4) Release--------------------
</h3>
<h3>Command Lines</h3>
Creating temporary file "E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP11D.tmp" with contents
[
/nologo /W3 /D "ARM" /D "_ARM_" /D "ARMV4" /D UNDER_CE=420 /D _WIN32_WCE=420 /D "WIN32_PLATFORM_PSPC=400" /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_AFXDLL" /Fp"ARMV4Rel/CeDB.pch" /Yu"stdafx.h" /Fo"ARMV4Rel/" /O2 /MC /c 
"E:\uskt\install\samplesCE\CeDB\CeDB.cpp"
"E:\uskt\install\samplesCE\CeDB\CeDBDlg.cpp"
"E:\uskt\install\include\sprowrap.cpp"
]
Creating command line "clarm.exe @E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP11D.tmp" 
Creating temporary file "E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP11E.tmp" with contents
[
/nologo /W3 /D "ARM" /D "_ARM_" /D "ARMV4" /D UNDER_CE=420 /D _WIN32_WCE=420 /D "WIN32_PLATFORM_PSPC=400" /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_AFXDLL" /Fp"ARMV4Rel/CeDB.pch" /Yc"stdafx.h" /Fo"ARMV4Rel/" /O2 /MC /c 
"E:\uskt\install\samplesCE\CeDB\StdAfx.cpp"
]
Creating command line "clarm.exe @E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP11E.tmp" 
Creating temporary file "E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP11F.tmp" with contents
[
/nologo /base:"0x00010000" /stack:0x20000,0x10000 /entry:"wWinMainCRTStartup" /incremental:no /pdb:"ARMV4Rel/CeDB.pdb" /out:"ARMV4Rel/CeDB.exe" /subsystem:windowsce,4.20 /align:"4096" /MACHINE:ARM 
.\ARMV4Rel\CeDB.obj
.\ARMV4Rel\CeDBDlg.obj
.\ARMV4Rel\sprowrap.obj
.\ARMV4Rel\StdAfx.obj
.\ARMV4Rel\CeDB.res
]
Creating command line "link.exe @E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP11F.tmp"
<h3>Output Window</h3>
Compiling...
StdAfx.cpp
Compiling...
CeDB.cpp
CeDBDlg.cpp
sprowrap.cpp
Generating Code...
Linking...





<h3>Results</h3>
CeDB.exe - 0 error(s), 0 warning(s)
<h3>
--------------------Configuration: CeDB - Win32 (WCE ARMV4) Debug--------------------
</h3>
<h3>Command Lines</h3>
Creating temporary file "E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP122.tmp" with contents
[
/nologo /W3 /Zi /Od /D "DEBUG" /D "ARM" /D "_ARM_" /D "ARMV4" /D UNDER_CE=420 /D _WIN32_WCE=420 /D "WIN32_PLATFORM_PSPC=400" /D "UNICODE" /D "_UNICODE" /D "_AFXDLL" /FR"ARMV4Dbg/" /Fp"ARMV4Dbg/CeDB.pch" /Yu"stdafx.h" /Fo"ARMV4Dbg/" /Fd"ARMV4Dbg/" /MC /c 
"E:\uskt\install\samplesCE\CeDB\CeDB.cpp"
"E:\uskt\install\samplesCE\CeDB\CeDBDlg.cpp"
"E:\uskt\install\include\sprowrap.cpp"
]
Creating command line "clarm.exe @E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP122.tmp" 
Creating temporary file "E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP123.tmp" with contents
[
/nologo /W3 /Zi /Od /D "DEBUG" /D "ARM" /D "_ARM_" /D "ARMV4" /D UNDER_CE=420 /D _WIN32_WCE=420 /D "WIN32_PLATFORM_PSPC=400" /D "UNICODE" /D "_UNICODE" /D "_AFXDLL" /FR"ARMV4Dbg/" /Fp"ARMV4Dbg/CeDB.pch" /Yc"stdafx.h" /Fo"ARMV4Dbg/" /Fd"ARMV4Dbg/" /MC /c 
"E:\uskt\install\samplesCE\CeDB\StdAfx.cpp"
]
Creating command line "clarm.exe @E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP123.tmp" 
Creating temporary file "E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP124.tmp" with contents
[
/nologo /base:"0x00010000" /stack:0x20000,0x2000 /entry:"wWinMainCRTStartup" /incremental:yes /pdb:"ARMV4Dbg/CeDB.pdb" /debug /out:"ARMV4Dbg/CeDB.exe" /subsystem:windowsce,4.20 /align:"4096" /MACHINE:ARM 
.\ARMV4Dbg\CeDB.obj
.\ARMV4Dbg\CeDBDlg.obj
.\ARMV4Dbg\sprowrap.obj
.\ARMV4Dbg\StdAfx.obj
.\ARMV4Dbg\CeDB.res
]
Creating command line "link.exe @E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP124.tmp"
<h3>Output Window</h3>
Compiling...
StdAfx.cpp
Compiling...
CeDB.cpp
CeDBDlg.cpp
sprowrap.cpp
Generating Code...
Linking...





<h3>Results</h3>
CeDB.exe - 0 error(s), 0 warning(s)
</pre>
</body>
</html>
