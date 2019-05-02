Imports System.Collections
Imports System.ComponentModel
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide
Imports SocketProAdapter.ClientSide
Imports USOCKETLib
Imports System.Runtime.InteropServices

Namespace SocketPool
	''' <summary>
	''' Summary description for Form1.
	''' </summary>
	Public Class frmSocketPool
		Inherits Form
		Private label1 As Label
		Private WithEvents btnStart As Button
		Private txtPort As TextBox
		Private lstInfo As ListBox
		Private lblCount As Label
		Private txtCount As TextBox
		''' <summary>
		''' Required designer variable.
		''' </summary>
		Private components As System.ComponentModel.Container = Nothing
		Private label2 As Label
		Private txtLBStatus As TextBox

		Private groupBox1 As GroupBox
		Private txtThreadCount As TextBox
		Private label3 As Label
		Private label4 As Label
		Private txtSocketsPerThread As TextBox
		Private label5 As Label
		Private txtConnectedSockets As TextBox
		Private WithEvents btnStop As Button
		Private WithEvents btnAdd As Button
		Private m_PoolSvr As CPoolSvr

		Public Sub New()
			'
			' Required for Windows Form Designer support
			'
			InitializeComponent()

			'
			' TODO: Add any constructor code after InitializeComponent call
			'
		End Sub

		''' <summary>
		''' Clean up any resources being used.
		''' </summary>
		Protected Overrides Overloads Sub Dispose(ByVal disposing As Boolean)
            If disposing Then
                If components IsNot Nothing Then
                    components.Dispose()
                End If
            End If
            MyBase.Dispose(disposing)
        End Sub

		#Region "Windows Form Designer generated code"
		''' <summary>
		''' Required method for Designer support - do not modify
		''' the contents of this method with the code editor.
		''' </summary>
		Private Sub InitializeComponent()
			Me.label1 = New Label()
			Me.btnStart = New Button()
			Me.txtPort = New TextBox()
			Me.lstInfo = New ListBox()
			Me.lblCount = New Label()
			Me.txtCount = New TextBox()
			Me.label2 = New Label()
			Me.txtLBStatus = New TextBox()
			Me.groupBox1 = New GroupBox()
			Me.txtConnectedSockets = New TextBox()
			Me.label5 = New Label()
			Me.txtSocketsPerThread = New TextBox()
			Me.label4 = New Label()
			Me.label3 = New Label()
			Me.txtThreadCount = New TextBox()
			Me.btnStop = New Button()
			Me.btnAdd = New Button()
			Me.groupBox1.SuspendLayout()
			Me.SuspendLayout()
			' 
			' label1
			' 
			Me.label1.Location = New Point(624, 0)
			Me.label1.Name = "label1"
			Me.label1.Size = New Size(120, 16)
			Me.label1.TabIndex = 0
			Me.label1.Text = "Listening socket port:"
			' 
			' btnStart
			' 
			Me.btnStart.Enabled = False
			Me.btnStart.Location = New Point(624, 40)
			Me.btnStart.Name = "btnStart"
			Me.btnStart.Size = New Size(136, 24)
			Me.btnStart.TabIndex = 1
			Me.btnStart.Text = "Start server"
'			Me.btnStart.Click += New System.EventHandler(Me.btnStart_Click)
			' 
			' txtPort
			' 
			Me.txtPort.Location = New Point(624, 16)
			Me.txtPort.Name = "txtPort"
			Me.txtPort.Size = New Size(128, 20)
			Me.txtPort.TabIndex = 3
			Me.txtPort.Text = "20910"
			' 
			' lstInfo
			' 
			Me.lstInfo.Location = New Point(8, 48)
			Me.lstInfo.Name = "lstInfo"
			Me.lstInfo.Size = New Size(608, 264)
			Me.lstInfo.TabIndex = 4
			' 
			' lblCount
			' 
			Me.lblCount.Location = New Point(624, 104)
			Me.lblCount.Name = "lblCount"
			Me.lblCount.Size = New Size(120, 16)
			Me.lblCount.TabIndex = 5
			Me.lblCount.Text = "Socket Connections:"
			' 
			' txtCount
			' 
			Me.txtCount.Location = New Point(624, 128)
			Me.txtCount.Name = "txtCount"
			Me.txtCount.Size = New Size(128, 20)
			Me.txtCount.TabIndex = 6
			' 
			' label2
			' 
			Me.label2.Location = New Point(8, 0)
			Me.label2.Name = "label2"
			Me.label2.Size = New Size(200, 16)
			Me.label2.TabIndex = 7
			Me.label2.Text = "RAdo Loading Balance Status:"
			' 
			' txtLBStatus
			' 
			Me.txtLBStatus.Location = New Point(8, 16)
			Me.txtLBStatus.Name = "txtLBStatus"
			Me.txtLBStatus.ReadOnly = True
			Me.txtLBStatus.Size = New Size(466, 20)
			Me.txtLBStatus.TabIndex = 8
			' 
			' groupBox1
			' 
			Me.groupBox1.Controls.Add(Me.txtConnectedSockets)
			Me.groupBox1.Controls.Add(Me.label5)
			Me.groupBox1.Controls.Add(Me.txtSocketsPerThread)
			Me.groupBox1.Controls.Add(Me.label4)
			Me.groupBox1.Controls.Add(Me.label3)
			Me.groupBox1.Controls.Add(Me.txtThreadCount)
			Me.groupBox1.Location = New Point(624, 160)
			Me.groupBox1.Name = "groupBox1"
			Me.groupBox1.Size = New Size(136, 152)
			Me.groupBox1.TabIndex = 9
			Me.groupBox1.TabStop = False
			Me.groupBox1.Text = "Pool Status:"
			' 
			' txtConnectedSockets
			' 
			Me.txtConnectedSockets.Location = New Point(8, 120)
			Me.txtConnectedSockets.Name = "txtConnectedSockets"
			Me.txtConnectedSockets.ReadOnly = True
			Me.txtConnectedSockets.Size = New Size(88, 20)
			Me.txtConnectedSockets.TabIndex = 5
			Me.txtConnectedSockets.Text = "0"
			' 
			' label5
			' 
			Me.label5.Location = New Point(8, 104)
			Me.label5.Name = "label5"
			Me.label5.Size = New Size(112, 16)
			Me.label5.TabIndex = 4
			Me.label5.Text = "Connected Sockets"
			' 
			' txtSocketsPerThread
			' 
			Me.txtSocketsPerThread.Location = New Point(8, 80)
			Me.txtSocketsPerThread.Name = "txtSocketsPerThread"
			Me.txtSocketsPerThread.Size = New Size(72, 20)
			Me.txtSocketsPerThread.TabIndex = 3
			Me.txtSocketsPerThread.Text = "2"
			' 
			' label4
			' 
			Me.label4.Location = New Point(8, 64)
			Me.label4.Name = "label4"
			Me.label4.Size = New Size(120, 16)
			Me.label4.TabIndex = 2
			Me.label4.Text = "Sockets per thread:"
			' 
			' label3
			' 
			Me.label3.Location = New Point(8, 24)
			Me.label3.Name = "label3"
			Me.label3.Size = New Size(104, 16)
			Me.label3.TabIndex = 1
			Me.label3.Text = "Threads count:"
			' 
			' txtThreadCount
			' 
			Me.txtThreadCount.Location = New Point(8, 40)
			Me.txtThreadCount.Name = "txtThreadCount"
			Me.txtThreadCount.Size = New Size(56, 20)
			Me.txtThreadCount.TabIndex = 0
			Me.txtThreadCount.Text = "3"
			' 
			' btnStop
			' 
			Me.btnStop.Location = New Point(624, 71)
			Me.btnStop.Name = "btnStop"
			Me.btnStop.Size = New Size(136, 23)
			Me.btnStop.TabIndex = 10
			Me.btnStop.Text = "Stop"
			Me.btnStop.UseVisualStyleBackColor = True
'			Me.btnStop.Click += New System.EventHandler(Me.btnStop_Click)
			' 
			' btnAdd
			' 
			Me.btnAdd.Location = New Point(480, 16)
			Me.btnAdd.Name = "btnAdd"
			Me.btnAdd.Size = New Size(136, 23)
			Me.btnAdd.TabIndex = 11
			Me.btnAdd.Text = "Add a real server"
			Me.btnAdd.UseVisualStyleBackColor = True
'			Me.btnAdd.Click += New System.EventHandler(Me.btnAdd_Click)
			' 
			' frmSocketPool
			' 
			Me.AutoScaleBaseSize = New Size(5, 13)
			Me.ClientSize = New Size(776, 333)
			Me.Controls.Add(Me.btnAdd)
			Me.Controls.Add(Me.btnStop)
			Me.Controls.Add(Me.groupBox1)
			Me.Controls.Add(Me.txtLBStatus)
			Me.Controls.Add(Me.txtCount)
			Me.Controls.Add(Me.txtPort)
			Me.Controls.Add(Me.label2)
			Me.Controls.Add(Me.lblCount)
			Me.Controls.Add(Me.lstInfo)
			Me.Controls.Add(Me.btnStart)
			Me.Controls.Add(Me.label1)
			Me.Name = "frmSocketPool"
			Me.Text = "A server side application to demonstrate SocketPro pool"
'			Me.Load += New System.EventHandler(Me.frmSocketPool_Load)
'			Me.FormClosed += New System.Windows.Forms.FormClosedEventHandler(Me.frmSocketPool_Closed)
			Me.groupBox1.ResumeLayout(False)
			Me.groupBox1.PerformLayout()
			Me.ResumeLayout(False)
			Me.PerformLayout()

		End Sub
		#End Region

		''' <summary>
		''' The main entry point for the application.
		''' </summary>

        <STAThread()> _
  Shared Sub Main()
            Application.Run(New frmSocketPool())
        End Sub

		Friend m_OnIsPermitted As DPermmitEvent
		Friend m_OnClose As DSocketEvent
		Friend m_OnAccept As DSocketEvent
		Friend m_OnUpdateLBStatus As DUpdateLBStatus

		Private Sub InitializeSocketPool()
			m_PoolSvr = New CPoolSvr()
			m_PoolSvr.m_frmSocketPool = Me
            m_OnClose = AddressOf OnClose
            m_OnAccept = AddressOf OnAccept
            m_OnIsPermitted = AddressOf OnIsPermitted
            m_OnUpdateLBStatus = AddressOf OnUpdateLBStatus
		End Sub

		Private Sub OnUpdateLBStatus()
			System.Threading.Thread.Sleep(0)
			Dim strMsg As String = ("Parallels = " & m_PoolSvr.m_RadoPoolSvs.SocketPool.SocketsInParallel.ToString())
			strMsg &= (", Fails = " & m_PoolSvr.m_RadoPoolSvs.SocketPool.Fails.ToString())
			strMsg &= (", Paused = " & m_PoolSvr.m_RadoPoolSvs.SocketPool.Paused.ToString())
			strMsg &= (", Working = " & m_PoolSvr.m_RadoPoolSvs.SocketPool.Working.ToString())
			strMsg &= (", Queue size = " & m_PoolSvr.m_RadoPoolSvs.SocketPool.JobManager.CountOfJobs.ToString())
			txtLBStatus.Text = strMsg
			txtConnectedSockets.Text = m_PoolSvr.m_RadoPoolSvs.SocketPool.GetUSocketPool().ConnectedSocketsEx.ToString()
		End Sub

		Private Sub frmSocketPool_Load(ByVal sender As Object, ByVal e As EventArgs) Handles MyBase.Load
			InitializeSocketPool()
			btnStart.Enabled = True
		End Sub

		Private Sub btnStart_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnStart.Click
			Dim bThreads As Byte = CByte(Int32.Parse(txtThreadCount.Text))
			Dim bSocketsPerThread As Byte = CByte(Int32.Parse(Me.txtSocketsPerThread.Text))
			Dim nPort As Integer = Int32.Parse(txtPort.Text)
			Dim strOLEDBConnection As String = txtLBStatus.Text
            m_PoolSvr.RunMe(nPort, bThreads, bSocketsPerThread)
			If m_PoolSvr.IsRunning() Then
				btnStart.Enabled = False
				Dim str As String = Nothing
				str &= m_PoolSvr.m_RadoPoolSvs.SocketPool.GetUSocketPool().ConnectedSocketsEx
				txtConnectedSockets.Text = str
			End If
			OnUpdateLBStatus()
		End Sub

        <DllImport("usktpror.dll")> _
  Private Shared Function GetPeerName(ByVal hSocket As UInteger, ByRef pnPeerPort As UInteger, <MarshalAs(UnmanagedType.LPWStr)> ByVal strPeerAddr As String, ByVal usChars As UShort) As Boolean
        End Function
		Private Function GetPeerName(ByVal hSocket As Integer, <System.Runtime.InteropServices.Out()> ByRef nPort As Integer) As String
			Dim uPort As UInteger = 0
			Dim str As New String(ChrW(0), 256)
            Dim ok As Boolean = GetPeerName(CUInt(hSocket), uPort, str, 256)
            Dim nLen As Integer = str.IndexOf(ChrW(0), 0, 256)
            If (Not ok) OrElse nLen = 0 Then
                nPort = 0
                Return Nothing
            End If
			nPort = CInt(Fix(uPort))
			str = str.Remove(nLen, 256-nLen)
			Return str
		End Function

		Private Function OnIsPermitted(ByVal hSocket As Integer, ByVal lSvsID As Integer) As Boolean
			Dim strUID As String = CSocketProServer.GetUserID(hSocket)

			Dim nPort As Integer = 0
			Dim str As String = GetPeerName(hSocket, nPort)
			If str IsNot Nothing AndAlso str.Length > 0 Then
				Dim strMsg As String = strUID & " from "
				strMsg &= str
				strMsg &= "@"
				strMsg &= nPort
				lstInfo.Items.Add(strMsg)
				lstInfo.Show()
			End If

			Return True
		End Function

		Private Sub OnClose(ByVal hSocket As Integer, ByVal nError As Integer)
			Dim str As String = Nothing
			System.Threading.Thread.Sleep(0)
			str &= CSocketProServer.CountOfClients
			txtCount.Text = str

			If nError <> 0 Then
				Dim nPort As Integer = 0
				Dim strIPAddr As String = GetPeerName(hSocket, nPort)
				If str IsNot Nothing AndAlso str.Length > 0 Then
					Dim strMsg As String = strIPAddr
					strMsg &= "@"
					strMsg &= nPort
					strMsg &= " disconnected because of error code = "
					strMsg &= nError
					lstInfo.Items.Add(strMsg)
					lstInfo.Show()
				End If
			End If
		End Sub

		Private Sub OnAccept(ByVal hSocket As Integer, ByVal nError As Integer)
			Dim str As String = Nothing
			str &= CSocketProServer.CountOfClients
			txtCount.Text = str

			Dim nPort As Integer = 0
			str = GetPeerName(hSocket, nPort)
			If str IsNot Nothing AndAlso str.Length > 0 Then
				Dim strMsg As String = "Socket establised  from "
				strMsg &= str
				strMsg &= "@"
				strMsg &= nPort
				lstInfo.Items.Add(strMsg)
				lstInfo.Show()
			End If
		End Sub

		Private Sub btnStop_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnStop.Click
			m_PoolSvr.Stop()
			m_PoolSvr.Dispose()
			m_PoolSvr = New CPoolSvr()
			m_PoolSvr.m_frmSocketPool = Me
			btnStart.Enabled = True
			OnUpdateLBStatus()
		End Sub

		Private Sub btnAdd_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnAdd.Click
			If m_PoolSvr.IsRunning() Then
				Dim cc As New CConnectionContext()
				cc.m_bZip = False
				cc.m_nPort = 20901
				cc.m_strPassword = "PassOne"
				cc.m_strUID = "SocketPro"
				cc.m_strHost = "127.0.0.1"
				cc.m_EncrytionMethod = USOCKETLib.tagEncryptionMethod.NoEncryption
				If m_PoolSvr.m_RadoPoolSvs.SocketPool.MakeConnection(cc) Then
					OnUpdateLBStatus()
				End If
			End If
		End Sub
	End Class
End Namespace
