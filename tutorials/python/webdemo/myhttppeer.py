
from spa.serverside.httppeer import CHttpPeerBase
import time

class CMyHttpPeer(CHttpPeerBase):
    def OnSubscribe(self, groups):
        print(self.UID, ' subscribes for groups ', str(groups))

    def OnUnsubscribe(self, groups):
        print(self.UID, ' unsubscribes for groups ', str(groups))

    def OnPublish(self, objMessage, groups):
        print(self.UID, ' publishes a message (', str(objMessage), ') to groups ', str(groups))

    def OnSendUserMessage(self, receiver, objMessage):
        print(self.UID, ' sends a message (', str(objMessage), ') to ', receiver)

    def DoAuthentication(self, userId, password):
        self.Push.Subscribe([1, 2, 7])
        print('User id = ', userId, ', password = ', password)
        return True

    def OnGet(self):
        dot = self.Path.rfind('.')
        if dot != 1:
            self.DownloadFile(self.Path[1:])
        else:
            self.SendResult('test result --- GET ---')
    def OnPost(self):
        res = self.SendResult('+++ POST +++ test result')

    def Sleep(self, ms):
        time.sleep(float(ms)/1000.0)

    def SayHello(self, fName, lName):
        str = u'Say hello from ' + fName + u' ' + lName
        self.Push.Publish(str, [2,3])
        return u'Hello ' + fName + u' ' + lName

    def OnUserRequest(self):
        reqName = self.RequestName
        args = self.Args
        if reqName == 'sleep':
            ms = int(self.Args[0])
            self.Sleep(ms)
            self.SendResult('')
        elif reqName == 'sayHello':
            self.SendResult(self.SayHello(args[0], args[1]))
        else:
            self.SendResult('')