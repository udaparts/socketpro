Imports System.Configuration
Imports System.Collections
Imports System.Web.Security
Imports System.Web.UI
Imports System.Web.UI.WebControls
Imports System.Web.UI.WebControls.WebParts
Imports System.Web.UI.HtmlControls
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports SocketProAdapter.ClientSide.RemoteDB

Namespace AdoWebAsync
	Partial Public Class Parallel
		Inherits System.Web.UI.Page

		Protected Sub btnParallel_Click(ByVal sender As Object, ByVal e As EventArgs)
			'lock two instances of data accessing handlers, CAsynDBLite and CMyAdoHandler
			Dim AsynDBLiteHandler As CAsynDBLite = [Global].m_AsyncRdb.Lock(100)
			Dim AsyncAdoHandler As CMyAdoHandler = [Global].m_AsyncADO.Lock(100)

			If AsynDBLiteHandler Is Nothing OrElse AsyncAdoHandler Is Nothing Then
				'indicate error message like server is too busy
				Return
			End If

			If Not AsynDBLiteHandler.DBConnected Then
				'connect to DB one time only
				AsynDBLiteHandler.ConnectDB(Nothing) 'using global oledb connection string at server side
				AsynDBLiteHandler.GetAttachedClientSocket().WaitAll()
			End If

			'execute two SQLs in parallel with two different instances, AsynDBLiteHandler and AsyncAdoHandler
			AsynDBLiteHandler.OpenRowset(txtSQL1.Text, "SQL1")
            AsyncAdoHandler.GetDataTableAsync(txtSQL2.Text)

			'Block until two requests are processed
			AsyncAdoHandler.GetAttachedClientSocket().WaitAll()
			AsynDBLiteHandler.GetAttachedClientSocket().WaitAll()

			gvQueryOne.DataSource = AsynDBLiteHandler.CurrentDataTable
			gvQueryTwo.DataSource = AsyncAdoHandler.CurrentDataTable

			gvQueryOne.DataBind()
			gvQueryTwo.DataBind()

			'unlock sockets and their associated DB handlers, and return them back into pool for reuse
			[Global].m_AsyncRdb.Unlock(AsynDBLiteHandler)
			[Global].m_AsyncADO.Unlock(AsyncAdoHandler)
		End Sub
	End Class
End Namespace
