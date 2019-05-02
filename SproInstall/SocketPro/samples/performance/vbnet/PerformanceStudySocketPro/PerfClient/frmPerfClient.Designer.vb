Namespace PerfClient
	Partial Public Class frmPerfClient
		''' <summary>
		''' Required designer variable.
		''' </summary>
		Private components As System.ComponentModel.IContainer = Nothing

		''' <summary>
		''' Clean up any resources being used.
		''' </summary>
		''' <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		Protected Overrides Sub Dispose(ByVal disposing As Boolean)
			If disposing AndAlso (components IsNot Nothing) Then
				components.Dispose()
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
			Me.txtHost = New TextBox()
			Me.txtPort = New TextBox()
			Me.label2 = New Label()
			Me.btnConnect = New Button()
			Me.btnDisconnect = New Button()
			Me.chkZip = New CheckBox()
			Me.radioRealtime = New RadioButton()
			Me.radioDefault = New RadioButton()
			Me.btnEcho = New Button()
			Me.txtTime = New TextBox()
			Me.btnSQL = New Button()
			Me.txtSQL = New TextBox()
			Me.chkBatch = New CheckBox()
			Me.label3 = New Label()
			Me.SuspendLayout()
			' 
			' label1
			' 
			Me.label1.AutoSize = True
			Me.label1.Location = New Point(24, 9)
			Me.label1.Name = "label1"
			Me.label1.Size = New Size(72, 13)
			Me.label1.TabIndex = 0
			Me.label1.Text = "Remote Host:"
			' 
			' txtHost
			' 
			Me.txtHost.Location = New Point(27, 25)
			Me.txtHost.Name = "txtHost"
			Me.txtHost.Size = New Size(166, 20)
			Me.txtHost.TabIndex = 1
			Me.txtHost.Text = "localhost"
			' 
			' txtPort
			' 
			Me.txtPort.Location = New Point(210, 24)
			Me.txtPort.Name = "txtPort"
			Me.txtPort.Size = New Size(100, 20)
			Me.txtPort.TabIndex = 2
			Me.txtPort.Text = "21911"
			' 
			' label2
			' 
			Me.label2.AutoSize = True
			Me.label2.Location = New Point(210, 9)
			Me.label2.Name = "label2"
			Me.label2.Size = New Size(36, 13)
			Me.label2.TabIndex = 3
			Me.label2.Text = "Port#:"
			' 
			' btnConnect
			' 
			Me.btnConnect.Location = New Point(457, 25)
			Me.btnConnect.Name = "btnConnect"
			Me.btnConnect.Size = New Size(75, 23)
			Me.btnConnect.TabIndex = 4
			Me.btnConnect.Text = "Connect"
			Me.btnConnect.UseVisualStyleBackColor = True
'			Me.btnConnect.Click += New System.EventHandler(Me.btnConnect_Click)
			' 
			' btnDisconnect
			' 
			Me.btnDisconnect.Location = New Point(539, 24)
			Me.btnDisconnect.Name = "btnDisconnect"
			Me.btnDisconnect.Size = New Size(75, 23)
			Me.btnDisconnect.TabIndex = 5
			Me.btnDisconnect.Text = "Disconnect"
			Me.btnDisconnect.UseVisualStyleBackColor = True
'			Me.btnDisconnect.Click += New System.EventHandler(Me.btnDisconnect_Click)
			' 
			' chkZip
			' 
			Me.chkZip.AutoSize = True
			Me.chkZip.Location = New Point(317, 27)
			Me.chkZip.Name = "chkZip"
			Me.chkZip.Size = New Size(47, 17)
			Me.chkZip.TabIndex = 6
			Me.chkZip.Text = "Zip?"
			Me.chkZip.UseVisualStyleBackColor = True
'			Me.chkZip.CheckedChanged += New System.EventHandler(Me.chkZip_CheckedChanged)
			' 
			' radioRealtime
			' 
			Me.radioRealtime.AutoSize = True
			Me.radioRealtime.Checked = True
			Me.radioRealtime.Location = New Point(376, 4)
			Me.radioRealtime.Name = "radioRealtime"
			Me.radioRealtime.Size = New Size(66, 17)
			Me.radioRealtime.TabIndex = 7
			Me.radioRealtime.TabStop = True
			Me.radioRealtime.Text = "Realtime"
			Me.radioRealtime.UseVisualStyleBackColor = True
'			Me.radioRealtime.CheckedChanged += New System.EventHandler(Me.radioRealtime_CheckedChanged)
			' 
			' radioDefault
			' 
			Me.radioDefault.AutoSize = True
			Me.radioDefault.Location = New Point(376, 30)
			Me.radioDefault.Name = "radioDefault"
			Me.radioDefault.Size = New Size(59, 17)
			Me.radioDefault.TabIndex = 8
			Me.radioDefault.Text = "Default"
			Me.radioDefault.UseVisualStyleBackColor = True
'			Me.radioDefault.CheckedChanged += New System.EventHandler(Me.radioDefault_CheckedChanged)
			' 
			' btnEcho
			' 
			Me.btnEcho.Enabled = False
			Me.btnEcho.Location = New Point(27, 61)
			Me.btnEcho.Name = "btnEcho"
			Me.btnEcho.Size = New Size(166, 23)
			Me.btnEcho.TabIndex = 9
			Me.btnEcho.Text = "Test Echo 10000 times"
			Me.btnEcho.UseVisualStyleBackColor = True
'			Me.btnEcho.Click += New System.EventHandler(Me.btnEcho_Click)
			' 
			' txtTime
			' 
			Me.txtTime.Location = New Point(457, 67)
			Me.txtTime.Name = "txtTime"
			Me.txtTime.ReadOnly = True
			Me.txtTime.Size = New Size(125, 20)
			Me.txtTime.TabIndex = 10
			' 
			' btnSQL
			' 
			Me.btnSQL.Enabled = False
			Me.btnSQL.Location = New Point(27, 118)
			Me.btnSQL.Name = "btnSQL"
			Me.btnSQL.Size = New Size(166, 23)
			Me.btnSQL.TabIndex = 11
			Me.btnSQL.Text = "Execute SQL 100 times"
			Me.btnSQL.UseVisualStyleBackColor = True
'			Me.btnSQL.Click += New System.EventHandler(Me.btnSQL_Click)
			' 
			' txtSQL
			' 
			Me.txtSQL.Location = New Point(27, 92)
			Me.txtSQL.Name = "txtSQL"
			Me.txtSQL.Size = New Size(587, 20)
			Me.txtSQL.TabIndex = 12
			Me.txtSQL.Text = "Select * from Shippers"
			' 
			' chkBatch
			' 
			Me.chkBatch.AutoSize = True
			Me.chkBatch.Checked = True
			Me.chkBatch.CheckState = CheckState.Checked
			Me.chkBatch.Location = New Point(210, 69)
			Me.chkBatch.Name = "chkBatch"
			Me.chkBatch.Size = New Size(60, 17)
			Me.chkBatch.TabIndex = 13
			Me.chkBatch.Text = "Batch?"
			Me.chkBatch.UseVisualStyleBackColor = True
'			Me.chkBatch.CheckedChanged += New System.EventHandler(Me.chkBatch_CheckedChanged)
			' 
			' label3
			' 
			Me.label3.AutoSize = True
			Me.label3.Location = New Point(336, 70)
			Me.label3.Name = "label3"
			Me.label3.Size = New Size(116, 13)
			Me.label3.TabIndex = 14
			Me.label3.Text = "Time (in micro-second):"
			Me.label3.TextAlign = ContentAlignment.MiddleRight
			' 
			' frmPerfClient
			' 
			Me.AutoScaleDimensions = New SizeF(6F, 13F)
			Me.AutoScaleMode = AutoScaleMode.Font
			Me.ClientSize = New Size(635, 156)
			Me.Controls.Add(Me.label3)
			Me.Controls.Add(Me.chkBatch)
			Me.Controls.Add(Me.txtSQL)
			Me.Controls.Add(Me.btnSQL)
			Me.Controls.Add(Me.txtTime)
			Me.Controls.Add(Me.btnEcho)
			Me.Controls.Add(Me.radioDefault)
			Me.Controls.Add(Me.radioRealtime)
			Me.Controls.Add(Me.chkZip)
			Me.Controls.Add(Me.btnDisconnect)
			Me.Controls.Add(Me.btnConnect)
			Me.Controls.Add(Me.label2)
			Me.Controls.Add(Me.txtPort)
			Me.Controls.Add(Me.txtHost)
			Me.Controls.Add(Me.label1)
			Me.Name = "frmPerfClient"
			Me.Text = "SocketPro Performance Investigation"
'			Me.Load += New System.EventHandler(Me.frmPerfClient_Load)
'			Me.FormClosing += New System.Windows.Forms.FormClosingEventHandler(Me.frmPerfClient_Closing)
			Me.ResumeLayout(False)
			Me.PerformLayout()

		End Sub

		#End Region

		Private label1 As Label
		Private txtHost As TextBox
		Private txtPort As TextBox
		Private label2 As Label
		Private WithEvents btnConnect As Button
		Private WithEvents btnDisconnect As Button
		Private WithEvents chkZip As CheckBox
		Private WithEvents radioRealtime As RadioButton
		Private WithEvents radioDefault As RadioButton
		Private WithEvents btnEcho As Button
		Private txtTime As TextBox
		Private WithEvents btnSQL As Button
		Private txtSQL As TextBox
		Private WithEvents chkBatch As CheckBox
		Private label3 As Label
	End Class
End Namespace

