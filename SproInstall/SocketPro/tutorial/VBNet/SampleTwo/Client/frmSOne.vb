Imports SocketProAdapter
Imports SocketProAdapter.ClientSide

Public Class frmSOne
    Inherits System.Windows.Forms.Form
    Implements SocketProAdapter.ClientSide.IAsyncResultsHandler

#Region " Windows Form Designer generated code "

    Public Sub New()
        MyBase.New()

        'This call is required by the Windows Form Designer.
        InitializeComponent()

        'Add any initialization after the InitializeComponent() call

    End Sub

    'Form overrides dispose to clean up the component list.
    Protected Overloads Overrides Sub Dispose(ByVal disposing As Boolean)
        If disposing Then
            If Not (components Is Nothing) Then
                components.Dispose()
            End If
        End If
        MyBase.Dispose(disposing)
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    Friend WithEvents txtPassword As System.Windows.Forms.TextBox
    Friend WithEvents txtUserID As System.Windows.Forms.TextBox
    Friend WithEvents txtQueryGlobalFastCount As System.Windows.Forms.TextBox
    Friend WithEvents txtQueryGlobalCount As System.Windows.Forms.TextBox
    Friend WithEvents txtCount As System.Windows.Forms.TextBox
    Friend WithEvents txtSleep As System.Windows.Forms.TextBox
    Friend WithEvents txtPort As System.Windows.Forms.TextBox
    Friend WithEvents txtHost As System.Windows.Forms.TextBox
    Friend WithEvents label5 As System.Windows.Forms.Label
    Friend WithEvents label2 As System.Windows.Forms.Label
    Friend WithEvents btnGetAllCounts As System.Windows.Forms.Button
    Friend WithEvents btnEchoData As System.Windows.Forms.Button
    Friend WithEvents btnQueryGlobalFastCount As System.Windows.Forms.Button
    Friend WithEvents btnQueryGlobalCount As System.Windows.Forms.Button
    Friend WithEvents btnQueryCount As System.Windows.Forms.Button
    Friend WithEvents chkFrozen As System.Windows.Forms.CheckBox
    Friend WithEvents btnSleep As System.Windows.Forms.Button
    Friend WithEvents chkZip As System.Windows.Forms.CheckBox
    Friend WithEvents btnDisconnect As System.Windows.Forms.Button
    Friend WithEvents btnConnect As System.Windows.Forms.Button
    Friend WithEvents label3 As System.Windows.Forms.Label
    Friend WithEvents label1 As System.Windows.Forms.Label
    Friend WithEvents txtMsg As System.Windows.Forms.TextBox
    Friend WithEvents label4 As System.Windows.Forms.Label
    Friend WithEvents chkUseSSL As System.Windows.Forms.CheckBox
    <System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
        Me.txtPassword = New System.Windows.Forms.TextBox
        Me.txtUserID = New System.Windows.Forms.TextBox
        Me.txtQueryGlobalFastCount = New System.Windows.Forms.TextBox
        Me.txtQueryGlobalCount = New System.Windows.Forms.TextBox
        Me.txtCount = New System.Windows.Forms.TextBox
        Me.txtSleep = New System.Windows.Forms.TextBox
        Me.txtPort = New System.Windows.Forms.TextBox
        Me.txtHost = New System.Windows.Forms.TextBox
        Me.label5 = New System.Windows.Forms.Label
        Me.label2 = New System.Windows.Forms.Label
        Me.btnGetAllCounts = New System.Windows.Forms.Button
        Me.btnEchoData = New System.Windows.Forms.Button
        Me.btnQueryGlobalFastCount = New System.Windows.Forms.Button
        Me.btnQueryGlobalCount = New System.Windows.Forms.Button
        Me.btnQueryCount = New System.Windows.Forms.Button
        Me.chkFrozen = New System.Windows.Forms.CheckBox
        Me.btnSleep = New System.Windows.Forms.Button
        Me.chkZip = New System.Windows.Forms.CheckBox
        Me.btnDisconnect = New System.Windows.Forms.Button
        Me.btnConnect = New System.Windows.Forms.Button
        Me.label3 = New System.Windows.Forms.Label
        Me.label1 = New System.Windows.Forms.Label
        Me.txtMsg = New System.Windows.Forms.TextBox
        Me.label4 = New System.Windows.Forms.Label
        Me.chkUseSSL = New System.Windows.Forms.CheckBox
        Me.SuspendLayout()
        '
        'txtPassword
        '
        Me.txtPassword.Location = New System.Drawing.Point(272, 32)
        Me.txtPassword.Name = "txtPassword"
        Me.txtPassword.PasswordChar = Global.Microsoft.VisualBasic.ChrW(42)
        Me.txtPassword.Size = New System.Drawing.Size(64, 20)
        Me.txtPassword.TabIndex = 59
        Me.txtPassword.Text = "PassOne"
        '
        'txtUserID
        '
        Me.txtUserID.Location = New System.Drawing.Point(272, 8)
        Me.txtUserID.Name = "txtUserID"
        Me.txtUserID.Size = New System.Drawing.Size(64, 20)
        Me.txtUserID.TabIndex = 58
        Me.txtUserID.Text = "SocketPro"
        '
        'txtQueryGlobalFastCount
        '
        Me.txtQueryGlobalFastCount.Location = New System.Drawing.Point(240, 240)
        Me.txtQueryGlobalFastCount.Name = "txtQueryGlobalFastCount"
        Me.txtQueryGlobalFastCount.Size = New System.Drawing.Size(64, 20)
        Me.txtQueryGlobalFastCount.TabIndex = 53
        Me.txtQueryGlobalFastCount.Text = "0"
        '
        'txtQueryGlobalCount
        '
        Me.txtQueryGlobalCount.Location = New System.Drawing.Point(184, 200)
        Me.txtQueryGlobalCount.Name = "txtQueryGlobalCount"
        Me.txtQueryGlobalCount.Size = New System.Drawing.Size(64, 20)
        Me.txtQueryGlobalCount.TabIndex = 51
        Me.txtQueryGlobalCount.Text = "0"
        '
        'txtCount
        '
        Me.txtCount.Location = New System.Drawing.Point(152, 160)
        Me.txtCount.Name = "txtCount"
        Me.txtCount.Size = New System.Drawing.Size(64, 20)
        Me.txtCount.TabIndex = 49
        Me.txtCount.Text = "0"
        '
        'txtSleep
        '
        Me.txtSleep.Location = New System.Drawing.Point(96, 80)
        Me.txtSleep.Name = "txtSleep"
        Me.txtSleep.Size = New System.Drawing.Size(48, 20)
        Me.txtSleep.TabIndex = 45
        Me.txtSleep.Text = "5000"
        '
        'txtPort
        '
        Me.txtPort.Location = New System.Drawing.Point(144, 32)
        Me.txtPort.Name = "txtPort"
        Me.txtPort.Size = New System.Drawing.Size(48, 20)
        Me.txtPort.TabIndex = 44
        Me.txtPort.Text = "20901"
        '
        'txtHost
        '
        Me.txtHost.Location = New System.Drawing.Point(16, 32)
        Me.txtHost.Name = "txtHost"
        Me.txtHost.Size = New System.Drawing.Size(120, 20)
        Me.txtHost.TabIndex = 43
        Me.txtHost.Text = "localhost"
        '
        'label5
        '
        Me.label5.Location = New System.Drawing.Point(200, 32)
        Me.label5.Name = "label5"
        Me.label5.Size = New System.Drawing.Size(64, 24)
        Me.label5.TabIndex = 57
        Me.label5.Text = "Password:"
        Me.label5.TextAlign = System.Drawing.ContentAlignment.MiddleRight
        '
        'label2
        '
        Me.label2.Location = New System.Drawing.Point(216, 8)
        Me.label2.Name = "label2"
        Me.label2.Size = New System.Drawing.Size(48, 16)
        Me.label2.TabIndex = 56
        Me.label2.Text = "User ID:"
        Me.label2.TextAlign = System.Drawing.ContentAlignment.MiddleRight
        '
        'btnGetAllCounts
        '
        Me.btnGetAllCounts.Enabled = False
        Me.btnGetAllCounts.Location = New System.Drawing.Point(16, 112)
        Me.btnGetAllCounts.Name = "btnGetAllCounts"
        Me.btnGetAllCounts.Size = New System.Drawing.Size(120, 32)
        Me.btnGetAllCounts.TabIndex = 55
        Me.btnGetAllCounts.Text = "Get All Counts"
        '
        'btnEchoData
        '
        Me.btnEchoData.Enabled = False
        Me.btnEchoData.Location = New System.Drawing.Point(16, 272)
        Me.btnEchoData.Name = "btnEchoData"
        Me.btnEchoData.Size = New System.Drawing.Size(112, 32)
        Me.btnEchoData.TabIndex = 54
        Me.btnEchoData.Text = "Echo Data"
        '
        'btnQueryGlobalFastCount
        '
        Me.btnQueryGlobalFastCount.Enabled = False
        Me.btnQueryGlobalFastCount.Location = New System.Drawing.Point(16, 232)
        Me.btnQueryGlobalFastCount.Name = "btnQueryGlobalFastCount"
        Me.btnQueryGlobalFastCount.Size = New System.Drawing.Size(216, 32)
        Me.btnQueryGlobalFastCount.TabIndex = 52
        Me.btnQueryGlobalFastCount.Text = "QueryGlobalFastCount"
        '
        'btnQueryGlobalCount
        '
        Me.btnQueryGlobalCount.Enabled = False
        Me.btnQueryGlobalCount.Location = New System.Drawing.Point(16, 192)
        Me.btnQueryGlobalCount.Name = "btnQueryGlobalCount"
        Me.btnQueryGlobalCount.Size = New System.Drawing.Size(160, 32)
        Me.btnQueryGlobalCount.TabIndex = 50
        Me.btnQueryGlobalCount.Text = "QueryGlobalCount"
        '
        'btnQueryCount
        '
        Me.btnQueryCount.Enabled = False
        Me.btnQueryCount.Location = New System.Drawing.Point(16, 152)
        Me.btnQueryCount.Name = "btnQueryCount"
        Me.btnQueryCount.Size = New System.Drawing.Size(128, 32)
        Me.btnQueryCount.TabIndex = 48
        Me.btnQueryCount.Text = "QueryCount"
        '
        'chkFrozen
        '
        Me.chkFrozen.Location = New System.Drawing.Point(160, 80)
        Me.chkFrozen.Name = "chkFrozen"
        Me.chkFrozen.Size = New System.Drawing.Size(104, 24)
        Me.chkFrozen.TabIndex = 47
        Me.chkFrozen.Text = "GUI Frozen"
        '
        'btnSleep
        '
        Me.btnSleep.Enabled = False
        Me.btnSleep.Location = New System.Drawing.Point(16, 72)
        Me.btnSleep.Name = "btnSleep"
        Me.btnSleep.Size = New System.Drawing.Size(72, 32)
        Me.btnSleep.TabIndex = 46
        Me.btnSleep.Text = "Sleep"
        '
        'chkZip
        '
        Me.chkZip.Location = New System.Drawing.Point(344, 40)
        Me.chkZip.Name = "chkZip"
        Me.chkZip.Size = New System.Drawing.Size(72, 20)
        Me.chkZip.TabIndex = 42
        Me.chkZip.Text = "Zip ?"
        '
        'btnDisconnect
        '
        Me.btnDisconnect.Location = New System.Drawing.Point(344, 64)
        Me.btnDisconnect.Name = "btnDisconnect"
        Me.btnDisconnect.Size = New System.Drawing.Size(168, 24)
        Me.btnDisconnect.TabIndex = 41
        Me.btnDisconnect.Text = "Disconnect"
        '
        'btnConnect
        '
        Me.btnConnect.Location = New System.Drawing.Point(344, 8)
        Me.btnConnect.Name = "btnConnect"
        Me.btnConnect.Size = New System.Drawing.Size(168, 24)
        Me.btnConnect.TabIndex = 40
        Me.btnConnect.Text = "Connect"
        '
        'label3
        '
        Me.label3.Location = New System.Drawing.Point(144, 8)
        Me.label3.Name = "label3"
        Me.label3.Size = New System.Drawing.Size(40, 16)
        Me.label3.TabIndex = 39
        Me.label3.Text = "Port:"
        '
        'label1
        '
        Me.label1.Location = New System.Drawing.Point(16, 8)
        Me.label1.Name = "label1"
        Me.label1.Size = New System.Drawing.Size(132, 16)
        Me.label1.TabIndex = 38
        Me.label1.Text = "Host Address:"
        '
        'txtMsg
        '
        Me.txtMsg.Location = New System.Drawing.Point(144, 128)
        Me.txtMsg.Name = "txtMsg"
        Me.txtMsg.Size = New System.Drawing.Size(368, 20)
        Me.txtMsg.TabIndex = 62
        Me.txtMsg.TextAlign = System.Windows.Forms.HorizontalAlignment.Center
        '
        'label4
        '
        Me.label4.Location = New System.Drawing.Point(152, 112)
        Me.label4.Name = "label4"
        Me.label4.Size = New System.Drawing.Size(360, 8)
        Me.label4.TabIndex = 61
        Me.label4.Text = "Notification Message:"
        Me.label4.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'chkUseSSL
        '
        Me.chkUseSSL.Checked = True
        Me.chkUseSSL.CheckState = System.Windows.Forms.CheckState.Checked
        Me.chkUseSSL.Location = New System.Drawing.Point(424, 40)
        Me.chkUseSSL.Name = "chkUseSSL"
        Me.chkUseSSL.Size = New System.Drawing.Size(72, 16)
        Me.chkUseSSL.TabIndex = 60
        Me.chkUseSSL.Text = "Use SSL"
        '
        'frmSOne
        '
        Me.AutoScaleBaseSize = New System.Drawing.Size(5, 13)
        Me.ClientSize = New System.Drawing.Size(528, 333)
        Me.Controls.Add(Me.txtMsg)
        Me.Controls.Add(Me.label4)
        Me.Controls.Add(Me.chkUseSSL)
        Me.Controls.Add(Me.txtPassword)
        Me.Controls.Add(Me.txtUserID)
        Me.Controls.Add(Me.txtQueryGlobalFastCount)
        Me.Controls.Add(Me.txtQueryGlobalCount)
        Me.Controls.Add(Me.txtCount)
        Me.Controls.Add(Me.txtSleep)
        Me.Controls.Add(Me.txtPort)
        Me.Controls.Add(Me.txtHost)
        Me.Controls.Add(Me.label5)
        Me.Controls.Add(Me.label2)
        Me.Controls.Add(Me.btnGetAllCounts)
        Me.Controls.Add(Me.btnEchoData)
        Me.Controls.Add(Me.btnQueryGlobalFastCount)
        Me.Controls.Add(Me.btnQueryGlobalCount)
        Me.Controls.Add(Me.btnQueryCount)
        Me.Controls.Add(Me.chkFrozen)
        Me.Controls.Add(Me.btnSleep)
        Me.Controls.Add(Me.chkZip)
        Me.Controls.Add(Me.btnDisconnect)
        Me.Controls.Add(Me.btnConnect)
        Me.Controls.Add(Me.label3)
        Me.Controls.Add(Me.label1)
        Me.Name = "frmSOne"
        Me.Text = "SOne Client"
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub

