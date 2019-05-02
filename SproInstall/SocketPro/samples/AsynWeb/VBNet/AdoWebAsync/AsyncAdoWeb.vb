Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports USOCKETLib

Public Class CMyAdoHandler
	Inherits CAsyncAdohandler
	Public Sub New()
		MyBase.New(AsyncAdoWebConst.sidCAsyncAdo)
	End Sub

	Public m_strError_GetDataTable As String
    Public Sub GetDataTableAsync(ByVal strSQL As String)
        SendRequest(AsyncAdoWebConst.idGetDataTableCAsyncAdo, strSQL)
    End Sub

	Public m_strError_ExecuteNoQuery As String
	Public m_ExecuteNoQueryRtn As Integer
    Public Sub ExecuteNoQueryAsync(ByVal strSQL As String)
        SendRequest(AsyncAdoWebConst.idExecuteNoQueryCAsyncAdo, strSQL)
    End Sub

	'We can process returning results inside the function.
	Protected Overrides Sub OnResultReturned(ByVal sRequestID As Short, ByVal UQueue As CUQueue)
		Select Case sRequestID
			Case CAsyncAdoSerializationHelper.idDataReaderHeaderArrive, CAsyncAdoSerializationHelper.idDataReaderRecordsArrive, CAsyncAdoSerializationHelper.idEndDataReader
				MyBase.OnResultReturned(sRequestID, UQueue)
			Case AsyncAdoWebConst.idGetDataTableCAsyncAdo
				UQueue.Load(m_strError_GetDataTable)
			Case AsyncAdoWebConst.idExecuteNoQueryCAsyncAdo
				UQueue.Load(m_strError_ExecuteNoQuery)
				UQueue.Pop(m_ExecuteNoQueryRtn)
			Case Else
		End Select
	End Sub

	Public ReadOnly Property CurrentDataTable() As DataTable
		Get
			Return m_AdoSerialier.CurrentDataTable
		End Get
	End Property
	Public Function GetDataTable(ByVal strSQL As String, <System.Runtime.InteropServices.Out()> ByRef strError As String) As DataTable
        GetDataTableAsync(strSQL)
		GetAttachedClientSocket().WaitAll()
		strError = m_strError_GetDataTable
		Return m_AdoSerialier.CurrentDataTable
	End Function

	Public Function ExecuteNoQuery(ByVal strSQL As String, <System.Runtime.InteropServices.Out()> ByRef strError As String) As Integer
        ExecuteNoQueryAsync(strSQL)
		GetAttachedClientSocket().WaitAll()
		strError = m_strError_ExecuteNoQuery
		Return m_ExecuteNoQueryRtn
	End Function
End Class
