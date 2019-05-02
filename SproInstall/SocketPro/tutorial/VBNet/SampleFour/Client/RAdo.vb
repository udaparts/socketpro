Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports USOCKETLib

Public Class CRAdo
    Inherits CAsyncAdohandler
    Public Sub New()
        MyBase.New(RAdoConst.sidCRAdo)
    End Sub

    Public Sub New(ByVal cs As CClientSocket)
        MyBase.New(RAdoConst.sidCRAdo, cs)
    End Sub

    Public Sub New(ByVal cs As CClientSocket, ByVal DefaultAsyncResultsHandler As IAsyncResultsHandler)
        MyBase.New(RAdoConst.sidCRAdo, cs, DefaultAsyncResultsHandler)
    End Sub

    Public ReadOnly Property CurrentDataTable() As DataTable
        Get
            Return m_AdoSerialier.CurrentDataTable
        End Get
    End Property

    Public Function GetDataSet(ByVal strSQL0 As String, ByVal strSQL2 As String) As DataSet
        Dim bProcessRy As Boolean = ProcessR0(RAdoConst.idGetDataSetCRAdo, strSQL0, strSQL2)
        Return m_AdoSerialier.CurrentDataSet
    End Function

    Public Function GetDataReader(ByVal strSQL As String) As DataTable
        Dim bProcessRy As Boolean = ProcessR0(RAdoConst.idGetDataReaderCRAdo, strSQL)
        Return m_AdoSerialier.CurrentDataTable
    End Function

    Public Function SendDataSet(ByVal ds As DataSet) As Boolean
        Dim b As Boolean
        Send(ds)
        Dim bProcessRy As Boolean = ProcessR1(RAdoConst.idSendDataSetCRAdo, b)
        Return b
    End Function

    Public Function SendDataReader(ByVal dr As IDataReader) As Boolean
        Dim b As Boolean
        Send(dr)
        Dim bProcessRy As Boolean = ProcessR1(RAdoConst.idSendDataReaderCRAdo, b)
        Return b
    End Function
End Class