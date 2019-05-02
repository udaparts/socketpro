Imports System.Configuration
Imports System.Collections
Imports System.Web.Security
Imports System.Web.UI
Imports System.Web.UI.WebControls
Imports System.Web.UI.WebControls.WebParts
Imports System.Web.UI.HtmlControls

Namespace AdoWebAsync
	Partial Public Class _Default
		Inherits System.Web.UI.Page
		Private m_AsynAdoHandler As CMyAdoHandler
		Protected Sub btnExecute_Click(ByVal sender As Object, ByVal e As EventArgs)
			'Register two event handlers
			AddOnPreRenderCompleteAsync(New BeginEventHandler(AddressOf BeginAsyncOperation), New EndEventHandler(AddressOf EndAsyncOperation))
		End Sub

		Private Function BeginAsyncOperation(ByVal sender As Object, ByVal e As EventArgs, ByVal cb As AsyncCallback, ByVal state As Object) As IAsyncResult
			m_AsynAdoHandler = [Global].m_AsyncADO.Lock(100) 'timeout -- 0.1 second
			If m_AsynAdoHandler IsNot Nothing Then
				'Start batch
                Dim ok As Boolean = m_AsynAdoHandler.GetAttachedClientSocket().BeginBatching()

				'delete some records
                m_AsynAdoHandler.ExecuteNoQueryAsync("Delete from Shippers Where ShipperID > 3")

				Dim strSQL As String = "Insert into Shippers (CompanyName, Phone) Values ('"
				strSQL &= txtCompany.Text
				strSQL &= "', '"
				strSQL &= txtPhoneNumber.Text
				strSQL &= "')"

				'Insert a record
                m_AsynAdoHandler.ExecuteNoQueryAsync(strSQL)

				'Query records and fetch all of them
                m_AsynAdoHandler.GetDataTableAsync("Select * from Shippers")

				'remember a callback
				m_AsynAdoHandler.GetAttachedClientSocket().Callback = cb

				'Batch requests and remember a callback cb
				m_AsynAdoHandler.GetAttachedClientSocket().Commit(True)

				Return m_AsynAdoHandler.GetAttachedClientSocket()
			Else 'no socket is available
				'report the error that web server is very busy here
			End If
			Return Nothing
		End Function

		Private Sub EndAsyncOperation(ByVal ar As IAsyncResult)
			If m_AsynAdoHandler IsNot Nothing Then
				'binding the last table onto a grid view
				Dim dt As DataTable = m_AsynAdoHandler.CurrentDataTable
				gvRowset.DataSource = dt
				gvRowset.DataBind()

				'unlock the attached socket, and return it back into socket pool for reuse
				[Global].m_AsyncADO.Unlock(m_AsynAdoHandler)
			End If
		End Sub
	End Class
End Namespace
