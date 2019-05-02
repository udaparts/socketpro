#Const USE_SQLCLIENT = False

#If USE_SQLCLIENT Then

Imports Microsoft.VisualBasic
Imports System.Data.SqlClient
#Else
Imports System.Data.OleDb
#End If

Imports System
Imports System.Data


Namespace DefMyCallsInterface
	Public Interface IMyCalls
        Function OpenRowset(ByVal strSQL As String) As DataTable
		Function MyEcho(ByVal strInput As String) As String
	End Interface

	Public Class CMyCallsImpl
		Inherits MarshalByRefObject
        Implements IMyCalls

#If USE_SQLCLIENT Then
        Dim conn As SqlConnection = New SqlConnection("server=localhost\sqlexpress;Integrated Security=SSPI;database=northwind")
#Else
        Dim conn As OleDbConnection = New OleDbConnection("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=C:\Program Files\udaparts\SocketPro\bin\nwind3.mdb")
#End If

#Region "IMyCalls Members"
        Public Function OpenRowset(ByVal strSQL As String) As System.Data.DataTable Implements IMyCalls.OpenRowset
            Dim dt As DataTable = Nothing
            If conn.State <> ConnectionState.Open Then
                conn.Open()
            End If
#If USE_SQLCLIENT Then
            Dim cmd As SqlCommand = New SqlCommand(strSQL, conn)
            Dim adapter As SqlDataAdapter = New SqlDataAdapter(cmd)
#Else
			Dim cmd As OleDbCommand = New OleDbCommand(strSQL, conn)
			Dim adapter As OleDbDataAdapter = New OleDbDataAdapter(cmd)
#End If
            dt = New DataTable("MyDataTable")
            adapter.Fill(dt)
            'dt.SchemaSerializationMode = SchemaSerializationMode.ExcludeSchema;
            dt.RemotingFormat = SerializationFormat.Binary
            Return dt
        End Function
		Public Function MyEcho(ByVal strInput As String) As String Implements IMyCalls.MyEcho
			Return strInput
		End Function
#End Region

	End Class
End Namespace
