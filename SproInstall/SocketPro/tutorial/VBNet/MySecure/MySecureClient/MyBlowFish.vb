Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports USOCKETLib

Public Class CMySecure
	Inherits CAsyncServiceHandler
	Public Sub New(ByVal cs As CClientSocket, ByVal pDefaultAsyncResultsHandler As IAsyncResultsHandler)
		MyBase.New(MyBlowFishConst.sidCMySecure, cs, pDefaultAsyncResultsHandler)
	End Sub

	Public Sub New(ByVal cs As CClientSocket)
		MyBase.New(MyBlowFishConst.sidCMySecure, cs)
	End Sub

	Public Sub New()
		MyBase.New(MyBlowFishConst.sidCMySecure)
	End Sub

	Public m_nErrorCode As Integer = 0
	Public m_strErrorMessage As String

	Protected m_OpenRtn As String
	Protected Sub OpenAsyn(ByVal strUserIDToDB As String, ByVal strPasswordToDB As String)
		SendRequest(MyBlowFishConst.idOpenCMySecure, strUserIDToDB, strPasswordToDB)
	End Sub

	Protected m_BeginTransRtn As Boolean
	Public Sub BeginTransAsyn()
		SendRequest(MyBlowFishConst.idBeginTransCMySecure)
	End Sub

	Protected m_ExecuteNoQueryRtn As Boolean
	Public Sub ExecuteNoQueryAsyn(ByVal strSQL As String)
		SendRequest(MyBlowFishConst.idExecuteNoQueryCMySecure, strSQL)
	End Sub

	Protected m_CommitRtn As Boolean
	Public Sub CommitAsyn(ByVal bSmart As Boolean)
		SendRequest(MyBlowFishConst.idCommitCMySecure, bSmart)
	End Sub

	'When a result comes from a remote SocketPro server, the below virtual function will be called.
	'We always process returning results inside the function.
	Protected Overrides Sub OnResultReturned(ByVal sRequestID As Short, ByVal UQueue As CUQueue)
		UQueue.Pop(m_nErrorCode)
		UQueue.Load(m_strErrorMessage)

		Select Case sRequestID
		Case MyBlowFishConst.idOpenCMySecure
			UQueue.Load(m_OpenRtn)
		Case MyBlowFishConst.idBeginTransCMySecure
			UQueue.Pop(m_BeginTransRtn)
		Case MyBlowFishConst.idExecuteNoQueryCMySecure
			UQueue.Pop(m_ExecuteNoQueryRtn)
		Case MyBlowFishConst.idCommitCMySecure
			UQueue.Pop(m_CommitRtn)
		Case Else
		End Select
	End Sub
	Public Function Open(ByVal strUserIDToDB As String, ByVal strPasswordToDB As String) As String
		OpenAsyn(strUserIDToDB, strPasswordToDB)
		GetAttachedClientSocket().WaitAll()
		Return m_OpenRtn
	End Function

	Public Function BeginTrans() As Boolean
		BeginTransAsyn()
		GetAttachedClientSocket().WaitAll()
		Return m_BeginTransRtn
	End Function

	Public Function ExecuteNoQuery(ByVal strSQL As String) As Boolean
		ExecuteNoQueryAsyn(strSQL)
		GetAttachedClientSocket().WaitAll()
		Return m_ExecuteNoQueryRtn
	End Function

	Public Function Commit(ByVal bSmart As Boolean) As Boolean
		CommitAsyn(bSmart)
		GetAttachedClientSocket().WaitAll()
		Return m_CommitRtn
	End Function
End Class
