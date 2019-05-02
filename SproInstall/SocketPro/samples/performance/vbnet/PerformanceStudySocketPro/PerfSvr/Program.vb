Imports System.Text


Namespace PerfSvr
	Friend Class Program
        Shared Sub Main(ByVal args() As String)
            Dim MySocketProServer As New CMySocketProServer()
            If Not MySocketProServer.Run(21911) Then
                Console.WriteLine("Error code = " & SocketProAdapter.ServerSide.CSocketProServer.LastSocketError.ToString())
            End If
            Console.WriteLine("Input a line to close the application ......")
            Dim str As String = Console.ReadLine()
            MySocketProServer.StopSocketProServer()
        End Sub
    End Class
End Namespace
