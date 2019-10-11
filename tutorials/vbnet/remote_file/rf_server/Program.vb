Imports System.Runtime.InteropServices
Imports SocketProAdapter.ServerSide

Public Class CMySocketProServer
    Inherits CSocketProServer

    <DllImport("ustreamfile")> _
    Shared Sub SetRootDirectory(<MarshalAs(UnmanagedType.LPWStr)> ByVal root As String)
    End Sub

    Protected Overrides Function OnSettingServer() As Boolean
        Dim p As IntPtr = CSocketProServer.DllManager.AddALibrary("ustreamfile")
        If p.ToInt64 <> 0 Then
            SetRootDirectory("C:\boost_1_60_0\stage\lib64")
            Return True
        End If
        Return False
    End Function

    Shared Sub Main(ByVal args() As String)
        Dim MySocketProServer As New CMySocketProServer()
        If Not MySocketProServer.Run(20901) Then
            Console.WriteLine("Error code = " & CSocketProServer.LastSocketError.ToString())
        End If
        Console.WriteLine("Input a line to close the application ......")
        Console.ReadLine()
        MySocketProServer.StopSocketProServer() 'or MySocketProServer.Dispose();
    End Sub

End Class
