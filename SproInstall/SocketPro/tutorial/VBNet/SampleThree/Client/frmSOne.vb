Imports SocketProAdapter
Imports SampleThreeShared
Imports System.Collections
Imports USOCKETLib
Imports SocketProAdapter.ClientSide

Public Class frmSOne
    Inherits System.Windows.Forms.Form

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
    Friend WithEvents txtBytesSent As System.Windows.Forms.TextBox
    Friend WithEvents txtBytesRecv As System.Windows.Forms.TextBox
    Friend WithEvents txtSendALot As System.Windows.Forms.TextBox
    Friend WithEvents txtGetALot As System.Windows.Forms.TextBox
    Friend WithEvents txtTimeRequired As System.Windows.Forms.TextBox
    Friend WithEvents txtLatency As System.Windows.Forms.TextBox
    Friend WithEvents label9 As System.Windows.Forms.Label
    Friend WithEvents label8 As System.Windows.Forms.Label
    Friend WithEvents btnSendALotItemsToServer As System.Windows.Forms.Button
    Friend WithEvents btnSendOneItemToServer As System.Windows.Forms.Button
    Friend WithEvents btnGetALotItemsFromServer As System.Windows.Forms.Button
    Friend WithEvents btnGetOneItemFromServer As System.Windows.Forms.Button
    Friend WithEvents label7 As System.Windows.Forms.Label
    Friend WithEvents label6 As System.Windows.Forms.Label
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
        Me.txtBytesSent = New System.Windows.Forms.TextBox
        Me.txtBytesRecv = New System.Windows.Forms.TextBox
        Me.txtSendALot = New System.Windows.Forms.TextBox
        Me.txtGetALot = New System.Windows.Forms.TextBox
        Me.txtTimeRequired = New System.Windows.Forms.TextBox
        Me.txtLatency = New System.Windows.Forms.TextBox
        Me.label9 = New System.Windows.Forms.Label
        Me.label8 = New System.Windows.Forms.Label
        Me.btnSendALotItemsToServer = New System.Windows.Forms.Button
        Me.btnSendOneItemToServer = New System.Windows.Forms.Button
        Me.btnGetALotItemsFromServer = New System.Windows.Forms.Button
        Me.btnGetOneItemFromServer = New System.Windows.Forms.Button
        Me.label7 = New System.Windows.Forms.Label
        Me.label6 = New System.Windows.Forms.Label
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
        Me.chkZip.Location = New System.Drawing.Point(456, 40)
        Me.chkZip.Name = "chkZip"
        Me.chkZip.Size = New System.Drawing.Size(72, 20)
        Me.chkZip.TabIndex = 42
        Me.chkZip.Text = "Zip ?"
        '
        'btnDisconnect
        '
        Me.btnDisconnect.Location = New System.Drawing.Point(456, 64)
        Me.btnDisconnect.Name = "btnDisconnect"
        Me.btnDisconnect.Size = New System.Drawing.Size(168, 24)
        Me.btnDisconnect.TabIndex = 41
        Me.btnDisconnect.Text = "Disconnect"
        '
        'btnConnect
        '
        Me.btnConnect.Location = New System.Drawing.Point(456, 8)
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
        Me.label4.Location = New System.Drawing.Point(152, 107)
        Me.label4.Name = "label4"
        Me.label4.Size = New System.Drawing.Size(360, 18)
        Me.label4.TabIndex = 61
        Me.label4.Text = "Notification Message:"
        Me.label4.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'chkUseSSL
        '
        Me.chkUseSSL.Location = New System.Drawing.Point(536, 40)
        Me.chkUseSSL.Name = "chkUseSSL"
        Me.chkUseSSL.Size = New System.Drawing.Size(72, 16)
        Me.chkUseSSL.TabIndex = 60
        Me.chkUseSSL.Text = "Use SSL"
        '
        'txtBytesSent
        '
        Me.txtBytesSent.Location = New System.Drawing.Point(56, 336)
        Me.txtBytesSent.Name = "txtBytesSent"
        Me.txtBytesSent.Size = New System.Drawing.Size(120, 20)
        Me.txtBytesSent.TabIndex = 84
        '
        'txtBytesRecv
        '
        Me.txtBytesRecv.Location = New System.Drawing.Point(200, 336)
        Me.txtBytesRecv.Name = "txtBytesRecv"
        Me.txtBytesRecv.Size = New System.Drawing.Size(96, 20)
        Me.txtBytesRecv.TabIndex = 81
        '
        'txtSendALot
        '
        Me.txtSendALot.Location = New System.Drawing.Point(568, 304)
        Me.txtSendALot.Name = "txtSendALot"
        Me.txtSendALot.Size = New System.Drawing.Size(56, 20)
        Me.txtSendALot.TabIndex = 78
        Me.txtSendALot.Text = "50000"
        '
        'txtGetALot
        '
        Me.txtGetALot.Location = New System.Drawing.Point(568, 192)
        Me.txtGetALot.Name = "txtGetALot"
        Me.txtGetALot.Size = New System.Drawing.Size(56, 20)
        Me.txtGetALot.TabIndex = 71
        Me.txtGetALot.Text = "50000"
        '
        'txtTimeRequired
        '
        Me.txtTimeRequired.Location = New System.Drawing.Point(344, 72)
        Me.txtTimeRequired.Name = "txtTimeRequired"
        Me.txtTimeRequired.Size = New System.Drawing.Size(104, 20)
        Me.txtTimeRequired.TabIndex = 66
        '
        'txtLatency
        '
        Me.txtLatency.Location = New System.Drawing.Point(344, 24)
        Me.txtLatency.Name = "txtLatency"
        Me.txtLatency.Size = New System.Drawing.Size(104, 20)
        Me.txtLatency.TabIndex = 64
        '
        'label9
        '
        Me.label9.Location = New System.Drawing.Point(56, 312)
        Me.label9.Name = "label9"
        Me.label9.Size = New System.Drawing.Size(80, 16)
        Me.label9.TabIndex = 83
        Me.label9.Text = "Bytes sent:"
        '
        'label8
        '
        Me.label8.Location = New System.Drawing.Point(200, 312)
        Me.label8.Name = "label8"
        Me.label8.Size = New System.Drawing.Size(96, 16)
        Me.label8.TabIndex = 82
        Me.label8.Text = "Bytes received"
        '
        'btnSendALotItemsToServer
        '
        Me.btnSendALotItemsToServer.Enabled = False
        Me.btnSendALotItemsToServer.Location = New System.Drawing.Point(320, 304)
        Me.btnSendALotItemsToServer.Name = "btnSendALotItemsToServer"
        Me.btnSendALotItemsToServer.Size = New System.Drawing.Size(240, 24)
        Me.btnSendALotItemsToServer.TabIndex = 77
        Me.btnSendALotItemsToServer.Text = "SendALotItemsToServer"
        '
        'btnSendOneItemToServer
        '
        Me.btnSendOneItemToServer.Enabled = False
        Me.btnSendOneItemToServer.Location = New System.Drawing.Point(320, 274)
        Me.btnSendOneItemToServer.Name = "btnSendOneItemToServer"
        Me.btnSendOneItemToServer.Size = New System.Drawing.Size(240, 24)
        Me.btnSendOneItemToServer.TabIndex = 74
        Me.btnSendOneItemToServer.Text = "SendOneItemToServer"
        '
        'btnGetALotItemsFromServer
        '
        Me.btnGetALotItemsFromServer.Enabled = False
        Me.btnGetALotItemsFromServer.Location = New System.Drawing.Point(320, 192)
        Me.btnGetALotItemsFromServer.Name = "btnGetALotItemsFromServer"
        Me.btnGetALotItemsFromServer.Size = New System.Drawing.Size(240, 24)
        Me.btnGetALotItemsFromServer.TabIndex = 70
        Me.btnGetALotItemsFromServer.Text = "GetALotItemsFromServer"
        '
        'btnGetOneItemFromServer
        '
        Me.btnGetOneItemFromServer.Enabled = False
        Me.btnGetOneItemFromServer.Location = New System.Drawing.Point(320, 162)
        Me.btnGetOneItemFromServer.Name = "btnGetOneItemFromServer"
        Me.btnGetOneItemFromServer.Size = New System.Drawing.Size(240, 24)
        Me.btnGetOneItemFromServer.TabIndex = 67
        Me.btnGetOneItemFromServer.Text = "GetOneItemFromServer"
        '
        'label7
        '
        Me.label7.Location = New System.Drawing.Point(344, 48)
        Me.label7.Name = "label7"
        Me.label7.Size = New System.Drawing.Size(104, 16)
        Me.label7.TabIndex = 65
        Me.label7.Text = "TimeRequired (us):"
        '
        'label6
        '
        Me.label6.Location = New System.Drawing.Point(344, 8)
        Me.label6.Name = "label6"
        Me.label6.Size = New System.Drawing.Size(80, 16)
        Me.label6.TabIndex = 63
        Me.label6.Text = "Latency (us):"
        '
        'frmSOne
        '
        Me.AutoScaleBaseSize = New System.Drawing.Size(5, 13)
        Me.ClientSize = New System.Drawing.Size(640, 381)
        Me.Controls.Add(Me.txtBytesSent)
        Me.Controls.Add(Me.txtBytesRecv)
        Me.Controls.Add(Me.txtSendALot)
        Me.Controls.Add(Me.txtGetALot)
        Me.Controls.Add(Me.txtTimeRequired)
        Me.Controls.Add(Me.txtLatency)
        Me.Controls.Add(Me.label9)
        Me.Controls.Add(Me.label8)
        Me.Controls.Add(Me.btnSendALotItemsToServer)
        Me.Controls.Add(Me.btnSendOneItemToServer)
        Me.Controls.Add(Me.btnGetALotItemsFromServer)
        Me.Controls.Add(Me.btnGetOneItemFromServer)
        Me.Controls.Add(Me.label7)
        Me.Controls.Add(Me.label6)
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
        EnableButtons(False)
        If nError <> 0 Then
            MsgBox(m_MySocket.GetErrorMsg())
        End If
    End Sub


    Private Sub OnBaseRequestProcessed(ByVal sRequestID As Short)
        Select Case (sRequestID)
            Case USOCKETLib.tagBaseRequestID.idSwitchTo
                txtTimeRequired.Text = m_PerfQuery.Diff(m_lPrev)
            Case USOCKETLib.tagChatRequestID.idEnter, USOCKETLib.tagChatRequestID.idXEnter
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
            Case USOCKETLib.tagChatRequestID.idSpeak, USOCKETLib.tagChatRequestID.idXSpeak
                Dim nGroup As Integer
                Dim nPort As Integer
                Dim nSvsID As Integer
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
        txtLatency.Text = m_PerfQuery.Diff(m_lPrev)
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

            m_MySocket.GetUSocket().ZipIsOn = chkZip.Checked

            'Incerasing TCP sending and receiving buffer sizes will help performance 
            'when there is a lot of data transferred if your network bandwidth is over 10 mbps
            m_MySocket.GetUSocket().SetSockOpt(USOCKETLib.tagSocketOption.soSndBuf, 116800, USOCKETLib.tagSocketLevel.slSocket)
            m_MySocket.GetUSocket().SetSockOpt(USOCKETLib.tagSocketOption.soRcvBuf, 116800, USOCKETLib.tagSocketLevel.slSocket)

            m_MySocket.BeginBatching()

            m_MySocket.SwitchTo(m_MySvsHandler)

            'Incerasing TCP sending and receiving buffer sizes will help performance 
            'when there is a lot of data transferred if your network bandwidth is over 10 mbps
            m_MySocket.GetUSocket().SetSockOptAtSvr(USOCKETLib.tagSocketOption.soSndBuf, 116800, USOCKETLib.tagSocketLevel.slSocket)
            m_MySocket.GetUSocket().SetSockOptAtSvr(USOCKETLib.tagSocketOption.soRcvBuf, 116800, USOCKETLib.tagSocketLevel.slSocket)

            m_MySocket.GetUSocket().TurnOnZipAtSvr(chkZip.Checked)

            m_MySocket.Commit(False) 'must be false for the very first switch

            EnableButtons(True)
        Else
            MsgBox(m_MySocket.GetErrorMsg())
        End If
    End Sub

    Private Sub btnConnect_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnConnect.Click
        m_lPrev = m_PerfQuery.Now()
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

    Private Sub OnRequestProcessed(ByVal hSocket As Integer, ByVal sRequestID As Short, ByVal nLen As Integer, ByVal nLenInBuffer As Integer, ByVal ReturnFlag As USOCKETLib.tagReturnFlag)
        If (ReturnFlag = USOCKETLib.tagReturnFlag.rfCompleted) Then
            UpdateBytes()
        End If
    End Sub

    Private Sub frmSOne_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load
        m_MySvsHandler = New CTOne
        m_MySocket = New CClientSocket
        m_MySocket.m_OnSocketClosed = New DOnSocketClosed(AddressOf OnSocketClosed)
        m_MySocket.m_OnSocketConnected = New DOnSocketConnected(AddressOf OnSocketConnected)
        m_MySocket.m_OnBaseRequestProcessed = New DOnBaseRequestProcessed(AddressOf OnBaseRequestProcessed)
        m_MySocket.m_OnRequestProcessed = New DOnRequestProcessed(AddressOf OnRequestProcessed)
        m_S3Handler = New CTThree
        m_MySvsHandler.Attach(m_MySocket)
        m_S3Handler.Attach(m_MySocket)
        m_PerfQuery = New CUPerformanceQuery
        m_Stack = New Stack
    End Sub

    Private m_MySocket As CClientSocket
    Private m_MySvsHandler As CTOne
    Private m_PerfQuery As CUPerformanceQuery
    Private m_S3Handler As CTThree
    Private m_lPrev As Long
    Private m_Stack As Stack

    Private Sub frmSOne_Closed(ByVal sender As Object, ByVal e As System.EventArgs) Handles MyBase.Closed
        m_MySvsHandler.Detach()
        m_MySocket.DestroyUSocket()
    End Sub

    Private Sub chkFrozen_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles chkFrozen.CheckedChanged
        m_MySocket.DisableUI(chkFrozen.Checked)
    End Sub

    Private Sub btnSleep_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnSleep.Click
        If m_MySocket.GetCurrentServiceID() <> m_MySvsHandler.GetSvsID() Then
            m_MySocket.SwitchTo(m_MySvsHandler)
        End If
        btnSleep.Enabled = False
        m_lPrev = m_PerfQuery.Now()

        m_MySvsHandler.Sleep(txtSleep.Text)
        txtTimeRequired.Text = m_PerfQuery.Diff(m_lPrev)
        btnSleep.Enabled = m_MySocket.IsConnected()
    End Sub

    Private Sub btnGetAllCounts_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnGetAllCounts.Click
        Dim nCount As Integer
        Dim nGlobalCount As Integer
        Dim nFastCount As Integer
        If m_MySocket.GetCurrentServiceID() <> m_MySvsHandler.GetSvsID() Then
            m_MySocket.SwitchTo(m_MySvsHandler)
        End If
        m_lPrev = m_PerfQuery.Now()
        m_MySvsHandler.GetAllCounts(nCount, nGlobalCount, nFastCount)
        txtTimeRequired.Text = m_PerfQuery.Diff(m_lPrev)

        txtCount.Text = nCount
        txtQueryGlobalCount.Text = nGlobalCount
        txtQueryGlobalFastCount.Text = nFastCount
    End Sub

    Private Sub btnQueryCount_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnQueryCount.Click
        If m_MySocket.GetCurrentServiceID() <> m_MySvsHandler.GetSvsID() Then
            m_MySocket.SwitchTo(m_MySvsHandler)
        End If
        m_lPrev = m_PerfQuery.Now()
        Dim nData As Integer = m_MySvsHandler.QueryCount()
        txtTimeRequired.Text = m_PerfQuery.Diff(m_lPrev)
        txtCount.Text = nData
    End Sub

    Private Sub btnQueryGlobalCount_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnQueryGlobalCount.Click
        If m_MySocket.GetCurrentServiceID() <> m_MySvsHandler.GetSvsID() Then
            m_MySocket.SwitchTo(m_MySvsHandler)
        End If
        m_lPrev = m_PerfQuery.Now()
        Dim nData As Integer = m_MySvsHandler.QueryGlobalCount()
        txtTimeRequired.Text = m_PerfQuery.Diff(m_lPrev)
        txtQueryGlobalCount.Text = nData
    End Sub

    Private Sub btnQueryGlobalFastCount_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnQueryGlobalFastCount.Click
        If m_MySocket.GetCurrentServiceID() <> m_MySvsHandler.GetSvsID() Then
            m_MySocket.SwitchTo(m_MySvsHandler)
        End If
        m_lPrev = m_PerfQuery.Now()
        Dim nData As Integer = m_MySvsHandler.QueryGlobalFastCount()
        txtTimeRequired.Text = m_PerfQuery.Diff(m_lPrev)
        txtQueryGlobalFastCount.Text = nData
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

        If m_MySocket.GetCurrentServiceID() <> m_MySvsHandler.GetSvsID() Then
            m_MySocket.SwitchTo(m_MySvsHandler)
        End If

        m_lPrev = m_PerfQuery.Now()
        objOut = m_MySvsHandler.Echo(objA)
        txtTimeRequired.Text = m_PerfQuery.Diff(m_lPrev)

        'put debug break here
        nTry = 0
    End Sub

    Private Sub chkUseSSL_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles chkUseSSL.CheckedChanged
        If chkUseSSL.Checked Then
            m_MySocket.GetUSocket().EncryptionMethod = USOCKETLib.tagEncryptionMethod.MSTLSv1
        Else
            m_MySocket.GetUSocket().EncryptionMethod = USOCKETLib.tagEncryptionMethod.NoEncryption
        End If
    End Sub

    Private Sub btnGetOneItemFromServer_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnGetOneItemFromServer.Click
        If (m_MySocket.GetCurrentServiceID() <> m_S3Handler.GetSvsID()) Then
            m_MySocket.SwitchTo(m_S3Handler)
        End If
        m_lPrev = m_PerfQuery.Now()
        Dim Item As CTestItem = m_S3Handler.GetOneItem()
        txtTimeRequired.Text = m_PerfQuery.Diff(m_lPrev)
    End Sub


    Private Sub btnGetALotItemsFromServer_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnGetALotItemsFromServer.Click
        If (m_MySocket.GetCurrentServiceID() <> m_S3Handler.GetSvsID()) Then
            m_MySocket.SwitchTo(m_S3Handler)
        End If
        m_lPrev = m_PerfQuery.Now()
        Dim myStack As Stack = m_S3Handler.GetManyItems(txtGetALot.Text)
        txtTimeRequired.Text = m_PerfQuery.Diff(m_lPrev)
    End Sub

    Private Sub btnSendOneItemToServer_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnSendOneItemToServer.Click
        If (m_MySocket.GetCurrentServiceID() <> m_S3Handler.GetSvsID()) Then
            m_MySocket.SwitchTo(m_S3Handler)
        End If
        Dim item As CTestItem = New CTestItem
        item.m_lData = 12345678901234
        item.m_dt = System.DateTime.Now
        item.m_strUID = m_MySocket.GetUSocket().UserID
        m_lPrev = m_PerfQuery.Now()
        m_S3Handler.SendOneItem(item)
        txtTimeRequired.Text = m_PerfQuery.Diff(m_lPrev)
    End Sub
    Public Sub UpdateBytes()
        Dim nHigh As Integer = 0
        Dim nSent As Integer = m_MySocket.GetUSocket().GetBytesSent(nHigh)
        Dim nRecv As Integer = m_MySocket.GetUSocket().GetBytesReceived(nHigh)
        txtBytesSent.Text = nSent
        txtBytesRecv.Text = nRecv
    End Sub

    Private Sub EnableButtons(ByVal bEnable As Boolean)
        btnSleep.Enabled = bEnable
        btnQueryCount.Enabled = bEnable
        btnEchoData.Enabled = bEnable
        btnGetAllCounts.Enabled = bEnable
        btnQueryGlobalFastCount.Enabled = bEnable
        btnQueryGlobalCount.Enabled = bEnable
        btnGetOneItemFromServer.Enabled = bEnable
        btnGetALotItemsFromServer.Enabled = bEnable
        btnSendOneItemToServer.Enabled = bEnable
        btnSendALotItemsToServer.Enabled = bEnable
    End Sub
    Private Sub PrepareStack(ByVal nSize As Integer)
        Dim n As Integer
        m_Stack.Clear()
        For n = 0 To nSize - 1
            Dim item As CTestItem = New CTestItem
            item.m_dt = System.DateTime.Now
            item.m_lData = n
            item.m_strUID = m_MySocket.GetUSocket().UserID
            m_Stack.Push(item)
        Next n
    End Sub

    Private Sub btnSendALotItemsToServer_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnSendALotItemsToServer.Click
        If (m_MySocket.GetCurrentServiceID() <> m_S3Handler.GetSvsID()) Then
            m_MySocket.SwitchTo(m_S3Handler)
        End If
        PrepareStack(Int32.Parse(txtSendALot.Text))
        m_lPrev = m_PerfQuery.Now()
        m_S3Handler.SendManyItems(m_Stack)
        txtTimeRequired.Text = m_PerfQuery.Diff(m_lPrev)
    End Sub
End Class
