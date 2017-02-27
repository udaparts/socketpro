Imports System
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide
Imports System.Data

Public Class CMySocketProServer
    Inherits CSocketProServer

    'for db push from ms sql server
    <ServiceAttr(repConst.sidRAdoRep)> _
    Private m_RAdoRep As New CSocketProService(Of DBPushPeer)()

    'Routing requires registering two services in pair
    <ServiceAttr(piConst.sidPi)> _
    Private m_Pi As New CSocketProService(Of CClientPeer)()
    <ServiceAttr(piConst.sidPiWorker)> _
    Private m_PiWorker As New CSocketProService(Of CClientPeer)()

    Shared Sub Main(ByVal args() As String)
        Using MySocketProServer As New CMySocketProServer()
            If Not MySocketProServer.Run(20901) Then
                Console.WriteLine("Error code = " & CSocketProServer.LastSocketError.ToString())
            Else
                CSocketProServer.Router.SetRouting(piConst.sidPi, piConst.sidPiWorker)
            End If
            Console.WriteLine("Input a line to close the application ......")
            Console.ReadLine()
        End Using
    End Sub
End Class
