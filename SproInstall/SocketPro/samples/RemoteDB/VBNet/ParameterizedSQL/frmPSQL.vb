Imports Microsoft.VisualBasic
Imports System
Imports System.Collections.Generic
Imports System.ComponentModel
Imports System.Data
Imports System.Drawing
Imports System.Text
Imports System.Windows.Forms
Imports USOCKETLib
Imports UDBLib
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports SocketProAdapter.ClientSide.RemoteDB

Namespace ParameterizedSQL
	Public Partial Class frmPSQL
		Inherits Form
		Private m_AsynDBLite As CAsynDBLiteEx = New CAsynDBLiteEx()
		Private m_ClientSocket As CClientSocket = New CClientSocket()

		Public Sub New()
			InitializeComponent()
		End Sub

		Private Sub OnSocketConnected(ByVal hSocket As Integer, ByVal nError As Integer)
			If nError = 0 Then
				m_ClientSocket.SetUID("SocketPro")
				m_ClientSocket.SetPassword("PassOne")
				m_ClientSocket.SwitchTo(m_AsynDBLite)
				m_AsynDBLite.ConnectDB("Provider=sqlncli;Data Source=localhost\sqlexpress;Initial Catalog=northwind;Integrated Security=SSPI")
				m_ClientSocket.WaitAll()
				btnTestPSQL.Enabled = m_AsynDBLite.DBConnected
			End If
		End Sub

		Private Sub OnSocketClosed(ByVal hSocket As Integer, ByVal nError As Integer)
			btnTestPSQL.Enabled = False
		End Sub

		Private Sub OutputDataCome(ByVal arrayOutput As Object())
			Dim strOut As String = "Output data 0 = "
			strOut &= arrayOutput(0).ToString()
			strOut &= ", output data 1 = "
			strOut &= arrayOutput(1).ToString()
			MessageBox.Show(strOut)
		End Sub

		Private Sub btnTestPSQL_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnTestPSQL.Click
			Dim strCreateProcedure As String = "create Procedure OrderInfoEx @dtOrderDate datetime, @strCustomerID nchar(5), @strRegion nvarchar(15), @nSumEmployeeID int out, @strInfo nchar(255) out " & "as " & "select * from Orders where ShipRegion <> @strRegion and OrderDate <> @dtOrderDate and CustomerID<>@strCustomerID and EmployeeID<@nSumEmployeeID " & "select @nSumEmployeeID=sum(EmployeeID) from Orders " & "select @strInfo='This is a test from a procedure ' + @strCustomerID"

			btnTestPSQL.Enabled = False
			m_AsynDBLite.DBErrors.Clear()

            m_AsynDBLite.GetAttachedClientSocket().BeginBatching()
            m_AsynDBLite.ExecuteNonQuery("Drop Procedure OrderInfoEx")
            m_AsynDBLite.ExecuteNonQuery(strCreateProcedure)

            Dim lstParamInfo As List(Of CParamInfo) = New List(Of CParamInfo)()
            lstParamInfo.Add(New CParamInfo())
            lstParamInfo.Add(New CParamInfo())
            lstParamInfo.Add(New CParamInfo())
            lstParamInfo.Add(New CParamInfo())
            lstParamInfo.Add(New CParamInfo())

            lstParamInfo(0).m_sDBType = UDBLib.tagSockDataType.sdVT_DATE

            lstParamInfo(1).m_sDBType = UDBLib.tagSockDataType.sdVT_WSTR
            lstParamInfo(1).m_nLen = (5 + 1) * 2 'in bytes !!!!

            lstParamInfo(2).m_sDBType = UDBLib.tagSockDataType.sdVT_WSTR
            lstParamInfo(2).m_nLen = (15 + 1) * 2


            lstParamInfo(3).m_sDBType = UDBLib.tagSockDataType.sdVT_I4
            lstParamInfo(3).m_nParamIO = UDBLib.tagSockDBParamType.sdParamInputOutput


            lstParamInfo(4).m_sDBType = UDBLib.tagSockDataType.sdVT_WSTR
            lstParamInfo(4).m_nLen = (1024 + 1) * 2
            lstParamInfo(4).m_nParamIO = UDBLib.tagSockDBParamType.sdParamOutput

            m_AsynDBLite.GetAttachedClientSocket().BeginBatching()
            m_AsynDBLite.OpenCommandWithParameters("{CALL OrderInfoEx(?, ?, ?, ?, ?)}", lstParamInfo)

            Dim aData As Object() = New Object(4) {}
            aData(0) = DateTime.Now
            aData(1) = "YZYZ"
            aData(2) = "RG"
            aData(3) = CInt(Fix(3))
            aData(4) = Nothing

            m_AsynDBLite.OpenRowsetFromParameters(aData, "OrderInfo", UDBLib.tagCursorType.ctForwardOnly, CAsynDBLite.AsynFetch, 0, -1)

            m_AsynDBLite.GetAttachedClientSocket().Commit(True)
            m_AsynDBLite.GetAttachedClientSocket().WaitAll()
			btnTestPSQL.Enabled = True
		End Sub

		Private Sub btnConnect_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnConnect.Click
			m_ClientSocket.Connect("localhost", 17001)
		End Sub

		Private Sub frmPSQL_Load(ByVal sender As Object, ByVal e As EventArgs) Handles MyBase.Load
            m_AsynDBLite.Attach(m_ClientSocket)

            m_ClientSocket.m_OnSocketClosed = m_ClientSocket.m_OnSocketClosed.Combine(m_ClientSocket.m_OnSocketClosed, New DOnSocketClosed(AddressOf OnSocketClosed))
            m_ClientSocket.m_OnSocketConnected = m_ClientSocket.m_OnSocketConnected.Combine(m_ClientSocket.m_OnSocketConnected, New DOnSocketConnected(AddressOf OnSocketConnected))
            m_AsynDBLite.m_OnOutputDataCome = New DOutputDataCome(AddressOf OutputDataCome)
			m_AsynDBLite.AttachedDataGridView = dgvRowset
		End Sub

		Private Sub btnDisconnect_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnDisconnect.Click
			m_ClientSocket.Disconnect()
		End Sub
	End Class
End Namespace