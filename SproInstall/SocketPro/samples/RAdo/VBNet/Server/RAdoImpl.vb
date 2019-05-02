' **** including all of defines, service id(s) and request id(s) ***** 
'#define USE_SQLCLIENT

#If USE_SQLCLIENT Then
Imports System.Data.SqlClient
#Else
Imports System.Data.OleDb
#End If

Imports SocketProAdapter
Imports SocketProAdapter.ServerSide
Imports USOCKETLib 'you may need it for accessing various constants

'server implementation for service CRAdo
Public Class CRAdoPeer 'CClientPeer
	Inherits CAdoClientPeer
	Private m_bSuc As Boolean = False

	Protected Sub GetDataSet(ByVal strSQL0 As String, ByVal strSQL1 As String)
		Dim ds As New DataSet("MyDataSet")
#If USE_SQLCLIENT Then
		Dim conn As New SqlConnection("server=localhost\sqlexpress;Integrated Security=SSPI;database=northwind")
#Else
        Dim conn As New OleDbConnection("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=c:\Program Files\udaparts\SocketPro\bin\nwind3.mdb")
#End If
		Try
			conn.Open()
		Catch err As Exception
			Console.WriteLine(err.Message)
			Return
		End Try
#If USE_SQLCLIENT Then
		Dim cmd As New SqlCommand(strSQL0, conn)
		Dim adapter As New SqlDataAdapter(cmd)
		Dim cmd1 As New SqlCommand(strSQL1, conn)
		Dim adapter1 As New SqlDataAdapter(cmd1)
#Else
		Dim cmd As New OleDbCommand(strSQL0, conn)
		Dim adapter As New OleDbDataAdapter(cmd)
		Dim cmd1 As New OleDbCommand(strSQL1, conn)
		Dim adapter1 As New OleDbDataAdapter(cmd1)
#End If
		Try
			adapter.Fill(ds, "Table1")
			adapter1.Fill(ds, "Table2")
		Catch err As Exception
			Console.WriteLine(err.Message)
			conn.Close()
			Return
		End Try
		Send(ds)
		conn.Close()
	End Sub

	Protected Sub GetDataReader(ByVal strSQL As String)
		Dim dr As IDataReader = Nothing
#If USE_SQLCLIENT Then
		Dim conn As New SqlConnection("server=localhost\sqlexpress;Integrated Security=SSPI;database=northwind")
#Else
        Dim conn As New OleDbConnection("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=c:\Program Files\udaparts\SocketPro\bin\nwind3.mdb")
#End If
		Try
			conn.Open()
		Catch err As Exception
			Console.WriteLine(err.Message)
			Return
		End Try

#If USE_SQLCLIENT Then
		Dim cmd As New SqlCommand(strSQL, conn)
#Else
		Dim cmd As New OleDbCommand(strSQL, conn)
#End If
		Try
			dr = cmd.ExecuteReader()
		Catch err As Exception
			Console.WriteLine(err.Message)
			conn.Close()
			Return
		End Try
        Send(dr)
        dr.Close()
		conn.Close()
	End Sub

    Protected Sub SendDataSet(ByRef SendDataSetRtn As Boolean)
        Dim ds As DataSet = m_AdoSerialier.CurrentDataSet

        'do whatever you like here ......

        SendDataSetRtn = m_bSuc
    End Sub

    Protected Sub SendDataReader(ByRef SendDataReaderRtn As Boolean)
        Dim dt As DataTable = m_AdoSerialier.CurrentDataTable

        'do whatever you like here ......

        SendDataReaderRtn = m_bSuc
    End Sub

	Protected Overrides Sub OnFastRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer)
		Select Case sRequestID
            Case CAsyncAdoSerializationHelper.idDataSetHeaderArrive, CAsyncAdoSerializationHelper.idDataTableHeaderArrive, CAsyncAdoSerializationHelper.idDataReaderHeaderArrive, CAsyncAdoSerializationHelper.idEndDataTable, CAsyncAdoSerializationHelper.idEndDataReader, CAsyncAdoSerializationHelper.idEndDataSet
                MyBase.OnFastRequestArrive(sRequestID, nLen) 'chain down to CAdoClientPeer for processing
			Case RAdoConst.idSendDataSetCRAdo
				M_I0_R1(Of Boolean)(AddressOf SendDataSet)
			Case RAdoConst.idSendDataReaderCRAdo
				M_I0_R1(Of Boolean)(AddressOf SendDataReader)
			Case Else
				SendResult(sRequestID)
		End Select
	End Sub

	Protected Overrides Function OnSlowRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer) As Integer
		Select Case sRequestID
            Case CAsyncAdoSerializationHelper.idDataReaderRecordsArrive, CAsyncAdoSerializationHelper.idDataTableRowsArrive
                MyBase.OnSlowRequestArrive(sRequestID, nLen) 'chain down to CAdoClientPeer for processing
			Case RAdoConst.idGetDataSetCRAdo
				M_I2_R0(Of String, String)(AddressOf GetDataSet)
			Case RAdoConst.idGetDataReaderCRAdo
				M_I1_R0(Of String)(AddressOf GetDataReader)
		Case Else
			SendResult(sRequestID)
		End Select
		Return 0
	End Function
End Class

Public Class CMySocketProServer
	Inherits CSocketProServer
	Protected Overrides Function OnIsPermitted(ByVal hSocket As Integer, ByVal nSvsID As Integer) As Boolean
		Console.WriteLine("Switch to service = " & nSvsID)
		'give permission to all
		Return True
	End Function

	Protected Overrides Sub OnAccept(ByVal hSocket As Integer, ByVal nError As Integer)
		'when a socket is initially established
	End Sub

	Protected Overrides Sub OnClose(ByVal hSocket As Integer, ByVal nError As Integer)
		Console.WriteLine("Socket closed = " & hSocket)
	End Sub

	Protected Overrides Function OnSettingServer() As Boolean
		'try amIntegrated and amMixed
		Config.AuthenticationMethod = tagAuthenticationMethod.amOwn

		AddService()

		Return True
	End Function

	Private m_CRAdo As New CSocketProService(Of CRAdoPeer)()

	Private Sub AddService()
        Dim ok As Boolean

        'No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
        ok = m_CRAdo.AddMe(RAdoConst.sidCRAdo, 0, tagThreadApartment.taNone)
        'If ok is false, very possibly you have two services with the same service id!

        ok = m_CRAdo.AddSlowRequest(RAdoConst.idGetDataSetCRAdo)
        ok = m_CRAdo.AddSlowRequest(RAdoConst.idGetDataReaderCRAdo)
        ok = m_CRAdo.AddSlowRequest(CAsyncAdoSerializationHelper.idDataReaderRecordsArrive)
        ok = m_CRAdo.AddSlowRequest(CAsyncAdoSerializationHelper.idDataTableRowsArrive)
	End Sub
End Class

