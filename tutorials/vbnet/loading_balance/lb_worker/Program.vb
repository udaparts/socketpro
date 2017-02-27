Imports System
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide

Friend Class Program
    Shared Sub Main(ByVal args() As String)
        Dim cc As New CConnectionContext("127.0.0.1", 20901, "lb_worker", "pwdForlb_worker")
        Using spPi As New CSocketPool(Of PiWorker)(True) 'true -- automatic reconnecting
            spPi.StartSocketPool(cc, 1)
            Console.WriteLine("Press key ENTER to shutdown the demo application ......")
            Console.ReadLine()
        End Using
    End Sub
End Class

