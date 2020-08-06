from spa.serverside import CUQueue, CClientPeer as Cp,\
    CScopeUQueue as Sb, CSocketProServer as Sps
from msstruct import CMyStruct
import time


class CHelloWorldPeer(Cp):
    def OnSwitchFrom(self, old_service_id):
        self.Push.Subscribe((1, 3))

    def sayHello(self):
        assert(Sps.IsMainThread)
        fName = self.UQueue.LoadString()
        lName = self.UQueue.LoadString()
        # notify a message to groups [2, 3] at server side
        self.Push.Publish('Say hello from ' + fName + ' ' + lName, (2,3))
        res = u'Hello ' + fName + ' ' + lName
        print(res)
        return Sb().SaveString(res)

    def sleep(self):
        assert(not Sps.IsMainThread)
        ms = self.UQueue.LoadUInt()
        time.sleep(ms/1000.0)

    def echo(self):
        assert(Sps.IsMainThread)
        ms = CMyStruct()
        ms.LoadFrom(self.UQueue)
        return ms.SaveTo(Sb())

    def OnSubscribe(self, groups):
        print(self.UID + ' subscribes for groups ' + str(groups))

    def OnUnsubscribe(self, groups):
        print(self.UID + ' unsubscribes for groups ' + str(groups))

    def OnPublish(self, objMessage, groups):
        print(self.UID + ' publishes a message (' + str(objMessage) + ') to groups ' + str(groups))

    def OnSendUserMessage(self, receiver, objMessage):
        print(self.UID + ' sends a message (' + str(objMessage) + ') to ' + receiver)

    def OnSendUserMessageEx(self, receiver, message):
        print(self.UID + ' sends a message ex (' + CUQueue(message).LoadAString() + ') to ' + receiver)

    def OnPublishEx(self, groups, message):
        print(self.UID + ' publishes a message ex (' + CUQueue(message).LoadAString() + ') to groups ' + str(groups))
