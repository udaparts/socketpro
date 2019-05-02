
@echo off
set STLROOT=C:\stlport521\STLport-5.2.1
cd %STL_ROOT%


set OSVERSION=WCE500
set PLATFORM=STANDARDSDK_500
set ARCH=ARMV4I
set SDKROOT=C:\Program Files (x86)\Windows CE Tools\wce500
set VSROOT=C:\Program Files (x86)\Microsoft Visual Studio 9.0
set BOOSTROOT=C:\boost_1_54_CE


call "%VSROOT%\VC\vcvarsall.bat"

set PATH=%VSROOT%\VC\ce\bin;%VSROOT%\VC\ce\bin\x86_arm;%VSROOT%\VC\bin;%VSROOT%\Common7\IDE;;%PATH%

set INCLUDE=%SDKROOT%\%PLATFORM%\include\%ARCH%;%SDKROOT%\%PLATFORM%\ATL\include;;%INCLUDE%

set LIB=%SDKROOT%\%PLATFORM%\lib\%ARCH%;%SDKROOT%\%PLATFORM%\ATL\lib\%ARCH%;%VSROOT%\VC\ce\lib\%ARCH%;;%LIB%

set CC=cl.exe
set TARGETCPU=ARMV4I
set CFG=none

call configure evc9 -x --with-static-rtl --use-boost %BOOSTROOT%

cd %STL_ROOT%/build/lib
nmake install