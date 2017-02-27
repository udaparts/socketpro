Imports System
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide

Public Class CMySocketProServer
    Inherits CSocketProServer

    Protected Overrides Function OnSettingServer() As Boolean
        'amIntegrated and amMixed not supported yet
        Config.AuthenticationMethod = tagAuthenticationMethod.amOwn

        PushManager.AddAChatGroup(1, "R&D Department")
        PushManager.AddAChatGroup(2, "Sales Department")
        PushManager.AddAChatGroup(3, "Management Department")

        Return True 'true -- ok; false -- no listening server
    End Function

    <ServiceAttr(hwConst.sidHelloWorld)> _
    Private m_HelloWorld As New CSocketProService(Of HelloWorldPeer)()
    'One SocketPro server supports any number of services. You can list them here!

    Shared Sub Main(ByVal args() As String)
        Dim MySocketProServer As New CMySocketProServer()

        'test certificate and private key files are located at ../SocketProRoot/bin
        MySocketProServer.UseSSL("intermediate.pfx", "", "mypassword")
        'MySocketProServer.UseSSL("root"/*"my"*/, "UDAParts Intermediate CA", "") 'or load cert and private key from windows system cert store

        If Not MySocketProServer.Run(20901) Then
            Console.WriteLine("Error code = " & CSocketProServer.LastSocketError.ToString())
        End If
        Console.WriteLine("Input a line to close the application ......")
        Console.ReadLine()
        MySocketProServer.StopSocketProServer() 'or MySocketProServer.Dispose();
    End Sub
End Class
