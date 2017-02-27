
Imports System
Imports System.Runtime.InteropServices
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide

Public Class CMySocketProServer
	Inherits CSocketProServer

    'SocketPro sqlite server defines, which can be found at ../socketpro/include/sqlite/usqlite_server.h
    Const ENABLE_GLOBAL_SQLITE_UPDATE_HOOK As Integer = 1
    Const USE_SHARED_CACHE_MODE As Integer = 2
    Const USE_UTF16_ENCODING As Integer = 4
    <DllImport("ssqlite")> _
    Shared Sub SetSqliteDBGlobalConnectionString(ByRef sqliteDbFile As String)
    End Sub

    <ServiceAttr(hwConst.sidHelloWorld)>
    Private m_HelloWorld As New CSocketProService(Of HelloWorldPeer)()

    'for db push from ms sql server
    <ServiceAttr(repConst.sidRAdoRep)>
    Private m_RAdoRep As New CSocketProService(Of DBPushPeer)()

    'Routing requires registering two services in pair
    <ServiceAttr(piConst.sidPi)>
    Private m_Pi As New CSocketProService(Of CClientPeer)()
    <ServiceAttr(piConst.sidPiWorker)>
    Private m_PiWorker As New CSocketProService(Of CClientPeer)()

    <ServiceAttr(radoConst.sidRAdo)>
    Private m_RAdo As New CSocketProService(Of RAdoPeer)()

    <ServiceAttr(RemFileConst.sidRemotingFile)>
    Private m_RemotingFile As New CSocketProService(Of RemotingFilePeer)()

    <ServiceAttr(BaseServiceID.sidHTTP)>
    Private m_http As New CSocketProService(Of CMyHttpPeer)()

    Shared Sub Main(ByVal args() As String)
        If System.Environment.OSVersion.Platform = PlatformID.Unix Then
            CSocketProServer.QueueManager.WorkDirectory = "/home/yye/sp_test/"
        Else
            CSocketProServer.QueueManager.WorkDirectory = "c:\sp_test"
        End If
        Using MySocketProServer As New CMySocketProServer()
            'CSocketProServer.QueueManager.MessageQueuePassword = "MyPasswordForMsgQueue"

            'test certificate, private key and DH params files are located at the directory ..\SocketProRoot\bin
            'MySocketProServer.UseSSL("server.pem", "server.pem", "test");

            If Not MySocketProServer.Run(20901) Then
                Console.WriteLine("Error code = " & CSocketProServer.LastSocketError.ToString())
            Else
                CSocketProServer.Router.SetRouting(piConst.sidPi, piConst.sidPiWorker)
            End If
            Console.WriteLine("Input a line to close the application ......")
            Console.ReadLine()
        End Using
    End Sub

    Protected Overrides Function OnSettingServer() As Boolean
        'amIntegrated and amMixed not supported yet
        Config.AuthenticationMethod = tagAuthenticationMethod.amOwn

        PushManager.AddAChatGroup(1, "R&D Department")
        PushManager.AddAChatGroup(2, "Sales Department")
        PushManager.AddAChatGroup(3, "Management Department")
        PushManager.AddAChatGroup(7, "HR Department")

        'load socketpro async sqlite and queue server libraries located at the directory ../socketpro/bin
        Dim p As IntPtr = CSocketProServer.DllManager.AddALibrary("ssqlite", ENABLE_GLOBAL_SQLITE_UPDATE_HOOK)
        If p.ToInt64() <> 0 Then
            SetSqliteDBGlobalConnectionString("usqlite.db")
        End If
        p = CSocketProServer.DllManager.AddALibrary("uasyncqueue", 24 * 1024) '24 * 1024 batch dequeuing size in bytes

        Return True 'true -- ok; false -- no listening server
    End Function

    Protected Overrides Function OnIsPermitted(ByVal hSocket As ULong, ByVal userId As String, ByVal password As String, ByVal nSvsID As UInteger) As Boolean
        Console.WriteLine("Ask for a service " & nSvsID & " from user " & userId & " with password = " & password)
        Return True
    End Function

    Protected Overrides Sub OnClose(ByVal hSocket As ULong, ByVal nError As Integer)
        Dim bs As CBaseService = CBaseService.SeekService(hSocket)
        If bs IsNot Nothing Then
            Dim sp As CSocketPeer = bs.Seek(hSocket)
            ' ......
        End If
    End Sub
End Class
