' **** including all of defines, service id(s) and request id(s) ***** 
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide
Imports USOCKETLib 'you may need it for accessing various constants
Imports System.Runtime.InteropServices

'server implementation for service CTOne
Public Class CTOnePeer
    Inherits CClientPeer

    <DllImport("Kernel32.dll")> _
 Public Shared Function GetCurrentThreadId() As Integer
    End Function

	Protected Overrides Sub OnSwitchFrom(ByVal nServiceID As Integer)
		'always processed within main thread
		Trace.Assert(CSocketProServer.MainThreadID = GetCurrentThreadId())

		m_uCount = 0 'initialize the object here
        Console.WriteLine("Socket is switched for the service CTOneSvs")

        Dim nGroups() As Integer = {1, 2, 3, 4, 5}
        Push.Enter(nGroups)

	End Sub

	Protected Overrides Sub OnReleaseResource(ByVal bClosing As Boolean, ByVal nInfo As Integer)
		'always processed within main thread
		Trace.Assert(CSocketProServer.MainThreadID = GetCurrentThreadId())

		If bClosing Then
			Console.WriteLine("Socket is closed with error code = " & nInfo)
		Else
			Console.WriteLine("Socket is going to be switched to new service with service id = " & nInfo)
		End If

		'release all of your resources here as early as possible
	End Sub

	Protected Overrides Sub OnDispatchingSlowRequest(ByVal sRequestID As Short)
		m_uGlobalCount += 1
	End Sub

	Protected Sub QueryCount(ByRef QueryCountRtn As Integer)
		QueryCountRtn = CInt(Fix(m_uCount))
	End Sub

	Protected Sub QueryGlobalCount(ByRef QueryGlobalCountRtn As Integer)
		QueryGlobalCountRtn = CInt(Fix(m_uGlobalCount))
	End Sub

	Protected Sub QueryGlobalFastCount(ByRef QueryGlobalFastCountRtn As Integer)
		QueryGlobalFastCountRtn = CInt(Fix(m_uGlobalFastCount))
	End Sub

	Protected Sub Sleep(ByVal nTime As Integer)
		Dim groups() As Integer = {1,2}
		'inform all of joined clients that idSleep is called
		Push.Broadcast("Sleep called", groups)

		If TransferServerException AndAlso nTime < 200 Then
			Throw New CSocketProServerException(12345, "Sleeping time is too short!")
		End If
		System.Threading.Thread.Sleep(nTime)
	End Sub

	Protected Sub Echo(ByVal objInput As Object, ByRef EchoRtn As Object)
		EchoRtn = objInput
	End Sub

	Protected Overrides Sub OnFastRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer)
		'always processed within main thread
		Trace.Assert(CSocketProServer.MainThreadID = GetCurrentThreadId())

		m_uCount += 1
		m_uGlobalFastCount += 1
		m_uGlobalCount += 1

		Select Case sRequestID
			Case TOneConst.idQueryCountCTOne
				'Ix x -- the number of inputs from client, Ry y -- the number of outputs that will be sent to client
				M_I0_R1(Of Integer)(AddressOf QueryCount)
			Case TOneConst.idQueryGlobalCountCTOne
				M_I0_R1(Of Integer)(AddressOf QueryGlobalCount)
			Case TOneConst.idQueryGlobalFastCountCTOne
				M_I0_R1(Of Integer)(AddressOf QueryGlobalFastCount)
			Case TOneConst.idEchoCTOne
				M_I1_R1(Of Object, Object)(AddressOf Echo)
			Case Else
		End Select
	End Sub

	Protected Overrides Function OnSlowRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer) As Integer
		'always processed within a worker thread
		Trace.Assert(CSocketProServer.MainThreadID <> GetCurrentThreadId())
		m_uCount += 1

		Select Case sRequestID
			Case TOneConst.idSleepCTOne
				'Ix x -- the number of inputs from client, Ry y -- the number of outputs that will be sent to client
				M_I1_R0(Of Integer)(AddressOf Sleep)
			Case Else
		End Select

		Return 0
	End Function

	'm_GlobalCount doesn't need to be synchronized 
	'because it is accessed only within main thread
	'although is is accessed from different socket clients
    Private Shared m_uGlobalCount As Integer = 0

	'm_uGlobalFastCount doesn't need to be synchronized 
	'because it is accessed within main thread, 
	'although is is accessed from different socket clients
    Private Shared m_uGlobalFastCount As Integer = 0

	'm_uCount doesn't need to be synchronized 
	'because it is always accessed from one socket client only, 
	'even though it may be accessed within different threads.
    Private m_uCount As Integer
