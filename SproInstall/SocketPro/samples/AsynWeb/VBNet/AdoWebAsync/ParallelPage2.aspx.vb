Imports System.Configuration
Imports System.Collections
Imports System.Web.Security
Imports System.Web.UI
Imports System.Web.UI.WebControls
Imports System.Web.UI.WebControls.WebParts
Imports System.Web.UI.HtmlControls
Imports USOCKETLib
Imports UDBLib
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports SocketProAdapter.ClientSide.RemoteDB

Namespace AdoWebAsync
	Partial Public Class ParallelPage
		Inherits System.Web.UI.Page
		Private m_dbCmd As CAsynDBLite
		Private m_dbQuery As CMyAdoHandler
		Private m_nFirst As Integer
		Private Function BeginAsyncOperationCmd(ByVal sender As Object, ByVal e As EventArgs, ByVal cb As AsyncCallback, ByVal state As Object) As IAsyncResult
			m_dbCmd = [Global].m_AsyncRdb.Lock()
			If m_dbCmd IsNot Nothing Then
				Dim cs As CClientSocket = m_dbCmd.GetAttachedClientSocket()
				cs.Callback = cb 'remember a callback
				If Not m_dbCmd.DBConnected Then
					m_dbCmd.ConnectDB(Nothing) 'use global oledb connection string at server side
				End If
				m_dbCmd.ExecuteNonQuery(txtCmd.Text)
				Return cs
			End If
			Return Nothing
		End Function

		Private Sub EndAsyncOperationCmd(ByVal ar As IAsyncResult)
			If m_dbCmd IsNot Nothing Then
				If m_nFirst = 0 Then
					m_nFirst = 1
					lblInfo.Text = "Command First"
				End If
				[Global].m_AsyncRdb.Unlock(m_dbCmd)
			End If
		End Sub

		Private Sub TimeoutAsyncOperationCmd(ByVal ar As IAsyncResult)
			lblInfo.Text = "Data temporarily unavailable"
		End Sub

		Private Function BeginAsyncOperationQuery(ByVal sender As Object, ByVal e As EventArgs, ByVal cb As AsyncCallback, ByVal state As Object) As IAsyncResult
			m_dbQuery = [Global].m_AsyncADO.Lock()
			If m_dbQuery IsNot Nothing Then
				Dim cs As CClientSocket = m_dbQuery.GetAttachedClientSocket()
				cs.Callback = cb 'remember callback;
                m_dbQuery.GetDataTableAsync(txtQuery.Text)
				Return cs
			End If
			Return Nothing
		End Function

		Private Sub EndAsyncOperationQuery(ByVal ar As IAsyncResult)
			If m_dbQuery IsNot Nothing Then
				Dim dt As DataTable = m_dbQuery.CurrentDataTable
				gvQuery.DataSource = dt
				gvQuery.DataBind()
				[Global].m_AsyncADO.Unlock(m_dbQuery)
				If m_nFirst = 0 Then
					m_nFirst = 2
					lblInfo.Text = "Query First"
				End If
			End If
		End Sub

		Private Sub TimeoutAsyncOperationQuery(ByVal ar As IAsyncResult)
			lblInfo.Text = "Data temporarily unavailable"
		End Sub

		Protected Sub idDoMultiTask_Click(ByVal sender As Object, ByVal e As EventArgs)
			m_nFirst = 0
			Dim task As New PageAsyncTask(AddressOf BeginAsyncOperationQuery, AddressOf EndAsyncOperationQuery, AddressOf TimeoutAsyncOperationQuery, Nothing, True)
			RegisterAsyncTask(task)

			task = New PageAsyncTask(AddressOf BeginAsyncOperationCmd, AddressOf EndAsyncOperationCmd, AddressOf TimeoutAsyncOperationCmd, Nothing, True)
			RegisterAsyncTask(task)
		End Sub
	End Class
End Namespace
