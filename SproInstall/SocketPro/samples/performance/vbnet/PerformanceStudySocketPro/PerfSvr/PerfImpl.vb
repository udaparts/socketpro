' **** including all of defines, service id(s) and request id(s) ***** 

#Const USE_SQLCLIENT = False

#If USE_SQLCLIENT Then
Imports System.Data.SqlClient
#Else
Imports System.Data.OleDb
#End If

Imports System.Runtime.InteropServices
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide
Imports USOCKETLib 'you may need it for accessing various constants

'server implementation for service CPerf
Public Class CPerfPeer 'CClientPeer
    Inherits CAdoClientPeer

#If USE_SQLCLIENT Then
    private cmd As SqlCommand
	Private conn As New SqlConnection("server=localhost\sqlexpress;Integrated Security=SSPI;database=northwind")
#Else
    Private cmd As OleDbCommand
    Private conn As New OleDbConnection("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=c:\Program Files\udaparts\SocketPro\bin\nwind3.mdb")
#End If

    Protected Overrides Sub OnSwitchFrom(ByVal nSvsID As Integer)
#If USE_SQLCLIENT Then
        cmd = new SqlCommand("", conn)
#Else
        cmd = New OleDbCommand("", conn)
#End If
    End Sub

	Protected Overrides Sub OnReleaseResource(ByVal bClosing As Boolean, ByVal nInfo As Integer)
        cmd.Dispose()
        If conn.State = ConnectionState.Open Then
            conn.Close()
        End If
	End Sub
	Private Sub OpenRecords(ByVal strSQL As String)
		Dim dr As IDataReader = Nothing
		If conn.State <> ConnectionState.Open Then
			Try
				conn.Open()
			Catch err As Exception
				Console.WriteLine(err.Message)
				Return
			End Try
        End If
        cmd.CommandText = strSQL
		Try
			dr = cmd.ExecuteReader()
		Catch err As Exception
			Console.WriteLine(err.Message)
			Return
		End Try
        Send(dr)
        dr.Close()
	End Sub

	Protected Sub MyEcho(ByVal strInput As String, ByRef MyEchoRtn As String)
		MyEchoRtn = strInput
	End Sub

	Protected Overrides Sub OnFastRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer)
		Select Case sRequestID
            Case PerfConst.idMyEchoCPerf
                M_I1_R1(Of String, String)(AddressOf MyEcho)
            Case Else
        End Select
	End Sub

	Protected Overrides Function OnSlowRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer) As Integer
		Select Case sRequestID
            Case PerfConst.idOpenRecords
                M_I1_R0(Of String)(AddressOf OpenRecords)
            Case Else
        End Select
		Return 0
	End Function
End Class

Public Class CMySocketProServer
	Inherits CSocketProServer
	Protected Overrides Function OnIsPermitted(ByVal hSocket As Integer, ByVal nSvsID As Integer) As Boolean
		Dim strUID As String = GetUserID(hSocket)

		'password is available ONLY IF authentication method to either amOwn or amMixed
		Dim strPassword As String = GetPassword(hSocket)

		Console.WriteLine("For service = {0}, User ID = {1}, Password = {2}", nSvsID, strUID, strPassword)

		Return True 'true -- permission given; otherwise permission denied
	End Function

	Protected Overrides Sub OnAccept(ByVal hSocket As Integer, ByVal nError As Integer)
		If nError = 0 Then
			Console.WriteLine("A socket is initially establised")
		End If
	End Sub

	Protected Overrides Sub OnClose(ByVal hSocket As Integer, ByVal nError As Integer)
		Console.WriteLine("A socket is closed with error code = " & nError)
	End Sub

	Protected Overrides Function OnSettingServer() As Boolean
        AddService()

        Return True
    End Function

	Private m_CPerf As New CSocketProService(Of CPerfPeer)()
	Private Sub AddService()
        Dim ok As Boolean

        'No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
        ok = m_CPerf.AddMe(PerfConst.sidCPerf, 0, tagThreadApartment.taNone)
        'If ok is false, very possibly you have two services with the same service id!

        ok = m_CPerf.AddSlowRequest(PerfConst.idOpenRecords)
	End Sub
End Class

