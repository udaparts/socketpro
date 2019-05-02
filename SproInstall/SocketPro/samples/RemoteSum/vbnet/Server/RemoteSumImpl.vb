' **** including all of defines, service id(s) and request id(s) ***** 
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide
Imports USOCKETLib 'you may need it for accessing various constants
'server implementation for service RemSum
Public Class RemSumPeer
	Inherits CClientPeer
	Private m_nSum As Integer = 0
	Private m_nStart As Integer = 0
	Private m_nEnd As Integer = 0

    Private Sub DoSum(ByVal start As Integer, ByVal [end] As Integer, ByRef DoSumRtn As Integer) 'out
        'initialize stateful members
        m_nSum = 0
        m_nStart = start
        m_nEnd = [end]

        'do calculation
        DoSumRtn = Compute()
    End Sub

	Private Function Compute() As Integer
		Dim n As Integer
		For n = m_nStart To m_nEnd
			m_nSum += n
			System.Threading.Thread.Sleep(100) 'simulate slow request
			Dim rtn As Integer = SendResult(RemoteSumConst.idReportProgress, n, m_nSum)
			If rtn = CClientPeer.REQUEST_CANCELED OrElse rtn = CClientPeer.SOCKET_NOT_FOUND Then
				Exit For
			End If
		Next n
		m_nStart = (n + 1)
		Return m_nSum
	End Function

    Private Sub RedoSum(ByRef RedoSumRtn As Integer) 'out
        RedoSumRtn = Compute()
    End Sub

    Private Sub Pause(ByRef PauseRtn As Integer) 'out
        PauseRtn = m_nSum
    End Sub

	Protected Overrides Sub OnFastRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer)
		Select Case sRequestID
		Case RemoteSumConst.idPauseRemSum
			M_I0_R1(Of Integer)(AddressOf Pause)
		Case Else
		End Select
	End Sub

	Protected Overrides Function OnSlowRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer) As Integer
		Select Case sRequestID
		Case RemoteSumConst.idDoSumRemSum
			M_I2_R1(Of Integer, Integer, Integer)(AddressOf DoSum)
		Case RemoteSumConst.idRedoSumRemSum
			M_I0_R1(Of Integer)(AddressOf RedoSum)
		Case Else
		End Select
		Return 0
	End Function
End Class

Public Class CMySocketProServer
	Inherits CSocketProServer
	Protected Overrides Function OnSettingServer() As Boolean
		'try amIntegrated and amMixed instead by yourself
        Config.AuthenticationMethod = USOCKETLib.tagAuthenticationMethod.amOwn

		'add service(s) into SocketPro server
		AddService()

		'You may set others here

		Return True 'true -- ok; false -- no listening server
	End Function

	Private m_RemSum As New CSocketProService(Of RemSumPeer)()
	'One SocketPro server supports any number of services. You can list them here!

	Private Sub AddService()
		Dim ok As Boolean

		'No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
		ok = m_RemSum.AddMe(RemoteSumConst.sidRemSum, 0, tagThreadApartment.taNone)
		'If ok is false, very possibly you have two services with the same service id!

		ok = m_RemSum.AddSlowRequest(RemoteSumConst.idDoSumRemSum)
		ok = m_RemSum.AddSlowRequest(RemoteSumConst.idRedoSumRemSum)

		'Add all of other services into SocketPro server here!
	End Sub
End Class

