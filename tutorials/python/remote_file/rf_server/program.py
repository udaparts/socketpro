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

# bool U_MODULE_OPENED WINAPI SetSPluginGlobalOptions(const char *jsonUtf8Options);
SetSPluginGlobalOptions = _ussLib_.SetSPluginGlobalOptions
SetSPluginGlobalOptions.argtypes = [c_char_p]
SetSPluginGlobalOptions.restype = c_bool

with CSocketProServer() as server:
    ok = True
    handle = CSocketProServer.DllManager.AddALibrary('ustreamfile')
    if handle:
        ok = SetSPluginGlobalOptions('{"root_directory":"D:\\\\socketpro\\\\include"}'.encode('utf-8'))
    ok = server.Run(20901)
    if not ok:
        print('Error message = ' + CSocketProServer.ErrorMessage)
    print('Read a line to shutdown the application ......')
    line = sys.stdin.readline()
