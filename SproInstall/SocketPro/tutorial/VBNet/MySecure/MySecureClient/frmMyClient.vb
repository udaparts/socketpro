Imports System.ComponentModel
Imports System.Text
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports USOCKETLib

Namespace MySecureClient
	Partial Public Class frmMySecure
		Inherits Form
		Private m_ClientSocket As New CClientSocket()
        Private m_AsynHandler As New CMySecure()

		Public Sub New()
			InitializeComponent()
		End Sub

		Private Sub btnConnect_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnConnect.Click
			m_ClientSocket.Connect(txtAddress.Text, Integer.Parse(txtPort.Text))
		End Sub

		Private Sub btnDisconnect_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnDisconnect.Click
			m_ClientSocket.Disconnect()
		End Sub

		Private Overloads Sub OnClosed(ByVal nSocketHandle As Integer, ByVal nError As Integer)
			btnToDB.Enabled = False
			btnExecuteSQL.Enabled = False
		End Sub

		Private Sub OnConnected(ByVal nSocketHandle As Integer, ByVal nError As Integer)
			If nError = 0 Then
				btnToDB.Enabled = True

                m_ClientSocket.EncryptionMethod = USOCKETLib.tagEncryptionMethod.BlowFish

				'set user id and password
				m_ClientSocket.SetUID(txtUserID.Text)
				m_ClientSocket.SetPassword(txtPassword.Text)

				'switch for the service identified by sidSOneSvs
				m_ClientSocket.SwitchTo(m_AsynHandler)
			Else
				MessageBox.Show(m_ClientSocket.GetErrorMsg())
			End If
		End Sub

		Private Sub frmMySecure_Load(ByVal sender As Object, ByVal e As EventArgs) Handles MyBase.Load
            m_ClientSocket.m_OnSocketClosed = AddressOf OnClosed
            m_ClientSocket.m_OnSocketConnected = AddressOf OnConnected
			m_AsynHandler.Attach(m_ClientSocket)
		End Sub

		Private Sub btnToDB_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnToDB.Click
			Dim usc As USocketClass = m_ClientSocket.GetUSocket()

			'we like to encrypt data before sending the request to server
			usc.EncryptionMethod = CShort(Fix(USOCKETLib.tagEncryptionMethod.BlowFish))

			If m_AsynHandler.Open(txtUserID.Text, txtPassword.Text) Is Nothing Then
				MessageBox.Show(m_AsynHandler.m_strErrorMessage)
			Else
				btnExecuteSQL.Enabled = True
			End If
		End Sub

		Private Sub btnExecuteSQL_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnExecuteSQL.Click
			Dim usc As USocketClass = m_ClientSocket.GetUSocket()

			'we do not want to encrypt data for the below requests
			usc.EncryptionMethod = CShort(Fix(USOCKETLib.tagEncryptionMethod.NoEncryption))
			m_ClientSocket.BeginBatching()
			m_AsynHandler.BeginTransAsyn()
			m_AsynHandler.ExecuteNoQueryAsyn(txtSQL.Text)
			m_AsynHandler.CommitAsyn(True)
			m_ClientSocket.Commit(True)
			m_ClientSocket.WaitAll()
		End Sub
	End Class
End Namespace