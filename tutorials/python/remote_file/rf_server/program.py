
from spa.serverside import CSocketProServer
import sys
from ctypes import *
from sys import platform as os

_ussLib_ = None
_IsWin_ = (os == "win32")
if _IsWin_:
    _ussLib_ = WinDLL('ustreamfile.dll')
else:
    _ussLib_ = CDLL('libustreamfile.so')

# void WINAPI SetRootDirectory(const wchar_t *pathRoot);
SetRootDirectory = _ussLib_.SetRootDirectory
SetRootDirectory.argtypes = [c_wchar_p]
SetRootDirectory.restype = None

with CSocketProServer() as server:
    handle = CSocketProServer.DllManager.AddALibrary('ustreamfile')
    if handle:
        SetRootDirectory('C:\\boost_1_60_0\\stage\\lib64')
    ok = server.Run(20901)
    if not ok:
        print('Error message = ' + CSocketProServer.ErrorMessage)
    print('Read a line to shutdown the application ......')
    line = sys.stdin.readline()
