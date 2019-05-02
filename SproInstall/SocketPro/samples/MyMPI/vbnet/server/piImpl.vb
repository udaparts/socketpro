' **** including all of defines, service id(s) and request id(s) ***** 
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide
Imports USOCKETLib 'you may need it for accessing various constants
'server implementation for service CPPi
Public Class CPPiPeer
	Inherits CClientPeer
    Protected Sub Compute(ByVal dStart As Double, ByVal dStep As Double, ByVal nNum As Integer, ByRef ComputeRtn As Double)
        Dim n As Integer
        Dim n100 As Integer = nNum \ 100
        Dim dX As Double = dStart
        dX += dStep / 2
        Dim dd As Double = dStep * 4.0
        ComputeRtn = 0.0
        For n = 0 To nNum - 1
            dX += dStep
            ComputeRtn += dd / (1 + dX * dX)
            If n100 > 0 AndAlso ((n + 1) Mod n100) = 0 AndAlso n > 0 Then
                If IsCanceled OrElse IsClosing() Then
                    Exit For
                End If
            End If
        Next n
    End Sub

	Protected Overrides Sub OnFastRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer)

	End Sub

	Protected Overrides Function OnSlowRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer) As Integer
		Select Case sRequestID
		Case piConst.idComputeCPPi
			Dim dStart As Double = 0
			Dim dStep As Double = 0
			Dim nNum As Integer = 0
			Dim ComputeRtn As Double
			m_UQueue.Pop(dStart)
			m_UQueue.Pop(dStep)
			m_UQueue.Pop(nNum)
			Compute(dStart, dStep, nNum, ComputeRtn)
			SendResult(sRequestID, ComputeRtn)
		Case Else
		End Select
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

	Private m_CPPi As New CSocketProService(Of CPPiPeer)()
	Private Sub AddService()
        Dim ok As Boolean

        'No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
        ok = m_CPPi.AddMe(piConst.sidCPPi, 0, tagThreadApartment.taNone)
        'If ok is false, very possibly you have two services with the same service id!

        ok = m_CPPi.AddSlowRequest(piConst.idComputeCPPi)
	End Sub
End Class

