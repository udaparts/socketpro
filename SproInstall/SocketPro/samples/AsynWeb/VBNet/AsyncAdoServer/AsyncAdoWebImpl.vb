' **** including all of defines, service id(s) and request id(s) ***** 
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide
Imports USOCKETLib 'you may need it for accessing various constants
Imports System.Runtime.InteropServices
Imports System.Data.SqlClient

'server implementation for service CMyAdoHandler
Public Class CAsyncAdoPeer 'CClientPeer
	Inherits CAdoClientPeer
    Private Shared m_strConn As String = "server=localhost\\sqlexpress;Integrated Security=SSPI;database=northwind"

	Protected Sub GetDataTable(ByVal strSQL As String, <System.Runtime.InteropServices.Out()> ByRef strError As String)
		strError = Nothing
		Dim dr As IDataReader = Nothing
		Dim conn As SqlConnection = Nothing
		Try
			conn = New SqlConnection(m_strConn)
			conn.Open()
			Dim cmd As New SqlCommand(strSQL, conn)
			dr = cmd.ExecuteReader()
		Catch err As Exception
			conn.Close()
			Console.WriteLine(err.Message)
			strError = err.Message
			Return
		End Try
		Send(dr)
		conn.Close()
	End Sub

	Protected Sub ExecuteNoQuery(ByVal strSQL As String, <System.Runtime.InteropServices.Out()> ByRef strError As String, <System.Runtime.InteropServices.Out()> ByRef ExecuteNoQueryRtn As Integer)
		strError = Nothing
		ExecuteNoQueryRtn = 0
		Dim conn As SqlConnection = Nothing
		Try
			conn = New SqlConnection(m_strConn)
			conn.Open()
			Dim cmd As New SqlCommand(strSQL, conn)
			ExecuteNoQueryRtn = cmd.ExecuteNonQuery()
		Catch err As Exception
			conn.Close()
			Console.WriteLine(err.Message)
			strError = err.Message
			Return
		End Try
		conn.Close()
	End Sub

	Protected Overrides Sub OnFastRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer)

	End Sub

	Protected Overrides Function OnSlowRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer) As Integer
		Select Case sRequestID
		Case AsyncAdoWebConst.idGetDataTableCAsyncAdo
			Dim strSQL As String = Nothing
			Dim strError As String
			m_UQueue.Load(strSQL)
			Dim bBatching As Boolean = IsBatching
			If Not bBatching Then
				StartBatching()
			End If
			GetDataTable(strSQL, strError)
			SendResult(sRequestID, strError)
			If Not bBatching Then
				CommitBatching()
			End If
		Case AsyncAdoWebConst.idExecuteNoQueryCAsyncAdo
			Dim strSQL As String = Nothing
			Dim strError As String
			Dim ExecuteNoQueryRtn As Integer
			m_UQueue.Load(strSQL)
			Dim bBatching As Boolean = IsBatching
			If Not bBatching Then
				StartBatching()
			End If
			ExecuteNoQuery(strSQL, strError, ExecuteNoQueryRtn)
			SendResult(sRequestID, strError, ExecuteNoQueryRtn)
			If Not bBatching Then
				CommitBatching()
			End If
		Case Else
		End Select
		Return 0
	End Function
End Class

Public Class CMySocketProServer
	Inherits CSocketProServer
	Protected Overrides Function OnIsPermitted(ByVal hSocket As Integer, ByVal nSvsID As Integer) As Boolean
		'give permission to all
		Return True
	End Function

	Protected Overrides Sub OnAccept(ByVal hSocket As Integer, ByVal nError As Integer)
		'when a socket is initially established
	End Sub

	Protected Overrides Sub OnClose(ByVal hSocket As Integer, ByVal nError As Integer)
		'when a socket is closed
	End Sub

	Protected Overrides Function OnSettingServer() As Boolean
		'try amIntegrated and amMixed
		Config.AuthenticationMethod = tagAuthenticationMethod.amOwn

		AddService()

		Return True
	End Function

	Private m_CAsyncAdo As New CSocketProService(Of CAsyncAdoPeer)()

    <DllImport("uodbsvr.dll")> _
 Private Shared Sub SetGlobalOLEDBConnectionString(<MarshalAs(UnmanagedType.LPWStr)> ByVal strOLEDBConnection As String)
    End Sub

	Private Sub AddService()
        Dim ok As Boolean

        'No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
        ok = m_CAsyncAdo.AddMe(AsyncAdoWebConst.sidCAsyncAdo, 0, tagThreadApartment.taNone)
        'If ok is false, very possibly you have two services with the same service id!

        ok = m_CAsyncAdo.AddSlowRequest(AsyncAdoWebConst.idGetDataTableCAsyncAdo)
        ok = m_CAsyncAdo.AddSlowRequest(AsyncAdoWebConst.idExecuteNoQueryCAsyncAdo)

		'load UDAParts generic remote database service using MS OLEDB technology
		Dim libRDB As IntPtr = CBaseService.AddALibrary("uodbsvr.dll")
		If libRDB = IntPtr.Zero Then
			Console.WriteLine("Can't load remote database service library uodbsvr.dll!")
		Else
			SetGlobalOLEDBConnectionString("Provider=sqloledb;Data Source=CHARLIEDEVVM;User ID=sa;Password=cye;Initial Catalog=Northwind")
		End If
	End Sub
End Class

