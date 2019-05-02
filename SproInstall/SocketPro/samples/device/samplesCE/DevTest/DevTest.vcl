<html>
<body>
<pre>
<h1>Build Log</h1>
<h3>
--------------------Configuration: DevTest - Win32 (WCE ARMV4) Release--------------------
</h3>
<h3>Command Lines</h3>
Creating command line "rc.exe /l 0x409 /fo"ARMV4Rel/DevTest.res" /d UNDER_CE=420 /d _WIN32_WCE=420 /d "NDEBUG" /d "UNICODE" /d "_UNICODE" /d "WIN32_PLATFORM_PSPC=400" /d "ARM" /d "_ARM_" /d "ARMV4" /d "_AFXDLL" /r "E:\uskt\install\samplesCE\DevTest\DevTest.rc"" 
Creating temporary file "E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSPB5.tmp" with contents
[
/nologo /W3 /D "ARM" /D "_ARM_" /D "ARMV4" /D UNDER_CE=420 /D _WIN32_WCE=420 /D "WIN32_PLATFORM_PSPC=400" /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_AFXDLL" /FR"ARMV4Rel/" /Fp"ARMV4Rel/DevTest.pch" /Yu"stdafx.h" /Fo"ARMV4Rel/" /O2 /MC /c 
"E:\uskt\install\samplesCE\DevTest\DevTest.cpp"
"E:\uskt\install\samplesCE\DevTest\DevTestDlg.cpp"
"E:\uskt\install\include\sprowrap.cpp"
]
Creating command line "clarm.exe @E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSPB5.tmp" 
Creating temporary file "E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSPB6.tmp" with contents
[
/nologo /W3 /D "ARM" /D "_ARM_" /D "ARMV4" /D UNDER_CE=420 /D _WIN32_WCE=420 /D "WIN32_PLATFORM_PSPC=400" /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_AFXDLL" /FR"ARMV4Rel/" /Fp"ARMV4Rel/DevTest.pch" /Yc"stdafx.h" /Fo"ARMV4Rel/" /O2 /MC /c 
"E:\uskt\install\samplesCE\DevTest\StdAfx.cpp"
]
Creating command line "clarm.exe @E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSPB6.tmp" 
Creating temporary file "E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSPB7.tmp" with contents
[
/nologo /base:"0x00010000" /stack:0x10000,0x1000 /entry:"wWinMainCRTStartup" /incremental:no /pdb:"ARMV4Rel/DevTest.pdb" /out:"ARMV4Rel/DevTest.exe" /subsystem:windowsce,4.20 /align:"4096" /MACHINE:ARM 
.\ARMV4Rel\DevTest.obj
.\ARMV4Rel\DevTestDlg.obj
.\ARMV4Rel\sprowrap.obj
.\ARMV4Rel\StdAfx.obj
.\ARMV4Rel\DevTest.res
]
Creating command line "link.exe @E:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSPB7.tmp"
<h3>Output Window</h3>
Compiling resources...
Compiling...
StdAfx.cpp
Compiling...
DevTest.cpp
DevTestDlg.cpp
sprowrap.cpp
Generating Code...
Linking...




<h3>Results</h3>
DevTest.exe - 0 error(s), 0 warning(s)
</pre>
</body>
</html>
