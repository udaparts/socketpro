Imports SocketProAdapter
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
        Me.chkZip.Size = New System.Drawing.Size(48, 20)
        Me.chkZip.TabIndex = 42
        Me.chkZip.Text = "Zip ?"
        '
        'btnDisconnect
        '
        Me.btnDisconnect.Location = New System.Drawing.Point(344, 64)
        Me.btnDisconnect.Name = "btnDisconnect"
        Me.btnDisconnect.Size = New System.Drawing.Size(80, 24)
        Me.btnDisconnect.TabIndex = 41
        Me.btnDisconnect.Text = "Disconnect"
        '
        'btnConnect
        '
        Me.btnConnect.Location = New System.Drawing.Point(344, 8)
        Me.btnConnect.Name = "btnConnect"
        Me.btnConnect.Size = New System.Drawing.Size(80, 24)
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
        'frmSOne
        '
        Me.AutoScaleBaseSize = New System.Drawing.Size(5, 13)
        Me.ClientSize = New System.Drawing.Size(440, 333)
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

    Public Sub OnSocketClosed(ByVal hSocket As Integer, ByVal nError As Integer)
        btnEchoData.Enabled = False
        btnGetAllCounts.Enabled = False
        btnQueryCount.Enabled = False
        btnQueryGlobalCount.Enabled = False
        btnQueryGlobalFastCount.Enabled = False
        btnSleep.Enabled = False
    End Sub

    Public Sub OnSocketConnected(ByVal hSocket As Integer, ByVal nError As Integer)
        If nError = 0 Then
            m_Socket.SetUID(txtUserID.Text)
            m_Socket.SetPassword(txtPassword.Text)
            m_Socket.BeginBatching()
            m_Socket.SwitchTo(m_MySvsHandler)
            m_Socket.GetUSocket().TurnOnZipAtSvr(chkZip.Checked)
            m_Socket.Commit(False) 'must be false for the very first switch
            btnEchoData.Enabled = True
            btnGetAllCounts.Enabled = True
            btnQueryCount.Enabled = True
            btnQueryGlobalCount.Enabled = True
            btnQueryGlobalFastCount.Enabled = True
            btnSleep.Enabled = True
        Else
            MsgBox(m_Socket.GetErrorMsg())
        End If
    End Sub

    Private Sub btnConnect_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnConnect.Click
        m_Socket.Connect(txtHost.Text, txtPort.Text)
    End Sub

    Private Sub btnDisconnect_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnDisconnect.Click
        m_Socket.Disconnect()
    End Sub

    Private Sub chkZip_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles chkZip.CheckedChanged
        m_Socket.GetUSocket().ZipIsOn = chkZip.Checked
        If m_Socket.IsConnected() Then
            m_Socket.GetUSocket().TurnOnZipAtSvr(chkZip.Checked)
        End If
    End Sub

    Private Sub frmSOne_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load
        m_MySvsHandler = New CTOne
        m_Socket = New CClientSocket
        m_MySvsHandler.Attach(m_Socket)
        m_Socket.m_OnSocketClosed = AddressOf OnSocketClosed
        m_Socket.m_OnSocketConnected = AddressOf OnSocketConnected
    End Sub

    Private m_Socket As CClientSocket
    Private m_MySvsHandler As CTOne

    Private Sub frmSOne_Closed(ByVal sender As Object, ByVal e As System.EventArgs) Handles MyBase.Closed
        m_MySvsHandler.Detach()
        m_Socket.DestroyUSocket()
    End Sub

    Private Sub chkFrozen_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles chkFrozen.CheckedChanged
        m_Socket.DisableUI(chkFrozen.Checked)
    End Sub

    Private Sub btnSleep_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnSleep.Click
        btnSleep.Enabled = False
        m_MySvsHandler.Sleep(txtSleep.Text)
        btnSleep.Enabled = m_Socket.IsConnected()
    End Sub

    Private Sub btnGetAllCounts_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnGetAllCounts.Click
        Dim nCount As Integer
        Dim nGlobalCount As Integer
        Dim nFastCount As Integer
        m_MySvsHandler.GetAllCounts(nCount, nGlobalCount, nFastCount)
        txtCount.Text = nCount
        txtQueryGlobalCount.Text = nGlobalCount
        txtQueryGlobalFastCount.Text = nFastCount
    End Sub

    Private Sub btnQueryCount_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnQueryCount.Click
        txtCount.Text = m_MySvsHandler.QueryCount()
    End Sub

    Private Sub btnQueryGlobalCount_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnQueryGlobalCount.Click
        txtQueryGlobalCount.Text = m_MySvsHandler.QueryGlobalCount()
    End Sub

    Private Sub btnQueryGlobalFastCount_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btnQueryGlobalFastCount.Click
        txtQueryGlobalFastCount.Text = m_MySvsHandler.QueryGlobalFastCount()
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

        objOut = m_MySvsHandler.Echo(objA)

        'put debug break here
        nTry = 0
    End Sub
End Class
