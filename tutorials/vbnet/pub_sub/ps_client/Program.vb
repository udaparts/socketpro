Imports System
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide

Friend Class Program
    Private Overloads Shared Function ToString(ByVal ms As CMessageSender) As String
        Return String.Format("Sender attributes = (ip = {0}, port = {1}, self = {2}, service id = {3}, userid = {4})", ms.IpAddress, ms.Port, ms.SelfMessage, ms.SvsID, ms.UserId)
    End Function

    Private Overloads Shared Function ToString(ByVal groups() As UInteger) As String
        Dim n As Integer = 0
        Dim s As String = "["
        For Each id As UInteger In groups
            If n <> 0 Then
                s &= ", "
            End If
            s &= id
            n += 1
        Next id
        s &= "]"
        Return s
    End Function

    Shared Sub Main(ByVal args() As String)
        Console.WriteLine("Input your user id ......")

        Dim cc As New CConnectionContext("localhost", 20901, Console.ReadLine(), "MyPassword", tagEncryptionMethod.TLSv1)
        'Dim cc As New CConnectionContext("localhost", 20901, Console.ReadLine(), "MyPassword")

        'for windows platforms, you can use windows system store instead
        CClientSocket.SSL.SetVerifyLocation("root") 'or "my", "my@currentuser", "root@localmachine"

        Using spHw As New CSocketPool(Of HelloWorld)() 'true -- automatic reconnecting
            AddHandler spHw.DoSslServerAuthentication, Function(sender, cs)
                                                           Dim errCode As Integer
                                                           Dim cert As IUcert = cs.UCert
                                                           Console.WriteLine(cert.SessionInfo)

                                                           'do ssl server certificate authentication here
                                                           Dim res As String = cert.Verify(errCode)

                                                           Return (errCode = 0) 'true -- user id and password will be sent to server
                                                       End Function

            'error handling ignored for code clarity
            Dim ok As Boolean = spHw.StartSocketPool(cc, 1, 1)
            Dim hw As HelloWorld = spHw.Seek() 'or HelloWorld hw = spHw.Lock();

            Dim ClientSocket As CClientSocket = hw.AttachedClientSocket
            AddHandler ClientSocket.Push.OnSubscribe, Sub(cs, messageSender, groups)
                                                          Console.WriteLine("Subscribe for " & ToString(groups))
                                                          Console.WriteLine(ToString(messageSender))
                                                          Console.WriteLine()
                                                      End Sub

            AddHandler ClientSocket.Push.OnUnsubscribe, Sub(cs, messageSender, groups)
                                                            Console.WriteLine("Unsubscribe from " & ToString(groups))
                                                            Console.WriteLine(ToString(messageSender))
                                                            Console.WriteLine()
                                                        End Sub

            AddHandler ClientSocket.Push.OnPublish, Sub(cs, messageSender, groups, msg)
                                                        Console.WriteLine("Publish to " & ToString(groups))
                                                        Console.WriteLine(ToString(messageSender))
                                                        Console.WriteLine("message = " & msg.ToString())
                                                        Console.WriteLine()
                                                    End Sub

            AddHandler ClientSocket.Push.OnSendUserMessage, Sub(cs, messageSender, msg)
                                                                Console.WriteLine("SendUserMessage")
                                                                Console.WriteLine(ToString(messageSender))
                                                                Console.WriteLine("message = " & msg.ToString())
                                                                Console.WriteLine()
                                                            End Sub

            'asynchronously process multiple requests with inline batching for best network efficiency
            ok = hw.SendRequest(hwConst.idSayHelloHelloWorld, "Jack", "Smith", Sub(ar)
                                                                                   Dim ret As String = Nothing
                                                                                   ar.Load(ret)
                                                                                   Console.WriteLine(ret)
                                                                               End Sub)

            Dim chat_ids() As UInteger = {1, 2}
            ok = ClientSocket.Push.Publish("We are going to call the method Sleep", chat_ids)
            Dim arh As CAsyncServiceHandler.DAsyncResultHandler = Nothing
            ok = hw.SendRequest(hwConst.idSleepHelloWorld, CInt(5000), arh)

            Console.WriteLine("Input a receiver for receiving my message ......")
            Console.WriteLine()
            ok = ClientSocket.Push.SendUserMessage("A message from " & cc.UserId, Console.ReadLine())
            ok = hw.WaitAll()

            Console.WriteLine("Press key ENTER to shutdown the demo application ......")
            Console.ReadLine()
        End Using
    End Sub
End Class