#End Region

    Private Sub OnSocketClosed(ByVal hSocket As Integer, ByVal nError As Integer)
        btnEchoData.Enabled = False
        btnGetAllCounts.Enabled = False
        btnQueryCount.Enabled = False
        btnQueryGlobalCount.Enabled = False
        btnQueryGlobalFastCount.Enabled = False
        btnSleep.Enabled = False
        If nError <> 0 Then
            MsgBox(m_MySocket.GetErrorMsg())
        End If
    End Sub

    Private Sub OnBaseRequestProcessed(ByVal sRequestID As Short)
        Select Case (sRequestID)
            Case USOCKETLib.tagChatRequestID.idEnter, USOCKETLib.tagChatRequestID.idXEnter
                Dim strMsg As String = Nothing
                Dim nGroup As Integer = 0
                Dim nPort As Integer = 0
                Dim nSvsID As Integer = 0
                Dim strUID As String = Nothing
                Dim strIPAddr As String
                strIPAddr = m_MySocket.GetUSocket().GetInfo(0, nGroup, strUID, nSvsID, nPort)
                strMsg = strUID
                strMsg = strMsg & "@"
                strMsg = strMsg & strIPAddr
                strMsg = strMsg & ":"
                strMsg = strMsg & nPort
                strMsg = strMsg & " has just joined the group"
                txtMsg.Text = strMsg
            Case USOCKETLib.tagChatRequestID.idExit
                Dim strMsg As String
                Dim nGroup As Integer
                Dim nPort As Integer
                Dim nSvsID As Integer
                Dim strUID As String = Nothing
                Dim strIPAddr As String = m_MySocket.GetUSocket().GetInfo(0, nGroup, strUID, nSvsID, nPort)
                strMsg = strUID
                strMsg = strMsg & "@"
                strMsg = strMsg & strIPAddr
                strMsg = strMsg & ":"
                strMsg = strMsg & nPort
                strMsg = strMsg & " has just exited from the group"
                txtMsg.Text = strMsg
            Case USOCKETLib.tagChatRequestID.idSpeak, USOCKETLib.tagChatRequestID.idXSpeak, USOCKETLib.tagChatRequestID.idXSpeakEx, USOCKETLib.tagChatRequestID.idSendUserMessage
                Dim nGroup As Integer
                Dim nPort As Integer
                Dim nSvsID As Integer = 0
                Dim strUID As String = Nothing
                Dim strMsg As String = m_MySocket.GetUSocket().Message
                Dim strIPAddr As String = m_MySocket.GetUSocket().GetInfo(0, nGroup, strUID, nSvsID, nPort)
                strMsg = strMsg & " from "
                strMsg = strMsg & strUID
                strMsg = strMsg & "@"
                strMsg = strMsg & strIPAddr
                strMsg = strMsg & ":"
                strMsg = strMsg & nPort
                txtMsg.Text = strMsg
            Case Else
        End Select
    End Sub

    Private Sub OnSocketConnected(ByVal hSocket As Integer, ByVal nError As Integer)
        If nError = 0 Then
            Dim em As Short = m_MySocket.GetUSocket().EncryptionMethod
            If (em <> USOCKETLib.tagEncryptionMethod.NoEncryption And em <> USOCKETLib.tagEncryptionMethod.BlowFish) Then
                Dim nErrorCode As Integer
                Dim UCert As USOCKETLib.IUCert = m_MySocket.GetUSocket().PeerCertificate

                Dim str As String = UCert.Subject.ToLower()

                'check certificate subject here

                'verify certificate chain
                str = UCert.Verify(nErrorCode)
                str = Nothing

                'authenticate a remote server before sending password by verifing a certificate
            End If

            m_MySocket.SetUID(txtUserID.Text)
            m_MySocket.SetPassword(txtPassword.Text)

            m_MySocket.BeginBatching()
            m_MySocket.SwitchTo(m_MySvsHandler, True)
            m_MySocket.GetUSocket().TurnOnZipAtSvr(chkZip.Checked)
            m_MySocket.Commit(False) 'must be false for the very first switch

            btnEchoData.Enabled = True
            btnGetAllCounts.Enabled = True
            btnQueryCount.Enabled = True
            btnQueryGlobalCount.Enabled = True
            btnQueryGlobalFastCount.Enabled = True
            btnSleep.Enabled = True
        Else
            MsgBox(m_MySocket.GetErrorMsg())
        End If
    End Sub

    Private Sub btnConnect_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnConnect.Click
        If chkUseSSL.Checked Then
            m_MySocket.GetUSocket().EncryptionMethod = USOCKETLib.tagEncryptionMethod.MSTLSv1
        Else
            m_MySocket.GetUSocket().EncryptionMethod = USOCKETLib.tagEncryptionMethod.NoEncryption
        End If
        m_MySocket.Connect(txtHost.Text, txtPort.Text)
    End Sub

    Private Sub btnDisconnect_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnDisconnect.Click
        m_MySocket.Disconnect()
    End Sub

    Private Sub chkZip_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles chkZip.CheckedChanged
        m_MySocket.GetUSocket().ZipIsOn = chkZip.Checked
        If m_MySocket.IsConnected() Then
            m_MySocket.GetUSocket().TurnOnZipAtSvr(chkZip.Checked)
        End If
    End Sub

    Private Sub frmSOne_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load
        m_MySocket = New CClientSocket()

        'attach to m_MySocket, and use IAsyncResultsHandler for processing returning results
        m_MySvsHandler = New CAsyncServiceHandler(TOneConst.sidCTOne, m_MySocket, Me)

        m_MySocket.m_OnSocketConnected = New DOnSocketConnected(AddressOf OnSocketConnected)
        m_MySocket.m_OnSocketClosed = New DOnSocketClosed(AddressOf OnSocketClosed)
        m_MySocket.m_OnBaseRequestProcessed = New DOnBaseRequestProcessed(AddressOf OnBaseRequestProcessed)
        m_MySocket.GetUSocket().EncryptionMethod = USOCKETLib.tagEncryptionMethod.TLSv1
    End Sub

    Private m_MySocket As CClientSocket
    Private m_MySvsHandler As CAsyncServiceHandler


    Private Sub chkFrozen_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles chkFrozen.CheckedChanged
        m_MySocket.DisableUI(chkFrozen.Checked)
    End Sub

    Private Sub btnSleep_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnSleep.Click
        btnSleep.Enabled = False

        Dim nSleep As Integer = txtSleep.Text
        'use IAsyncResultsHandler for processing return results
        Me.m_MySvsHandler.SendRequest(TOneConst.idSleepCTOne, nSleep)
    End Sub

    Private Sub btnGetAllCounts_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnGetAllCounts.Click
        Me.m_MySocket.BeginBatching()
        'use IAsyncResultsHandler for processing return results
        Me.m_MySvsHandler.SendRequest(TOneConst.idQueryCountCTOne)
        Me.m_MySvsHandler.SendRequest(TOneConst.idQueryGlobalCountCTOne)
        Me.m_MySvsHandler.SendRequest(TOneConst.idQueryGlobalFastCountCTOne)
        Me.m_MySocket.Commit(True) 'Requests batch, and results batch
    End Sub

    Private Sub btnQueryCount_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnQueryCount.Click
        Me.m_MySvsHandler.SendRequest(TOneConst.idQueryCountCTOne)
    End Sub

    Private Sub btnQueryGlobalCount_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnQueryGlobalCount.Click
        Me.m_MySvsHandler.SendRequest(TOneConst.idQueryGlobalCountCTOne)
    End Sub

    Private Sub btnQueryGlobalFastCount_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnQueryGlobalFastCount.Click
        Me.m_MySvsHandler.SendRequest(TOneConst.idQueryGlobalFastCountCTOne)
    End Sub

    Private Sub btnEchoData_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnEchoData.Click
        Dim nTry As Integer = 12345678
        Dim objA(2) As Object
        Dim objChildren(3) As Object
        Dim objOut As Object = Nothing
        objA(0) = 234321
        objA(1) = "This is a simple try string"
        objChildren(0) = Now()
        objChildren(1) = "Here is a sub string"
        objChildren(2) = 45678900000000
        objChildren(3) = Nothing
        objA(2) = objChildren

        Dim obj As Object

        Dim nGroups() As Integer = {1, 2, 3, 4, 5}

        obj = objA

        Me.m_MySocket.BeginBatching()
        'use IAsyncResultsHandler for processing return results
        m_MySvsHandler.SendRequest(TOneConst.idEchoCTOne, obj)
        Me.m_MySocket.Push.Broadcast("Test message from the method Echo ", nGroups)

        Me.m_MySocket.Commit(True)
    End Sub

