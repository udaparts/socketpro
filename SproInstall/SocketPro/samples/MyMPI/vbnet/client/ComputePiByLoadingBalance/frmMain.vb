Imports System.ComponentModel
Imports System.Text
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports USOCKETLib

Namespace PiLBClient
	Partial Public Class frmMain
		Inherits Form
		Private m_requests() As Short
		Private m_dPi As Double
		Private m_nPercent As Integer
		Private m_cs As CClientSocket
		Private m_AsyncPi As CPPi
		Private Const m_nDivision As Integer = 100
		Private Const m_nNum As Integer = 10000000

		Public Sub New()
			InitializeComponent()
			m_cs = New CClientSocket()
			m_AsyncPi = New CPPi()
			m_AsyncPi.Attach(m_cs)
            m_cs.m_OnRequestProcessed = AddressOf OnRequestProcessed
            m_cs.m_OnSocketClosed = AddressOf OnSocketClosed
            m_cs.m_OnSocketConnected = AddressOf OnSocketConnected

		End Sub

		Private Sub OnSocketConnected(ByVal hSocket As Integer, ByVal nError As Integer)
			If nError = 0 Then
				btnStart.Enabled = True
				m_cs.SetUID("SocketPro")
				m_cs.SetPassword("PassOne")
				m_cs.SwitchTo(m_AsyncPi)
			End If
		End Sub

		Private Sub OnSocketClosed(ByVal hSocket As Integer, ByVal nError As Integer)
			Dim strErrorMsg As String = m_cs.GetErrorMsg()
			btnStart.Enabled = False
		End Sub

		Private Sub OnRequestProcessed(ByVal hSocket As Integer, ByVal sRequestID As Short, ByVal nLen As Integer, ByVal nLenInBuffer As Integer, ByVal ReturnFlag As tagReturnFlag)
			If sRequestID = piConst.idComputeCPPi Then
				m_dPi += m_AsyncPi.GetCompute()
				m_nPercent += 1
				txtPi.Text = "Pi = " & m_dPi & " with " & m_nPercent & "%"
				Dim obj As Object = m_cs.GetUSocket().GetRequestsInQueue()
				m_requests = CType(obj, Short())
			End If
		End Sub

		Private Sub btnConnect_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnConnect.Click
			m_cs.Connect(txtHost.Text, Integer.Parse(txtPort.Text))
		End Sub

		Private Sub btnClose_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnClose.Click
			m_cs.Shutdown()
		End Sub

		Private Sub btnStart_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnStart.Click
			Dim n As Integer
			m_dPi = 0.0
			m_nPercent = 0

            Dim dStep As Double = 1.0 / m_nNum / m_nDivision
			m_cs.BeginBatching()
			For n = 0 To m_nDivision - 1
				Dim dStart As Double = CDbl(n) / m_nDivision
                m_AsyncPi.ComputeAsync(dStart, dStep, m_nNum)
			Next n
			m_cs.Commit()
		End Sub

		Private Sub btnCancel_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnCancel.Click
			m_cs.Cancel()
		End Sub
	End Class
End Namespace