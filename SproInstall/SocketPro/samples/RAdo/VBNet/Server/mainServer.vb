Imports SocketProAdapter
Imports SocketProAdapter.ServerSide

Namespace RAdoServer
	''' <summary>
	''' Summary description for Class1.
	''' </summary>
	Friend Class RAdoHost
		''' <summary>
		''' The main entry point for the application.
		''' </summary>
        Shared Sub Main(ByVal args() As String)
            Dim MySocketProServer As New CMySocketProServer()
            If Not MySocketProServer.Run(20901) Then
                Console.WriteLine("Error code = " & CSocketProServer.LastSocketError.ToString())
            End If
            Console.WriteLine("Input a line to close the application ......")
            Dim str As String = Console.ReadLine()
            MySocketProServer.StopSocketProServer()
        End Sub
	End Class
End Namespace
