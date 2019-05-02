' **** including all of defines, service id(s) and request id(s) ***** 
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide
Imports USOCKETLib 'you may need it for accessing various constants
'server implementation for service CMySecure
Public Class CMySecurePeer
	Inherits CClientPeer
	Protected Overrides Sub OnSwitchFrom(ByVal nServiceID As Integer)
		'All return results are not required to be secure
        EncryptionMethod = USOCKETLib.tagEncryptionMethod.NoEncryption
	End Sub

	Private m_nErrorCode As Integer = 0
	Private m_strErrorMessage As String

	Private Sub PushError()
		m_UQueue.Push(m_nErrorCode)
		m_UQueue.Save(m_strErrorMessage)
	End Sub

	Protected Overrides Sub OnReleaseResource(ByVal bClosing As Boolean, ByVal nInfo As Integer)
		If bClosing Then
			'closing the socket with error code = nInfo
		Else
			'switch to a new service with the service id = nInfo
		End If

		'release all of your resources here as early as possible
	End Sub

	Protected Sub Open(ByVal strUserIDToDB As String, ByVal strPasswordToDB As String, <System.Runtime.InteropServices.Out()> ByRef OpenRtn As String)
		OpenRtn = "Oracle Database"
		m_strErrorMessage = "Ok!"
		m_nErrorCode = 0
	End Sub

	Protected Sub BeginTrans(<System.Runtime.InteropServices.Out()> ByRef BeginTransRtn As Boolean)
		BeginTransRtn = True
		m_strErrorMessage = "BeginTrans OK!"
		m_nErrorCode = 0
	End Sub

	Protected Sub ExecuteNoQuery(ByVal strSQL As String, <System.Runtime.InteropServices.Out()> ByRef ExecuteNoQueryRtn As Boolean)
		ExecuteNoQueryRtn = True

		m_strErrorMessage = "ExecuteNoQuery OK!"
		m_nErrorCode = 0
	End Sub

	Protected Sub Commit(ByVal bSmart As Boolean, <System.Runtime.InteropServices.Out()> ByRef CommitRtn As Boolean)
		CommitRtn = True

		m_strErrorMessage = "Commit OK!"
		m_nErrorCode = 0
	End Sub

	Protected Overrides Sub OnFastRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer)
		Select Case sRequestID
		Case MyBlowFishConst.idBeginTransCMySecure
			Dim BeginTransRtn As Boolean
			BeginTrans(BeginTransRtn)
			m_UQueue.SetSize(0) 'initialize memory chunk size to 0
			PushError()
			m_UQueue.Push(BeginTransRtn)
			SendResult(sRequestID, m_UQueue)
		Case MyBlowFishConst.idCommitCMySecure
			Dim bSmart As Boolean = False
			Dim CommitRtn As Boolean
			m_UQueue.Pop(bSmart)
			Commit(bSmart, CommitRtn)
			m_UQueue.SetSize(0) 'initialize memory chunk size to 0
			PushError()
			m_UQueue.Push(CommitRtn)
			SendResult(sRequestID, m_UQueue)
		Case Else
		End Select
	End Sub

	Protected Overrides Function OnSlowRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer) As Integer
		Select Case sRequestID
		Case MyBlowFishConst.idOpenCMySecure
			Dim strUserIDToDB As String = Nothing
			Dim strPasswordToDB As String = Nothing
			Dim OpenRtn As String
			m_UQueue.Load(strUserIDToDB)
			m_UQueue.Load(strPasswordToDB)
			Open(strUserIDToDB, strPasswordToDB, OpenRtn)
			m_UQueue.SetSize(0) 'initialize memory chunk size to 0
			PushError()
			m_UQueue.Save(OpenRtn)
			SendResult(sRequestID, m_UQueue)
		Case MyBlowFishConst.idExecuteNoQueryCMySecure
			Dim strSQL As String = Nothing
			Dim ExecuteNoQueryRtn As Boolean
			m_UQueue.Load(strSQL)
			ExecuteNoQuery(strSQL, ExecuteNoQueryRtn)
			m_UQueue.SetSize(0) 'initialize memory chunk size to 0
			PushError()
			m_UQueue.Push(ExecuteNoQueryRtn)
			SendResult(sRequestID, m_UQueue)
		Case Else
		End Select
		Return 0
	End Function
End Class

Public Class CMySocketProServer
	Inherits CSocketProServer
	Private Function GetPasswordFromStore(ByVal strUserID As String) As String
		If String.Compare(strUserID, "SocketPro", True) = 0 Then
			Return "PassOne"
		ElseIf String.Compare(strUserID, "RDBClient", True) = 0 Then
			Return "PassTwo"
		End If
		Return Nothing
	End Function

	Protected Overrides Function OnIsPermitted(ByVal hSocket As Integer, ByVal nSvsID As Integer) As Boolean
		Dim strUserID As String = GetUserID(hSocket)
		Dim strPassword As String = GetPasswordFromStore(strUserID)

		'set password so that the hash of the password with salt can be sent to a client for authentication on client side.
		SetPassword(hSocket, strPassword)
		Return True
	End Function

	Protected Overrides Sub OnAccept(ByVal hSocket As Integer, ByVal nError As Integer)
		'when a socket is initially established
	End Sub

	Protected Overrides Sub OnClose(ByVal hSocket As Integer, ByVal nError As Integer)
		'when a socket is closed
	End Sub

	Protected Overrides Function OnSettingServer() As Boolean
		'try amIntegrated and amMixed
		Config.AuthenticationMethod = tagAuthenticationMethod.amOwn

        Config.DefaultEncryptionMethod = USOCKETLib.tagEncryptionMethod.BlowFish

		AddService()

		Return True
	End Function

	Private m_CMySecure As New CSocketProService(Of CMySecurePeer)()

	Private Sub AddService()
        Dim ok As Boolean

        'No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
        ok = m_CMySecure.AddMe(MyBlowFishConst.sidCMySecure, 0, tagThreadApartment.taNone)
        'If ok is false, very possibly you have two services with the same service id!

        ok = m_CMySecure.AddSlowRequest(MyBlowFishConst.idOpenCMySecure)
        ok = m_CMySecure.AddSlowRequest(MyBlowFishConst.idExecuteNoQueryCMySecure)
	End Sub
End Class

