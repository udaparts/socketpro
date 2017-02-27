Imports System
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide

Public Class CMySocketProServer
	Inherits CSocketProServer

    Protected Overrides Function OnSettingServer() As Boolean
        Dim p As IntPtr = CSocketProServer.DllManager.AddALibrary("uasyncqueue", 16 * 1024) '16 * 1024 batch dequeuing size in bytes
        Return p.ToInt64 <> 0
    End Function

	Shared Sub Main(ByVal args() As String)
		If System.Environment.OSVersion.Platform = PlatformID.Unix Then
            CSocketProServer.QueueManager.WorkDirectory = "/home/yye/sp_test/"
		Else
            CSocketProServer.QueueManager.WorkDirectory = "c:\sp_test"
		End If

		Using MySocketProServer As New CMySocketProServer()
            'CSocketProServer.QueueManager.MessageQueuePassword = "MyPasswordForMsgQueue"

			If Not MySocketProServer.Run(20901) Then
				Console.WriteLine("Error code = " & CSocketProServer.LastSocketError.ToString())
			End If
			Console.WriteLine("Input a line to close the application ......")
			Console.ReadLine()
		End Using
	End Sub
End Class

