Imports SocketProAdapter.ServerSide

Public Class CMySocketProServer
    Inherits CSocketProServer

    <ServiceAttr(radoConst.sidRAdo)> _
    Private m_RAdo As New CSocketProService(Of RAdoPeer)()
    'One SocketPro server supports any number of services. You can list them here!
End Class

Friend Class Program
    Shared Sub Main(ByVal args() As String)
        Using MySocketProServer As New CMySocketProServer()
            If Not MySocketProServer.Run(20901) Then
                Console.WriteLine("Error code = " & CSocketProServer.LastSocketError.ToString())
            End If
            Console.WriteLine("Input a line to close the application ......")
            Console.ReadLine()
        End Using
    End Sub
End Class

