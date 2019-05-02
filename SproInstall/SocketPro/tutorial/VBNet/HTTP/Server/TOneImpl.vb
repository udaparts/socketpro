' **** including all of defines, service id(s) and request id(s) ***** 
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide
Imports USOCKETLib 'you may need it for accessing various constants

Public Class CMySocketProServer
	Inherits CSocketProServer
	Protected Overrides Function OnIsPermitted(ByVal hSocket As Integer, ByVal nSvsID As Integer) As Boolean
		Console.WriteLine("A socket connection is permitted")

		'give permission to all
		Return True
	End Function

	Protected Overrides Sub OnAccept(ByVal hSocket As Integer, ByVal nError As Integer)
		Console.WriteLine("A socket is initially establised")
	End Sub

	Protected Overrides Sub OnClose(ByVal hSocket As Integer, ByVal nError As Integer)
		Console.WriteLine("A socket is closed with error code = " & nError)
	End Sub

	Protected Overrides Function OnSettingServer() As Boolean
		'try amIntegrated and amMixed
		Config.AuthenticationMethod = tagAuthenticationMethod.amOwn

		'add service(s) into SocketPro server
		AddService()

		Return True
	End Function

	Private m_HttpSvs As New CSocketProService(Of CHttpPeer)()

	Private Sub AddService()
        Dim ok As Boolean

        ok = m_HttpSvs.AddMe(CInt(Fix(USOCKETLib.tagServiceID.sidHTTP)), 0, tagThreadApartment.taNone)
        ok = m_HttpSvs.AddSlowRequest(CShort(Fix(USOCKETLib.tagHttpRequestID.idGet)))
        ok = m_HttpSvs.AddSlowRequest(CShort(Fix(USOCKETLib.tagHttpRequestID.idPost)))
        ok = m_HttpSvs.AddSlowRequest(CShort(Fix(USOCKETLib.tagHttpRequestID.idMultiPart)))
	End Sub

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

