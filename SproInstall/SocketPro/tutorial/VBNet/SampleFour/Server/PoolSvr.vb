Imports SocketProAdapter
Imports SocketProAdapter.ServerSide
Imports SocketProAdapter.ClientSide
Imports SocketProAdapter.ServerSide.PLG
Imports USOCKETLib
Imports System.Runtime.InteropServices
Imports System.Collections
Imports System.Threading

Namespace SocketPool
	Friend Delegate Sub DSocketEvent(ByVal hSocket As Integer, ByVal nError As Integer)
	Friend Delegate Function DPermmitEvent(ByVal hSocket As Integer, ByVal lSvsID As Integer) As Boolean
	Friend Delegate Sub DUpdateLBStatus()


	Friend Class CMyPLGPeer
		Inherits CClientPeer
		Implements IPeerJobContext
		Protected Overrides Sub OnFastRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer)
			'intercept data inside m_UQueue, and modify it here if neccessary
		End Sub

		Protected Overrides Function OnSlowRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer) As Integer
			'intercept data inside m_UQueue, and modify it here if neccessary
			Return 0
		End Function

		Protected Overrides Sub OnChatRequestComing(ByVal ChatRequestId As tagChatRequestID, ByVal Param0 As Object, ByVal Param1 As Object)
			Select Case ChatRequestId
				Case tagChatRequestID.idEnter, tagChatRequestID.idXEnter
						Dim lConnected As Long = m_JobManager.SocketPool.ConnectedSocketsEx
						Dim strUserId As String = UserID
						If lConnected = 0 Then
							Push.SendUserMessage(CMyPLGService.NoRealServerAvailable, strUserId)
						ElseIf m_JobManager.CountOfJobs >= CMyPLGService.MAX_JOB_QUEUE_SIZE Then
							Push.SendUserMessage(CMyPLGService.ExceedingMaxJobQueueSize, strUserId)
						Else
							Push.SendUserMessage(CMyPLGService.JobQueueNormal, strUserId)
						End If
				Case Else
			End Select
		End Sub

		Private m_JobManager As IJobManager

		#Region "IPeerJobContext Members"

		Private Property JobManager() As IJobManager Implements IPeerJobContext.JobManager
			Get
				Return m_JobManager
			End Get
			Set(ByVal value As IJobManager)
				m_JobManager = value
			End Set
		End Property

		Private Sub OnAddingTask(ByVal JobContext As IJobContext, ByVal sRequestId As Short) Implements IPeerJobContext.OnAddingTask

		End Sub

		Private Sub OnEnqueuingJob(ByVal JobContext As IJobContext, ByVal sRequestId As Short) Implements IPeerJobContext.OnEnqueuingJob

		End Sub

		Private Sub OnJobEnqueued(ByVal JobContext As IJobContext, ByVal sRequestId As Short) Implements IPeerJobContext.OnJobEnqueued

		End Sub

		Private Sub OnJobJustCreated(ByVal JobContext As IJobContext, ByVal sRequestId As Short) Implements IPeerJobContext.OnJobJustCreated

		End Sub

		Private Sub OnPeerDataSent(ByVal JobContext As IJobContext, ByVal sRequestId As Short) Implements IPeerJobContext.OnPeerDataSent

		End Sub

		Private Function OnSendingPeerData(ByVal JobContext As IJobContext, ByVal sRequestId As Short, ByVal UQueue As CUQueue) As Boolean Implements IPeerJobContext.OnSendingPeerData
			'you can modify data inside UQueue here if neccessary

			Return True 'true, will send result data in UQueue onto client peer; false, will not
		End Function

		Private Sub OnTaskJustAdded(ByVal JobContext As IJobContext, ByVal nTaskId As Integer, ByVal sRequestId As Short) Implements IPeerJobContext.OnTaskJustAdded

		End Sub

		Private Sub OnWaitable(ByVal JobContext As IJobContext, ByVal nTaskId As Integer, ByVal sRequestId As Short) Implements IPeerJobContext.OnWaitable
