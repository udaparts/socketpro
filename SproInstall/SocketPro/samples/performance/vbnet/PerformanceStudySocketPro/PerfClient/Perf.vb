Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports USOCKETLib

Public Class CPerf
	Inherits CAsyncAdohandler
	Public Sub New(ByVal cs As CClientSocket, ByVal ar As IAsyncResultsHandler)
		MyBase.New(PerfConst.sidCPerf, cs, ar)
	End Sub

	Public Sub New(ByVal cs As CClientSocket)
		MyBase.New(PerfConst.sidCPerf, cs)
	End Sub

	Public Sub New()
		MyBase.New(PerfConst.sidCPerf)
	End Sub

	Protected m_MyEchoRtn As String
    Public Sub MyEchoAsync(ByVal strInput As String)
        SendRequest(PerfConst.idMyEchoCPerf, strInput)
    End Sub

    Public Sub OpenRecordsAsync(ByVal strSQL As String)
        SendRequest(PerfConst.idOpenRecords, strSQL)
    End Sub

    'We can process returning results inside the function.
	Protected Overrides Sub OnResultReturned(ByVal sRequestID As Short, ByVal UQueue As CUQueue)
		Select Case sRequestID
			Case PerfConst.idMyEchoCPerf
				UQueue.Load(m_MyEchoRtn)
			Case PerfConst.idOpenRecords
			Case Else
				MyBase.OnResultReturned(sRequestID, UQueue) 'chain down for processing ADO.NET objects
		End Select
	End Sub
	Public Function MyEcho(ByVal strInput As String) As String
        MyEchoAsync(strInput)
		GetAttachedClientSocket().WaitAll()
		Return m_MyEchoRtn
	End Function

	Public Function OpenRecords(ByVal strSQL As String) As DataTable
		GetAttachedClientSocket().BeginBatching()
        OpenRecordsAsync(strSQL)
		GetAttachedClientSocket().Commit(True)
		GetAttachedClientSocket().WaitAll()
		Return m_AdoSerialier.CurrentDataTable
	End Function

	Public ReadOnly Property CurrentDataTable() As DataTable
		Get
			Return m_AdoSerialier.CurrentDataTable
		End Get
	End Property
End Class
