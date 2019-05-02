' **** including all of defines, service id(s) and request id(s) ***** 

Imports Microsoft.VisualBasic
Imports System
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide
Imports USOCKETLib 'you may need it for accessing various constants
Imports System.Diagnostics
Imports System.Runtime.InteropServices

'server implementation for service CTOne
Public Class CTOnePeer : Inherits CClientPeer
	<DllImport("Kernel32.dll")> _
	Public Shared Function GetCurrentThreadId() As Integer
	End Function

	Protected Overrides Sub OnSwitchFrom(ByVal nServiceID As Integer)
		'always processed within main thread
		Trace.Assert(CSocketProServer.MainThreadID = GetCurrentThreadId())

        m_uCount = 0 'initialize the object here
		Console.WriteLine("Socket is switched for the service CTOneSvs")
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

    Protected Sub QueryCount(ByRef QueryCountRtn As Integer)
        QueryCountRtn = m_uCount
    End Sub


    Protected Sub QueryGlobalCount(ByRef QueryGlobalCountRtn As Integer)
        SyncLock m_cs
            QueryGlobalCountRtn = m_uGlobalCount
        End SyncLock
    End Sub

    Protected Sub QueryGlobalFastCount(ByRef QueryGlobalFastCountRtn As Integer)
        QueryGlobalFastCountRtn = m_uGlobalFastCount
    End Sub

	Protected Sub Sleep(ByVal nTime As Integer)
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

        m_uCount = m_uCount + 1
        m_uGlobalFastCount += 1

        SyncLock m_cs
            m_uGlobalCount += 1
        End SyncLock

        Select Case sRequestID
            Case TOneConst.idQueryCountCTOne
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
        m_uCount = m_uCount + 1

        SyncLock m_cs
            m_uGlobalCount = m_uGlobalCount + 1
        End SyncLock

        Select Case sRequestID
            Case TOneConst.idSleepCTOne
                M_I1_R0(Of Integer)(AddressOf Sleep)
            Case Else
        End Select
        Return 0
    End Function


	'm_GlobalCount must be synchronized 
	'because it is accessed within different threads from different socket clients
    Private Shared m_cs As Object = New Object()
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


Public Class CMySocketProServer : Inherits CSocketProServer
    Protected Overrides Function OnIsPermitted(ByVal hSocket As Integer, ByVal nSvsID As Integer) As Boolean
        'always processed within main thread
        Trace.Assert(CSocketProServer.MainThreadID = CTOnePeer.GetCurrentThreadId())

        Console.WriteLine("A socket connection is permitted")

        'give permission to all
        Return True
    End Function


    Protected Overrides Function OnSettingServer() As Boolean
        'try amIntegrated and amMixed
        Config.AuthenticationMethod = USOCKETLib.tagAuthenticationMethod.amOwn

        'add service(s) into SocketPro server
        AddService()

        Return True 'true -- ok; and false -- no listening socket
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


    Private m_CTOne As CSocketProService(Of CTOnePeer) = New CSocketProService(Of CTOnePeer)()

    Private Sub AddService()
        Dim ok As Boolean

        'No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
        ok = m_CTOne.AddMe(TOneConst.sidCTOne, 0, tagThreadApartment.taNone)
        'If ok is false, very possibly you have two services with the same service id!

        ok = m_CTOne.AddSlowRequest(TOneConst.idSleepCTOne)
    End Sub

    Shared Sub Main(ByVal args As String())
        Dim MySocketProServer As New CMySocketProServer()
        If Not MySocketProServer.Run(20901) Then
            Console.WriteLine("Error code = " & CSocketProServer.LastSocketError.ToString())
        End If
        Console.WriteLine("Input a line to close the application ......")
        Dim str As String = Console.ReadLine()
        MySocketProServer.StopSocketProServer()
    End Sub
End Class

