from hello_world.client.asynchelloworld import CHelloWorld
from consts import hwConst
from spa.clientside import *
import sys

print('Input your user id ......')
# cc = CConnectionContext('localhost', 20901, sys.stdin.readline().strip(), 'MyPassword', tagEncryptionMethod.TLSv1)

cc = CConnectionContext('localhost', 20901, sys.stdin.readline().strip(), 'MyPassword')

# CA file is located at the directory ..\SocketProRoot\bin
CClientSocket.SSL.SetVerifyLocation('ca.cert.pem')  # linux

# for windows platforms, you can use windows system store instead
# CClientSocket.SSL.SetVerifyLocation("my"); #or "root", "my@currentuser", "root@localmachine"

with CSocketPool(CHelloWorld) as sp:

    def OnCertVerification(sp, cs):
        print(cs.UCert.CertPem)
        print(cs.UCert.Verify())
        return True

    sp.DoSslServerAuthentication = OnCertVerification

    ok = sp.StartSocketPool(cc, 1)
    hw = sp.Seek()
    cs = hw.Socket

    def OnSubscribe(cs, sender, groups):
         print('Enter: groups = ' + str(groups) + ', sender id = ' + sender.UserId + ', ip address = ' + sender.IpAddress + ', port = ' + str(sender.Port) + ', ServiceId = ' + str(sender.ServiceId) + ', SelfMessage = ' + str(sender.SelfMessage))

    def OnUnsubscribe(cs, sender, groups):
         print('Exit: groups = ' + str(groups) + ', sender id = ' + sender.UserId + ', ip address = ' + sender.IpAddress + ', port = ' + str(sender.Port) + ', ServiceId = ' + str(sender.ServiceId) + ', SelfMessage = ' + str(sender.SelfMessage))

    def OnPublish(cs, sender, groups, objMsg):
        print('Publish: groups = ' + str(groups) + ' message = ' + objMsg + ', sender id = ' + sender.UserId + ', ip address = ' + sender.IpAddress + ', port = ' + str(sender.Port) + ', ServiceId = ' + str(sender.ServiceId) + ', SelfMessage = ' + str(sender.SelfMessage))

    def OnSendUserMessage(cs, sender, objMsg):
        print('SendUserMessage: message = ' + objMsg + ', sender id = ' + sender.UserId + ', ip address = ' + sender.IpAddress + ', port = ' + str(sender.Port) + ', ServiceId = ' + str(sender.ServiceId) + ', SelfMessage = ' + str(sender.SelfMessage))

    cs.Push.OnSubscribe = OnSubscribe
    cs.Push.OnPublish = OnPublish
    cs.Push.OnSendUserMessage = OnSendUserMessage
    cs.Push.OnUnsubscribe = OnUnsubscribe

    # asynchronously process all three requests with inline batching for best network efficiency
    def cbSayHello(ar):
        print(ar.LoadString())

    ok = hw.SendRequest(hwConst.idSayHelloHelloWorld, CUQueue().SaveString('Jack').SaveString('Smith'), cbSayHello)
    cs.Push.Publish('We are going to call the method Sleep', (1,2))
    hw.SendRequest(hwConst.idSleepHelloWorld, CUQueue().SaveUInt(5000), None)
    print('Input a receiver for receiving my message ......')
    cs.Push.SendUserMessage('A message from ' + cc.UserId, sys.stdin.readline().strip())
    print('Press key ENTER to shutdown the demo application ......')
    sys.stdin.readline()
