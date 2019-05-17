
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
    Shared Sub SetSqliteDBGlobalConnectionString(<MarshalAs(UnmanagedType.LPWStr)> ByVal sqliteDbFile As String)
    End Sub

    <DllImport("ustreamfile")> _
    Shared Sub SetRootDirectory(<MarshalAs(UnmanagedType.LPWStr)> ByVal root As String)
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

    <ServiceAttr(BaseServiceID.sidHTTP)>
    Private m_http As New CSocketProService(Of CMyHttpPeer)()

    Shared Sub Main(ByVal args() As String)
        Using MySocketProServer As New CMySocketProServer()
            'test certificate and private key files are located at ../SocketProRoot/bin
            'MySocketProServer.UseSSL("intermediate.pfx", "", "mypassword")
            'MySocketProServer.UseSSL("root"/*"my"*/, "UDAParts Intermediate CA", "") 'or load cert and private key from windows system cert store

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
        PushManager.AddAChatGroup(SocketProAdapter.UDB.DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID, "Subscribe/publish for front clients")

        'load socketpro async sqlite and queue server libraries located at the directory ../socketpro/bin
        Dim p As IntPtr = CSocketProServer.DllManager.AddALibrary("ssqlite")
        If p.ToInt64() <> 0 Then
            SetSqliteDBGlobalConnectionString("usqlite.db+sakila.db.actor;sakila.db.language;sakila.db.category;sakila.db.country;sakila.db.film_actor")
        End If
        p = CSocketProServer.DllManager.AddALibrary("uasyncqueue", 24 * 1024) '24 * 1024 batch dequeuing size in bytes

        p = CSocketProServer.DllManager.AddALibrary("ustreamfile")

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
