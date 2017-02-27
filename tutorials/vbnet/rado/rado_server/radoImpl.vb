' **** including all of defines, service id(s) and request id(s) ***** 
Imports System
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide
Imports System.Data
Imports System.Data.SqlClient

'server implementation for service RAdo
Public Class RAdoPeer
	Inherits CAdoClientPeer

	Private m_sqlConnection As New SqlConnection("Data Source=localhost;Integrated Security=SSPI;Initial Catalog=AdventureWorksDW2012")

	<RequestAttr(radoConst.idGetDataSetRAdo, True)>
	Private Sub GetDataSet(ByVal sql0 As String, ByVal sql1 As String)
		Dim ds As New DataSet("MyDataSet")
		Try
			m_sqlConnection.Open()
			Dim cmd As New SqlCommand(sql0, m_sqlConnection)
			Dim adapter As New SqlDataAdapter(cmd)
			Dim cmd1 As New SqlCommand(sql1, m_sqlConnection)
			Dim adapter1 As New SqlDataAdapter(cmd1)
			adapter.Fill(ds, "Table1")
			adapter1.Fill(ds, "Table2")
			Send(ds)
		Catch err As Exception
			Console.WriteLine(err.Message)
		Finally
			m_sqlConnection.Close()
		End Try
	End Sub

	<RequestAttr(radoConst.idGetDataTableRAdo, True)>
	Private Sub GetDataTable(ByVal sql As String)
		Dim dr As IDataReader = Nothing
		Try
			m_sqlConnection.Open()
			Dim cmd As New SqlCommand(sql, m_sqlConnection)
			dr = cmd.ExecuteReader()
			Send(dr)
		Catch err As Exception
			Console.WriteLine(err.Message)
		Finally
			If dr IsNot Nothing Then
				dr.Close()
			End If
			m_sqlConnection.Close()
		End Try
	End Sub
End Class
