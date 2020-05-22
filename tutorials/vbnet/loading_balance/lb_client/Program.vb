Imports SocketProAdapter
Imports SocketProAdapter.ClientSide

Friend Class Program
    Shared Sub Main(ByVal args() As String)
        Dim ok As Boolean
        CClientSocket.QueueConfigure.MessageQueuePassword = "MyPwdForMsgQueue"
        If System.Environment.OSVersion.Platform = PlatformID.Unix Then
            CClientSocket.QueueConfigure.WorkDirectory = "/home/yye/sp_test/"
        Else
            CClientSocket.QueueConfigure.WorkDirectory = "c:\sp_test"
        End If
        Dim cc As New CConnectionContext("localhost", 20901, "lb_client", "pwd_lb_client")
        Using spPi As New CSocketPool(Of Pi)(True) 'true -- automatic reconnecting
            ok = spPi.StartSocketPool(cc, 1, 1)
            Dim cs As CClientSocket = spPi.Sockets(0)

            'use persistent queue to ensure auto failure recovery and at-least-once or once-only delivery
            ok = cs.ClientQueue.StartQueue("pi_queue", 24 * 3600, (cs.EncryptionMethod = tagEncryptionMethod.TLSv1))
            cs.ClientQueue.RoutingQueueIndex = True

            Dim pi As Pi = spPi.AsyncHandlers(0)
            pi.WaitAll() 'make sure all existing queued requests are processed before executing next requests

            Dim dPi As Double = 0.0
            Dim nDivision As Integer = 1000
            Dim nNum As Integer = 10000000
            Dim dStep As Double = 1.0 / nNum / nDivision
            Dim nReturns As Integer = 0
            For n As Integer = 0 To nDivision - 1
                Dim dStart As Double = CDbl(n) / nDivision
                ok = pi.SendRequest(piConst.idComputePi, dStart, dStep, nNum, Sub(ar)
                                                                                  Dim res As Double
                                                                                  ar.Load(res)
                                                                                  dPi += res
                                                                                  nReturns += 1
                                                                              End Sub)
            Next n
            ok = pi.WaitAll()
            Console.WriteLine("Your pi = {0}, returns = {1}", dPi, nReturns)
            Console.WriteLine("Press ENTER key to shutdown the demo application ......")
            Console.ReadLine()
        End Using
    End Sub
End Class