#Region "IAsyncResultsHandler Members"

    Public Sub OnExceptionFromServer(ByVal AsyncServiceHandler As CAsyncServiceHandler, ByVal Exception As CSocketProServerException) Implements IAsyncResultsHandler.OnExceptionFromServer
        Select Case Exception.m_sRequestID
            Case TOneConst.idSleepCTOne
                btnSleep.Enabled = True
                Exit Select
            Case Else
                Exit Select
        End Select
        MessageBox.Show(Exception.Message)
    End Sub

    Public Sub Process(ByVal AsyncResult As CAsyncResult) Implements IAsyncResultsHandler.Process
        Dim objOut As Object = Nothing
        Dim nData As Integer = 0
        Select Case AsyncResult.RequestId
            Case TOneConst.idQueryCountCTOne
                AsyncResult.UQueue.Pop(nData)
                txtCount.Text = nData.ToString()
                Exit Select
            Case TOneConst.idQueryGlobalCountCTOne
                AsyncResult.UQueue.Pop(nData)
                txtQueryGlobalCount.Text = nData.ToString()
                Exit Select
            Case TOneConst.idQueryGlobalFastCountCTOne
                AsyncResult.UQueue.Pop(nData)
                txtQueryGlobalFastCount.Text = nData.ToString()
                Exit Select
            Case TOneConst.idEchoCTOne
                AsyncResult.UQueue.Load(objOut)
                Exit Select
            Case TOneConst.idSleepCTOne
                btnSleep.Enabled = True
                Exit Select
            Case Else
                Exit Select
        End Select
    End Sub

#End Region


End Class
