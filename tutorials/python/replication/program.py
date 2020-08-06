from spa.clientside import *
from consts import hwConst
from msstruct import CMyStruct


class CHelloWorld(CAsyncServiceHandler):
    def __init__(self):
        super(CHelloWorld, self).__init__(hwConst.sidHelloWorld)


rs = ReplicationSetting()
with CReplication(CHelloWorld, rs) as rep:
    ConnQueue = {}
    cc = CConnectionContext("127.0.0.1", 20901, "replication", "p4localhost")
    ConnQueue["Tolocal"] = cc
    cc = CConnectionContext("192.168.1.122", 20901, "remote_rep", "PassOne")
    ConnQueue["ToLinux"] = cc
    ok = rep.Start(ConnQueue, "hw_root_queue_name")
    ok = rep.StartJob()
    ok = rep.Send(hwConst.idSayHelloHelloWorld, CUQueue().SaveString(u'Jack').SaveString(u'Smith'))
    ok = rep.Send(hwConst.idEchoHelloWorld, CUQueue().Save(CMyStruct.MakeOne()))
    ok = rep.EndJob()
    print('Read a line to continue ......')
    line = sys.stdin.readline()
