Imports System.Text
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide
Imports System.IO

<Serializable()> _
Friend Class CMyCall
    Public method As String
    Public version As String
    Public callIndex As String
    Public parameters As Dictionary(Of String, Object)
End Class

Friend Class CHttpPeer
	Inherits CHttpPushPeer
	Private Shared m_lstAllowIpAddress As New List(Of String)()
	Shared Sub New()
		m_lstAllowIpAddress.Add("127.0.0.1")
		m_lstAllowIpAddress.Add("10.1.100.112")
		m_lstAllowIpAddress.Add("10.1.100.103")

		'my others ....
	End Sub

	Protected Overrides Sub OnChatRequestComing(ByVal ChatRequestId As USOCKETLib.tagChatRequestID, ByVal Param0 As Object, ByVal Param1 As Object)
		Dim str As String = ""
		Dim Groups() As Integer
		Select Case ChatRequestId
			Case USOCKETLib.tagChatRequestID.idEnter, USOCKETLib.tagChatRequestID.idXEnter
				Groups = CType(Param0, Integer())
				For Each n As Integer In Groups
					If str.Length > 0 Then
						str &= ", "
					End If
					str &= n.ToString()
				Next n
				Console.WriteLine("User {0} joins chat groups {1}", UserID, str)
			Case USOCKETLib.tagChatRequestID.idSpeak, USOCKETLib.tagChatRequestID.idXSpeak
				Groups = CType(Param1, Integer())
				For Each n As Integer In Groups
					If str.Length > 0 Then
						str &= ", "
					End If
					str &= n.ToString()
				Next n
				Console.WriteLine("User {0} sends a message '{1}' to chat groups {2}", UserID, Param0.ToString(), str)
			Case USOCKETLib.tagChatRequestID.idExit
				Console.WriteLine("User {0} exits his or her chat groups", UserID)
			Case USOCKETLib.tagChatRequestID.idSendUserMessage
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

	Protected Overrides Function IsOk(ByVal parameters As Dictionary(Of String, Object)) As Boolean
		Dim nPort As Integer = 0
		Dim strIpAddress As String = GetPeerName(nPort)

		'control clients from a list of allowed ip addresses.
		'especially useful for cross-domain HTTP Push.
		'return (m_lstAllowIpAddress.IndexOf(strIpAddress) != -1);

		Return True
    End Function

    Protected Overrides Function OnSlowRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer) As Integer
        'SocketPro tries to hide browser-related problems as many as possible. However, 
        'you may still use the below code to solve a problem that SocketPro does not handle well 
		'usually by setting a proper response header 

        'http://www.useragentstring.com/pages/useragentstring.php
        Dim strUserAgent As String = Headers("User-Agent").ToLower()

        Console.WriteLine("BrowserIpAddress = " + BrowserIpAddress + ", data = " + Query)
        Return MyBase.OnSlowRequestArrive(sRequestID, nLen)
    End Function

	Protected Overrides Function OnDownloading(ByVal strFile As String) As Boolean
		Console.WriteLine(strFile)
		Return True
	End Function

	Protected Overrides Function OnProcessingRequest(ByVal strMethod As String, ByVal strVersion As String, ByVal strId As String, ByVal mapParams As Dictionary(Of String, Object)) As String
		Select Case PathName
			Case "/MyChannel"
					Dim myCall As New CMyCall()
					myCall.method = strMethod
					myCall.version = strVersion
					myCall.callIndex = strId
					myCall.parameters = mapParams

					Dim jss As New System.Web.Script.Serialization.JavaScriptSerializer()
					Dim str As String = jss.Serialize(myCall)
					Console.WriteLine(str)
					Return str
			Case Else
		End Select
		Throw New Exception("Request not supported")
	End Function
End Class

