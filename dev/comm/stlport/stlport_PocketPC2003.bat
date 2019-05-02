
@echo off
set STLROOT=C:\stlport521\STLport-5.2.1
cd %STL_ROOT%


set OSVERSION=WCE420
set PLATFORM=PocketPC2003
set ARCH=ARMV4
set SDKROOT=C:\Program Files (x86)\Microsoft Visual Studio 9.0\SmartDevices\SDK
set VSROOT=C:\Program Files (x86)\Microsoft Visual Studio 9.0
set BOOSTROOT=C:\boost_1_54_CE


call "%VSROOT%\VC\vcvarsall.bat"

set PATH=%VSROOT%\VC\ce\bin;%VSROOT%\VC\ce\bin\x86_arm;%VSROOT%\VC\bin;%VSROOT%\Common7\IDE;;%PATH%

set INCLUDE=%SDKROOT%\%PLATFORM%\include;%VSROOT%\vc\ce\atlmfc;;%INCLUDE%

set LIB=%SDKROOT%\%PLATFORM%\lib;%VSROOT%\VC\ce\lib\%ARCH%;;%LIB%

set CC=cl.exe
set TARGETCPU=ARMV4
set CFG=none

call configure evc9 -x --with-static-rtl --use-boost %BOOSTROOT%

cd %STL_ROOT%/build/lib
nmake install