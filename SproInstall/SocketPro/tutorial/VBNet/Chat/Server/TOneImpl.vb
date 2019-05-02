' **** including all of defines, service id(s) and request id(s) ***** 
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide
Imports USOCKETLib 'you may need it for accessing various constants


Friend Class CMyDummyPeer
	Inherits CClientPeer
	Protected Overrides Sub OnChatRequestComing(ByVal ChatRequestId As USOCKETLib.tagChatRequestID, ByVal Param0 As Object, ByVal Param1 As Object)
		Dim str As String = ""
		Dim Groups() As Integer
		Select Case ChatRequestId
			Case tagChatRequestID.idEnter, tagChatRequestID.idXEnter
				Groups = CType(Param0, Integer())
				For Each n As Integer In Groups
					If str.Length > 0 Then
						str &= ", "
					End If
					str &= n.ToString()
				Next n
				Console.WriteLine("User {0} joins chat groups {1}", UserID, str)
			Case tagChatRequestID.idSpeak, tagChatRequestID.idXSpeak
				Groups = CType(Param1, Integer())
				For Each n As Integer In Groups
					If str.Length > 0 Then
						str &= ", "
					End If
					str &= n.ToString()
				Next n
				If Param0 Is Nothing Then
					Param0 = "null"
				End If
				Console.WriteLine("User {0} sends a message '{1}' to chat groups {2}", UserID, Param0.ToString(), str)
			Case tagChatRequestID.idExit
				Console.WriteLine("User {0} exits his or her chat groups", UserID)
			Case tagChatRequestID.idSendUserMessage
				If Param0 Is Nothing Then
					Param0 = "null"
				End If
				If Param1 Is Nothing Then
					Param1 = "null"
				End If
				Console.WriteLine("User {0} sends a message '{1}' to {2}", UserID, Param1.ToString(), Param0.ToString())
			Case Else
		End Select
	End Sub

	Protected Overrides Sub OnFastRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer)
		'should never come here
	End Sub

	Protected Overrides Function OnSlowRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer) As Integer
		'should never come here
		Return 0
	End Function
End Class

Public Class CMySocketProServer
	Inherits CSocketProServer
	Protected Overrides Function OnIsPermitted(ByVal hSocket As Integer, ByVal nSvsID As Integer) As Boolean
		'give permission to all
		Return True
	End Function

	Protected Overrides Sub OnAccept(ByVal hSocket As Integer, ByVal nError As Integer)
		Console.WriteLine("Socket {0} accepted with error = {1}", hSocket, nError)
	End Sub

	Protected Overrides Sub OnClose(ByVal hSocket As Integer, ByVal nError As Integer)
		Console.WriteLine("Socket {0} closed with error = {1}", hSocket, nError)
	End Sub

	Protected Overrides Function OnSettingServer() As Boolean
		'try amIntegrated and amMixed
		Config.AuthenticationMethod = tagAuthenticationMethod.amOwn

		'add service(s) into SocketPro server
		AddService()

		Return True
	End Function

	Private m_HttpSvs As New CSocketProService(Of CHttpPeer)()
	Private m_ChatSvs As New CSocketProService(Of CMyDummyPeer)()

	Private Sub AddService()
        Dim ok As Boolean

        ok = PushManager.AddAChatGroup(1, "Group for SOne")
        ok = PushManager.AddAChatGroup(2, "DB Service")
        ok = PushManager.AddAChatGroup(4, "Management Department")
        ok = PushManager.AddAChatGroup(9, "IT Department")
        ok = PushManager.AddAChatGroup(16, "Sales Department")

        'one default page
        CHttpPushPeer.Default = "httppush.htm"

        ok = m_HttpSvs.AddMe(CInt(Fix(USOCKETLib.tagServiceID.sidHTTP)), 0, tagThreadApartment.taNone)
        ok = m_HttpSvs.AddSlowRequest(CShort(Fix(USOCKETLib.tagHttpRequestID.idGet)))
        ok = m_HttpSvs.AddSlowRequest(CShort(Fix(USOCKETLib.tagHttpRequestID.idPost)))

        ok = m_ChatSvs.AddMe(CInt(Fix(USOCKETLib.tagServiceID.sidChat)), 0, tagThreadApartment.taNone)
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

