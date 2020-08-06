from spa.serverside import *
import sys

with CSocketProServer() as server:
    handle = CSocketProServer.DllManager.AddALibrary('uasyncqueue', 16 * 1024)  # 16 * 1024 dequeue batch size in bytes
    if handle == 0:
        print('Cannot load async queue server library')
    elif not server.Run(20901):
        print('Error message = ' + CSocketProServer.ErrorMessage)
    print('Read a line to shutdown the application ......')
    line = sys.stdin.readline()
