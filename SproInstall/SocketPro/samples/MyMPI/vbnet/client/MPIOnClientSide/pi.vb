Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports USOCKETLib


Public Class CPPi
	Inherits CAsyncServiceHandler
	Public Sub New(ByVal cs As CClientSocket, ByVal pDefaultAsyncResultsHandler As IAsyncResultsHandler)
		MyBase.New(piConst.sidCPPi, cs, pDefaultAsyncResultsHandler)
	End Sub

	Public Sub New(ByVal cs As CClientSocket)
		MyBase.New(piConst.sidCPPi, cs)
	End Sub

	Public Sub New()
		MyBase.New(piConst.sidCPPi)
	End Sub

	Public m_ComputeRtn As Double
    Public Sub ComputeAsync(ByVal dStart As Double, ByVal dStep As Double, ByVal nNum As Integer)
        m_ComputeRtn = 0.0
        SendRequest(piConst.idComputeCPPi, dStart, dStep, nNum)
    End Sub

	'When a result comes from a remote SocketPro server, the below virtual function will be called.
	'We always process returning results inside the function.
	Protected Overrides Sub OnResultReturned(ByVal sRequestID As Short, ByVal UQueue As CUQueue)
		Select Case sRequestID
			Case piConst.idComputeCPPi
				UQueue.Pop(m_ComputeRtn)
			Case Else
		End Select
	End Sub
	Public Function Compute(ByVal dStart As Double, ByVal dStep As Double, ByVal nNum As Integer) As Double
        ComputeAsync(dStart, dStep, nNum)
		GetAttachedClientSocket().WaitAll()
		Return m_ComputeRtn
	End Function
End Class
