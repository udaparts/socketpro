
import sys
import datetime
from spa import CDataSet, CTable, CMasterPool
from webasynchandler import CWebAsyncHandler
from spa.clientside import CConnectionContext, CClientSocket

# CA file is located at the directory ../socketpro/bin
CClientSocket.SSL.SetVerifyLocation('ca.cert.pem')

with CMasterPool(CWebAsyncHandler, '', False) as sp:

    cc = CConnectionContext('localhost', 20901, 'PythonUser', 'TooMuchSecret')
    ok = sp.StartSocketPool(cc, 4, 1)
    cache = sp.Cache

    print('Press ENTER key to shutdown the demo application ......')
    line = sys.stdin.readline()
    res = 0

    