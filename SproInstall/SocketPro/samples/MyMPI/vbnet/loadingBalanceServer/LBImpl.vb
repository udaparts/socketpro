' **** including all of defines, service id(s) and request id(s) ***** 
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports SocketProAdapter.ServerSide
Imports SocketProAdapter.ServerSide.PLG
Imports USOCKETLib 'you may need it for accessing various constants

'server implementation for service CPPi
Public Class CPiPeer
	Inherits CClientPeer
	Implements IPeerJobContext
	Protected Overrides Sub OnFastRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer)
		'intercept data inside m_UQueue, and modify it here if neccessary
	End Sub

	Protected Overrides Function OnSlowRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer) As Integer
		'intercept data inside m_UQueue, and modify it here if neccessary
		Return 0
	End Function
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
		'you can call JobContext.Wait() here to barrier for result or something else.
		'Must pay close attention to main thread or worker thread.
		'In general, don't call JobContext.Wait() within main thread but worker thread only.
	End Sub
	#End Region
End Class

Public Class CMySocketProServer
	Inherits CSocketProServer
	Protected Overrides Function OnIsPermitted(ByVal hSocket As Integer, ByVal nSvsID As Integer) As Boolean
		'give permission to all
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

		'add service(s) into SocketPro server
		AddService()

		Return True
	End Function

	Private m_LoadingBalance As New CPLGService(Of CPiPeer)()

	Private Sub AddService()
        Dim ok As Boolean
        Dim n As Integer
        Const Count As Integer = 5

        Const sidCPPi As Integer = (CInt(USOCKETLib.tagOtherDefine.odUserServiceIDMin) + 1212)
        SocketProAdapter.ServerSide.PLG.CPLGService(Of CPiPeer).ServiceIdOnRealServer = sidCPPi

        Dim pConnectionContext(Count - 1) As CConnectionContext
        For n = 0 To Count - 1
            pConnectionContext(n) = New CConnectionContext()
        Next n

        'set connection contexts
        pConnectionContext(0).m_strHost = "127.0.0.1"
        pConnectionContext(1).m_strHost = "localhost"
        pConnectionContext(2).m_strHost = "localhost"
        pConnectionContext(3).m_strHost = "127.0.0.1"
        pConnectionContext(4).m_strHost = "localhost"
        For n = 0 To Count - 1
            pConnectionContext(n).m_nPort = 20901
            pConnectionContext(n).m_strPassword = "SocketPro"
            pConnectionContext(n).m_strUID = "PassOne"
            pConnectionContext(n).m_EncrytionMethod = USOCKETLib.tagEncryptionMethod.NoEncryption
            pConnectionContext(n).m_bZip = False
        Next n

        ok = m_LoadingBalance.SocketPool.StartSocketPool(pConnectionContext, 2, 3)
        ok = m_LoadingBalance.AddMe(sidCPPi, 0, tagThreadApartment.taFree)
	End Sub
End Class