End Class

Public Class CMySocketProServer
	Inherits CSocketProServer
	Private Sub ReuseLibraries()
		'those libraries are distributed in the directory ..\bin

		Dim hInst As IntPtr = CBaseService.AddALibrary("uodbsvr.dll", 0)
		If hInst = CType(0, IntPtr) Then
			Console.WriteLine("library uodbsvr.dll not available.")
		End If

		hInst = CBaseService.AddALibrary("ufilesvr.dll", 0)
		If hInst = CType(0, IntPtr) Then
			Console.WriteLine("library ufilesvr.dll not available.")
		End If

		hInst = CBaseService.AddALibrary("udemo.dll", 0)
		If hInst = CType(0, IntPtr) Then
			Console.WriteLine("library udemo.dll not available.")
		End If
	End Sub

	Private Sub SetBuiltinChatService()
        Dim ok As Boolean = PushManager.AddAChatGroup(1, "Group for SOne")

        Trace.Assert(ok)
    End Sub

    Private Function IsAllowed(ByVal strUserID As String, ByVal strPassword As String) As Boolean
        If strPassword <> "PassOne" Then
            Return False
        End If
        Return (strUserID.ToLower() = "socketpro")
    End Function

    Protected Overrides Function OnIsPermitted(ByVal hSocket As Integer, ByVal nSvsID As Integer) As Boolean
        'always processed within main thread
        Trace.Assert(CSocketProServer.MainThreadID = CTOnePeer.GetCurrentThreadId())

        Dim strUID As String = GetUserID(hSocket)

        'password is available ONLY IF authentication method to either amOwn or amMixed
        Dim strPassword As String = GetPassword(hSocket)

        Console.WriteLine("For service = {0}, User ID = {1}, Password = {2}", nSvsID, strUID, strPassword)

        Dim am As tagAuthenticationMethod = Config.AuthenticationMethod

        If am = tagAuthenticationMethod.amOwn OrElse am = tagAuthenticationMethod.amMixed Then
            'do my own authentication
            Return IsAllowed(strUID, strPassword)
        End If

        Return True
    End Function

    Protected Overrides Sub OnAccept(ByVal hSocket As Integer, ByVal nError As Integer)
        'always processed within main thread
        Trace.Assert(CSocketProServer.MainThreadID = CTOnePeer.GetCurrentThreadId())

        Console.WriteLine("A socket is initially establised")
    End Sub

    Protected Overrides Sub OnClose(ByVal hSocket As Integer, ByVal nError As Integer)
        'always processed within main thread
        Trace.Assert(CSocketProServer.MainThreadID = CTOnePeer.GetCurrentThreadId())

        Console.WriteLine("A socket is closed with error code = " & nError)
    End Sub

    Protected Overrides Function OnSettingServer() As Boolean
        'amMixed
        Config.AuthenticationMethod = tagAuthenticationMethod.amMixed

        Config.SharedAM = True

        'limit the max number of connections to 2 for a client machine
        Config.MaxConnectionsPerClient = 2

        'add service(s) into SocketPro server
        AddService()

        SetBuiltinChatService()

        'use TLSv1 to secure all of data communication between a client and a SocketPro server
        'udacert.pem contains both key and certificate, which is distributed in the ..\bin as sample certificate and key
        'UseSSL("C:\\Program Files\\UDAParts\\SocketPro\\bin\\udacert.pfx", "mypassword", "udaparts", tagEncryptionMethod.MSTLSv1);

        'reuse my high performance C/C++ libraries
        ReuseLibraries()

        Return True 'true -- ok; false -- no listening server
    End Function

    Private m_CTOne As New CSocketProService(Of CTOnePeer)()
    Private m_CTThree As New CTThreeSvs()

    Private Sub AddService()
        Dim ok As Boolean

        'No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
        ok = m_CTOne.AddMe(TOneConst.sidCTOne, 0, tagThreadApartment.taNone)
        'If ok is false, very possibly you have two services with the same service id!

        ok = m_CTOne.AddSlowRequest(TOneConst.idSleepCTOne)

        'No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
        ok = m_CTThree.AddMe(TThreeConst.sidCTThree, 0, tagThreadApartment.taNone)
        'If ok is false, very possibly you have two services with the same service id!

        ok = m_CTThree.AddSlowRequest(TThreeConst.idGetManyItemsCTThree)
        ok = m_CTThree.AddSlowRequest(TThreeConst.idSendBatchItemsCTThree)
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

