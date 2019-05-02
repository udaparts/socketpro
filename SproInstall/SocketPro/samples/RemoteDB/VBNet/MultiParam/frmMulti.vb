Imports Microsoft.VisualBasic
Imports System
Imports System.Collections.Generic
Imports System.ComponentModel
Imports System.Data
Imports System.Drawing
Imports System.Text
Imports System.Windows.Forms
Imports UDBLib
Imports USOCKETLib
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports SocketProAdapter.ClientSide.RemoteDB

Namespace MultiParam
	Public Partial Class frmMulti
		Inherits Form
		Private m_AsyDBLite As CAsynDBLite = New CAsynDBLite()
		Private m_ClientSocket As CClientSocket = New CClientSocket()

		Public Sub New()
			InitializeComponent()
		End Sub

		Private Sub OnSocketConnected(ByVal hSocket As Integer, ByVal nError As Integer)
			If nError = 0 Then
				m_ClientSocket.SetUID("SocketPro")
				m_ClientSocket.SetPassword("PassOne")
				m_ClientSocket.SwitchTo(m_AsyDBLite)
				m_AsyDBLite.ConnectDB("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=nwind3.mdb")
				m_ClientSocket.WaitAll()
				btnInsert.Enabled = m_AsyDBLite.DBConnected
			End If
		End Sub

		Private Sub OnSocketClosed(ByVal hSocket As Integer, ByVal nError As Integer)
			btnInsert.Enabled = False
		End Sub

		Private Sub frmMulti_Load(ByVal sender As Object, ByVal e As EventArgs) Handles MyBase.Load
			m_AsyDBLite.Attach(m_ClientSocket)
            m_ClientSocket.m_OnSocketConnected = m_ClientSocket.m_OnSocketConnected.Combine(m_ClientSocket.m_OnSocketConnected, New DOnSocketConnected(AddressOf OnSocketConnected))
            m_ClientSocket.m_OnSocketClosed = m_ClientSocket.m_OnSocketClosed.Combine(m_ClientSocket.m_OnSocketClosed, New DOnSocketClosed(AddressOf OnSocketClosed))
		End Sub

		Private Sub btnConnect_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnConnect.Click
			m_ClientSocket.Connect("127.0.0.1", 17001)
		End Sub

		Private Sub btnDisconnect_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnDisconnect.Click
			m_ClientSocket.Disconnect()
		End Sub

		Private Sub btnInsert_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnInsert.Click
			Dim lstParamInfo As List(Of CParamInfo) = New List(Of CParamInfo)()
			lstParamInfo.Add(New CParamInfo())
			lstParamInfo.Add(New CParamInfo())

            lstParamInfo(0).m_sDBType = UDBLib.tagSockDataType.sdVT_WSTR
			lstParamInfo(0).m_nLen = 255 * 2 'in bytes

            lstParamInfo(1).m_sDBType = UDBLib.tagSockDataType.sdVT_WSTR
			lstParamInfo(1).m_nLen = 255 * 2 'in bytes

			m_AsyDBLite.DBErrors.Clear()

			'ShipperID ignored because it is an auto-number.
			m_AsyDBLite.OpenCommandWithParameters("Insert into Shippers (CompanyName, Phone) Values (?, ?)", lstParamInfo)

			Dim aData As Object() = New Object(7){}
			aData(0) = "Oracle"
			aData(1) = "(111) 111-1111"

			aData(2) = "Microsoft"
			aData(3) = "(222) 121-1221"

			aData(4) = "Google"
			aData(5) = "(333) 111-1111"

			aData(6) = "Yahoo!"
			aData(7) = "(444) 444-4444"

			'4 sets of parameter data in one batch
			m_AsyDBLite.DoBatch(aData)

            m_AsyDBLite.GetAttachedClientSocket().WaitAll()

			If m_AsyDBLite.DBErrors.Count <> 0 Then
				MessageBox.Show(m_AsyDBLite.DBErrors(0).m_strErrorMsg)
			End If
		End Sub
	End Class
End Namespace