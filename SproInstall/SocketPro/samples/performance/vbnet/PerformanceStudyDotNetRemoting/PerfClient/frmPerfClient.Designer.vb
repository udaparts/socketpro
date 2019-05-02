Imports Microsoft.VisualBasic
Imports System
Namespace PerfClient
	Public Partial Class frmPerfClient
		''' <summary>
		''' Required designer variable.
		''' </summary>
		Private components As System.ComponentModel.IContainer = Nothing

		''' <summary>
		''' Clean up any resources being used.
		''' </summary>
		''' <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		Protected Overrides Sub Dispose(ByVal disposing As Boolean)
			If disposing AndAlso (Not components Is Nothing) Then
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
			Me.txtNetPort = New System.Windows.Forms.TextBox()
			Me.txtHost = New System.Windows.Forms.TextBox()
			Me.label2 = New System.Windows.Forms.Label()
			Me.label1 = New System.Windows.Forms.Label()
			Me.groupBox1 = New System.Windows.Forms.GroupBox()
			Me.btnMyEcho = New System.Windows.Forms.Button()
			Me.btnSQL = New System.Windows.Forms.Button()
			Me.txtSQL = New System.Windows.Forms.TextBox()
			Me.label5 = New System.Windows.Forms.Label()
			Me.txtTime = New System.Windows.Forms.TextBox()
			Me.btnDisconnect = New System.Windows.Forms.Button()
			Me.btnConnect = New System.Windows.Forms.Button()
			Me.groupBox1.SuspendLayout()
			Me.SuspendLayout()
			' 
			' txtNetPort
			' 
			Me.txtNetPort.Location = New System.Drawing.Point(230, 39)
			Me.txtNetPort.Name = "txtNetPort"
			Me.txtNetPort.Size = New System.Drawing.Size(76, 20)
			Me.txtNetPort.TabIndex = 12
			Me.txtNetPort.Text = "21910"
			' 
			' txtHost
			' 
			Me.txtHost.Location = New System.Drawing.Point(14, 39)
			Me.txtHost.Name = "txtHost"
			Me.txtHost.Size = New System.Drawing.Size(207, 20)
			Me.txtHost.TabIndex = 11
			Me.txtHost.Text = "localhost"
			' 
			' label2
			' 
			Me.label2.Location = New System.Drawing.Point(230, 12)
			Me.label2.Name = "label2"
			Me.label2.Size = New System.Drawing.Size(128, 21)
			Me.label2.TabIndex = 10
			Me.label2.Text = "dotNet Remoting Port:"
			' 
			' label1
			' 
			Me.label1.Location = New System.Drawing.Point(14, 13)
			Me.label1.Name = "label1"
			Me.label1.Size = New System.Drawing.Size(132, 20)
			Me.label1.TabIndex = 9
			Me.label1.Text = "Host Address:"
			' 
			' groupBox1
			' 
			Me.groupBox1.Controls.Add(Me.btnMyEcho)
			Me.groupBox1.Controls.Add(Me.btnSQL)
			Me.groupBox1.Controls.Add(Me.txtSQL)
			Me.groupBox1.Controls.Add(Me.label5)
			Me.groupBox1.Controls.Add(Me.txtTime)
			Me.groupBox1.Location = New System.Drawing.Point(13, 69)
			Me.groupBox1.Name = "groupBox1"
			Me.groupBox1.Size = New System.Drawing.Size(646, 105)
			Me.groupBox1.TabIndex = 13
			Me.groupBox1.TabStop = False
			Me.groupBox1.Text = "dotNet Remoting"
			' 
			' btnMyEcho
			' 
			Me.btnMyEcho.Enabled = False
			Me.btnMyEcho.Location = New System.Drawing.Point(8, 67)
			Me.btnMyEcho.Name = "btnMyEcho"
			Me.btnMyEcho.Size = New System.Drawing.Size(516, 24)
			Me.btnMyEcho.TabIndex = 6
			Me.btnMyEcho.Text = "Execute 10000 requests"
'			Me.btnMyEcho.Click += New System.EventHandler(Me.btnMyEcho_Click);
			' 
			' btnSQL
			' 
			Me.btnSQL.Enabled = False
			Me.btnSQL.Location = New System.Drawing.Point(468, 30)
			Me.btnSQL.Name = "btnSQL"
			Me.btnSQL.Size = New System.Drawing.Size(154, 20)
			Me.btnSQL.TabIndex = 5
			Me.btnSQL.Text = "Execute SQL 100 Times"
'			Me.btnSQL.Click += New System.EventHandler(Me.btnSQL_Click);
			' 
			' txtSQL
			' 
			Me.txtSQL.Location = New System.Drawing.Point(8, 28)
			Me.txtSQL.Name = "txtSQL"
			Me.txtSQL.Size = New System.Drawing.Size(454, 20)
			Me.txtSQL.TabIndex = 4
			Me.txtSQL.Text = "Select * from Shippers"
			' 
			' label5
			' 
			Me.label5.Location = New System.Drawing.Point(526, 54)
			Me.label5.Name = "label5"
			Me.label5.Size = New System.Drawing.Size(78, 12)
			Me.label5.TabIndex = 2
			Me.label5.Text = "Time (us):"
			' 
			' txtTime
			' 
			Me.txtTime.Location = New System.Drawing.Point(526, 70)
			Me.txtTime.Name = "txtTime"
			Me.txtTime.Size = New System.Drawing.Size(96, 20)
			Me.txtTime.TabIndex = 0
			Me.txtTime.Text = "0"
			' 
			' btnDisconnect
			' 
			Me.btnDisconnect.Location = New System.Drawing.Point(559, 35)
			Me.btnDisconnect.Name = "btnDisconnect"
			Me.btnDisconnect.Size = New System.Drawing.Size(100, 24)
			Me.btnDisconnect.TabIndex = 15
			Me.btnDisconnect.Text = "Disconnect"
			' 
			' btnConnect
			' 
			Me.btnConnect.Location = New System.Drawing.Point(559, 7)
			Me.btnConnect.Name = "btnConnect"
			Me.btnConnect.Size = New System.Drawing.Size(100, 24)
			Me.btnConnect.TabIndex = 14
			Me.btnConnect.Text = "Connect"
'			Me.btnConnect.Click += New System.EventHandler(Me.btnConnect_Click);
			' 
			' frmPerfClient
			' 
			Me.AutoScaleDimensions = New System.Drawing.SizeF(6F, 13F)
			Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
			Me.ClientSize = New System.Drawing.Size(682, 192)
			Me.Controls.Add(Me.btnDisconnect)
			Me.Controls.Add(Me.btnConnect)
			Me.Controls.Add(Me.groupBox1)
			Me.Controls.Add(Me.txtNetPort)
			Me.Controls.Add(Me.txtHost)
			Me.Controls.Add(Me.label2)
			Me.Controls.Add(Me.label1)
			Me.Name = "frmPerfClient"
			Me.Text = "DotNet Remoting Performance Investigation"
'			Me.Load += New System.EventHandler(Me.frmPerfClient_Load);
			Me.groupBox1.ResumeLayout(False)
			Me.groupBox1.PerformLayout()
			Me.ResumeLayout(False)
			Me.PerformLayout()

		End Sub

		#End Region

		Private txtNetPort As System.Windows.Forms.TextBox
		Private txtHost As System.Windows.Forms.TextBox
		Private label2 As System.Windows.Forms.Label
		Private label1 As System.Windows.Forms.Label
		Private groupBox1 As System.Windows.Forms.GroupBox
		Private WithEvents btnMyEcho As System.Windows.Forms.Button
		Private WithEvents btnSQL As System.Windows.Forms.Button
		Private txtSQL As System.Windows.Forms.TextBox
		Private label5 As System.Windows.Forms.Label
		Private txtTime As System.Windows.Forms.TextBox
		Private btnDisconnect As System.Windows.Forms.Button
		Private WithEvents btnConnect As System.Windows.Forms.Button
	End Class
End Namespace