'            
'            //you can put a barrier here
'            bool b = JobContext.Wait();
'            if (b)
'            {
'                //do your work here
'            }
		End Sub

		#End Region
	End Class

	Friend Class CMyPLGService
		Inherits CPLGService(Of CMyPLGPeer)
        Public Const sidCRAdo As Integer = (CInt(USOCKETLib.tagOtherDefine.odUserServiceIDMin) + 202)
		Public Const MAX_JOB_QUEUE_SIZE As Integer = 5
		Public Const Failover As Integer = 11111
		Public Const ExceedingMaxJobQueueSize As Integer = 11112
		Public Const JobQueueNormal As Integer = 11113
		Public Const NoRealServerAvailable As Integer = 11114

		Public m_frmSocketPool As frmSocketPool = Nothing

		Protected Overrides Sub OnAllSocketsDisconnected()
			'just get any one of peer sockets
			Dim nTotalClients As Integer = CSocketProServer.CountOfClients
			Do While nTotalClients > 0
				nTotalClients -= 1
				Dim hSocket As Integer = CSocketProServer.GetClient(nTotalClients)
				Dim p As CClientPeer = SeekClientPeer(hSocket)
				If p IsNot Nothing Then
					Dim groups() As Integer = { 1 }
					p.Push.Broadcast(CMyPLGService.NoRealServerAvailable, groups)
					Exit Do
				End If
			Loop
			m_frmSocketPool.BeginInvoke(m_frmSocketPool.m_OnUpdateLBStatus)
		End Sub

		Protected Overrides Function OnFailover(ByVal Handler As CAsyncServiceHandler, ByVal JobContext As IJobContext) As Boolean
			Dim peer As CClientPeer = CType(JobContext.Identity, CClientPeer)

			'send own a fail message
			peer.Push.SendUserMessage(CMyPLGService.Failover, peer.UserID)

			m_frmSocketPool.BeginInvoke(m_frmSocketPool.m_OnUpdateLBStatus)
			Return True 'true, make disaster recovery; false, no disaster recovery
		End Function

		Protected Overrides Sub OnJobDone(ByVal Handler As CAsyncServiceHandler, ByVal JobContext As IJobContext)
			If JobContext.JobManager.CountOfJobs < (CMyPLGService.MAX_JOB_QUEUE_SIZE -1) Then
				Dim peer As CClientPeer = CType(JobContext.Identity, CClientPeer)
				Dim groups() As Integer ={ 1 }
				peer.Push.Broadcast(CMyPLGService.JobQueueNormal, groups)
			End If
			m_frmSocketPool.BeginInvoke(m_frmSocketPool.m_OnUpdateLBStatus)
		End Sub

		Protected Overrides Function OnExecutingJob(ByVal Handler As CAsyncServiceHandler, ByVal JobContext As IJobContext) As Boolean
			If JobContext.JobManager.CountOfJobs >= CMyPLGService.MAX_JOB_QUEUE_SIZE Then
				Dim peer As CClientPeer = CType(JobContext.Identity, CClientPeer)
				Dim groups() As Integer ={ 1 }
				peer.Push.Broadcast(CMyPLGService.ExceedingMaxJobQueueSize, groups)
			End If

			m_frmSocketPool.BeginInvoke(m_frmSocketPool.m_OnUpdateLBStatus)
			Return True
		End Function
	End Class

	Public Class CPoolSvr
		Inherits CSocketProServer
		Public Sub New()
			CMyPLGService.ServiceIdOnRealServer = CMyPLGService.sidCRAdo
		End Sub

		Private Function IsAllowed(ByVal strUserID As String, ByVal strPassword As String) As Boolean
			If strPassword <> "PassOne" Then
				Return False
			End If
			Return (strUserID.ToLower() = "socketpro")
		End Function

		Protected Overrides Function OnIsPermitted(ByVal hSocket As Integer, ByVal lSvsID As Integer) As Boolean
			If m_frmSocketPool IsNot Nothing Then
				Dim args(1) As Object
				args(0) = hSocket
				args(1) = lSvsID

				Dim strUID As String = CSocketProServer.GetUserID(hSocket)

				'password is available ONLY IF authentication method to either amOwn or amMixed
				Dim strPassword As String = CSocketProServer.GetPassword(hSocket)

				Dim ar As IAsyncResult = m_frmSocketPool.BeginInvoke(m_frmSocketPool.m_OnIsPermitted, args)
				Return IsAllowed(strUID, strPassword)
			End If
			Return True
		End Function

		Protected Overrides Sub OnClose(ByVal hSocket As Integer, ByVal nError As Integer)
			If m_frmSocketPool IsNot Nothing Then
				Dim args(1) As Object
				args(0) = hSocket
				args(1) = nError
				m_frmSocketPool.BeginInvoke(m_frmSocketPool.m_OnClose, args)
			End If
		End Sub

		Protected Overrides Sub OnAccept(ByVal hSocket As Integer, ByVal nError As Integer)
			If m_frmSocketPool IsNot Nothing Then
				Dim args(1) As Object
				args(0) = hSocket
				args(1) = nError
				m_frmSocketPool.BeginInvoke(m_frmSocketPool.m_OnAccept, args)
			End If
		End Sub

		Protected Overrides Function OnSettingServer() As Boolean
			'turn off online compressing at the server side so that
			'SocketPro server will not compress return results unless a client turns on it 
			Config.DefaultZip = False

			'amMixed
			Config.AuthenticationMethod = USOCKETLib.tagAuthenticationMethod.amMixed

			PushManager.AddAChatGroup(1, "Loading Balance Job Queue Size")

			Return AddService()
		End Function

		Public Function IsRunning() As Boolean
			Return m_bRunning
		End Function

		Private Function AddService() As Boolean
			Dim n As Integer
			Const Count As Integer = 5
			Dim pConnectionContext(Count - 1) As CConnectionContext
			For n = 0 To Count - 1
				pConnectionContext(n) = New CConnectionContext()
			Next n

			'set connection contexts
			pConnectionContext(0).m_strHost = "127.0.0.1"
			pConnectionContext(1).m_strHost = "localhost"
			pConnectionContext(2).m_strHost = "127.0.0.1"
			pConnectionContext(3).m_strHost = "charliedev"
			pConnectionContext(4).m_strHost = "charliedev"
			For n = 0 To Count - 1
				pConnectionContext(n).m_nPort = 20901
				pConnectionContext(n).m_strPassword = "SocketPro"
				pConnectionContext(n).m_strUID = "PassOne"
				pConnectionContext(n).m_EncrytionMethod = USOCKETLib.tagEncryptionMethod.NoEncryption
				pConnectionContext(n).m_bZip = False
			Next n
			If Not m_RadoPoolSvs.SocketPool.StartSocketPool(pConnectionContext, m_bSocketsPerThread, m_bThreadCount) Then
				Return False
			End If
			If Not m_RadoPoolSvs.AddMe(CMyPLGService.sidCRAdo, tagThreadApartment.taFree) Then
				Return False
			End If
			If Not m_RadoPoolSvs.AddSlowRequest(CShort(Fix(USOCKETLib.tagBaseRequestID.idEndJob))) Then
				Return False
			End If
			Return True
		End Function


		Private m_nPort As Integer = 0
		Private m_bThreadCount As Byte = 0
		Private m_bSocketsPerThread As Byte = 0

        Public Sub RunMe(ByVal nPort As Integer, ByVal bThreadCount As Byte, ByVal bSocketsPerThread As Byte)
            Me.Stop()
            m_RadoPoolSvs.m_frmSocketPool = m_frmSocketPool
            m_nPort = nPort
            m_bThreadCount = bThreadCount
            m_bSocketsPerThread = bSocketsPerThread
            m_bRunning = Run(nPort)
        End Sub

		Public Sub [Stop]()
			If m_bRunning Then
				StopSocketProServer()
				m_bRunning = False
			End If
		End Sub

		Friend m_RadoPoolSvs As New CMyPLGService()
		Public m_frmSocketPool As frmSocketPool = Nothing
		Private m_bRunning As Boolean = False
	End Class
End Namespace
