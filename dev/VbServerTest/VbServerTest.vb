Imports SocketProAdapter
Imports SocketProAdapter.ServerSide

Module VbServerTest

    Sub Main(ByVal args As String())
        Dim MySocketProServer As New CMySocketProServer()
        If Not MySocketProServer.Run(20901) Then
            Console.WriteLine("Error code = " & CSocketProServer.LastSocketError.ToString())
        End If
        Console.WriteLine("Input a line to close the application ......")
        Dim str As String = Console.ReadLine()
        MySocketProServer.StopSocketProServer() 'or MySocketProServer.Dispose()
    End Sub

End Module
