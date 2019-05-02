Imports System.Data
Imports System.Collections
Imports System.Collections.Generic
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports USOCKETLib

Partial Class _Default
    Inherits System.Web.UI.Page
    Private Shared m_lbRAdo As CMyRAdoLB = New CMyRAdoLB()
    Private Shared Function StartLB() As Boolean
        If m_lbRAdo.IsStarted() Then
            Return True
        End If
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
        pConnectionContext(3).m_strHost = "localhost"
        pConnectionContext(4).m_strHost = "127.0.0.1"
        For n = 0 To Count - 1
            pConnectionContext(n).m_nPort = 20901
            pConnectionContext(n).m_strPassword = "SocketPro"
            pConnectionContext(n).m_strUID = "PassOne"
            pConnectionContext(n).m_EncrytionMethod = USOCKETLib.tagEncryptionMethod.NoEncryption
            pConnectionContext(n).m_bZip = False
        Next n
        Return m_lbRAdo.StartSocketPool(pConnectionContext, 3, 2)
    End Function

    Protected Sub btnExecute_Click(ByVal sender As Object, ByVal e As System.EventArgs)
        gvSQL1.DataSource = Nothing
        gvSQL2.DataSource = Nothing
        gvSQL3.DataSource = Nothing
        If Not StartLB() Then
            Return
        End If
        Dim lstSQL As List(Of String) = New List(Of String)()
        lstSQL.Add(txtSQL1.Text)
        lstSQL.Add(txtSQL2.Text)
        lstSQL.Add(txtSQL3.Text)
        Dim tables As IList(Of DataTable) = m_lbRAdo.PrepareAndExecuteJobs(lstSQL)
        gvSQL1.DataSource = tables(0)
        gvSQL2.DataSource = tables(1)
        gvSQL3.DataSource = tables(2)
        gvSQL1.DataBind()
        gvSQL2.DataBind()
        gvSQL3.DataBind()
    End Sub
End Class

Class CRAdo
    Inherits CAsyncAdohandler
    Const sidCRAdo As Integer = USOCKETLib.tagOtherDefine.odUserServiceIDMin + 202
    Const idGetDataSetCRAdo As Short = USOCKETLib.tagOtherDefine.odUserRequestIDMin + 0
    Const idGetDataReaderCRAdo As Short = (idGetDataSetCRAdo + 1)

    Public Sub New()
        MyBase.New(sidCRAdo)
    End Sub

    Public Sub New(ByVal cs As CClientSocket)
        MyBase.New(sidCRAdo, cs)
    End Sub

    Public Sub New(ByVal cs As CClientSocket, ByVal DefaultAsyncResultsHandler As IAsyncResultsHandler)
        MyBase.New(sidCRAdo, cs, DefaultAsyncResultsHandler)
    End Sub

    Public ReadOnly Property ReturnTable() As DataTable
        Get
            Return m_AdoSerialier.CurrentDataTable
        End Get
    End Property


    Public Sub GetDataReaderAsyn(ByVal strSQL As String)
        SendRequest(idGetDataReaderCRAdo, strSQL)
    End Sub

    Public m_JobTable As SortedList(Of Long, DataTable)

    'When a result comes from a remote SocketPro server, the below virtual function will be called.
    'We always process returning results inside the function.
    Protected Overrides Sub OnResultReturned(ByVal sRequestID As Short, ByVal UQueue As CUQueue)
        Select Case sRequestID
            Case CAsyncAdoSerializationHelper.idDataSetHeaderArrive, CAsyncAdoSerializationHelper.idDataTableHeaderArrive, CAsyncAdoSerializationHelper.idDataReaderHeaderArrive, CAsyncAdoSerializationHelper.idDataReaderRecordsArrive, CAsyncAdoSerializationHelper.idEndDataTable, CAsyncAdoSerializationHelper.idEndDataReader, CAsyncAdoSerializationHelper.idDataTableRowsArrive, CAsyncAdoSerializationHelper.idEndDataSet
                MyBase.OnResultReturned(sRequestID, UQueue) 'chain down to CAsyncAdohandler for processing
            Case idGetDataReaderCRAdo
            Case Else
        End Select
    End Sub
End Class

Class CMyRAdoLB
    Inherits CSocketPoolEx(Of CRAdo)
    Private m_cs As Object = New Object()

    Protected Overrides Sub OnJobDone(ByVal Handler As CRAdo, ByVal JobContext As IJobContext)
        Dim RAdo As CRAdo = JobContext.Identity
        SyncLock (m_cs)
            RAdo.m_JobTable(JobContext.JobId) = Handler.ReturnTable
        End SyncLock
    End Sub

    Public Function PrepareAndExecuteJobs(ByVal lstSQL As List(Of String)) As IList(Of DataTable)

        Dim tables As IList(Of DataTable)

        If lstSQL Is Nothing Or lstSQL.Count = 0 Then
            Throw New InvalidOperationException("Must pass in a list of SQL statements")
        End If

        Dim handler As CRAdo = JobManager.LockIdentity()
        If handler Is Nothing Then
            Throw New InvalidOperationException("ADO loading balance is down")
        End If

        handler.m_JobTable = New SortedList(Of Long, DataTable)()
        For Each strSQL As String In lstSQL
            handler.GetDataReaderAsyn(strSQL)
        Next

        'wait until all of jobs with this identity are completed.
        JobManager.Wait(handler)

        tables = handler.m_JobTable.Values

        'release identity
        JobManager.UnlockIdentity(handler)
        Return tables
    End Function
End Class