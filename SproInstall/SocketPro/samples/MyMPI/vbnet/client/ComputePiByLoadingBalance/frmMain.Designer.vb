Namespace PiLBClient
	Partial Public Class frmMain
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
			Me.btnConnect = New Button()
			Me.btnClose = New Button()
			Me.label1 = New Label()
			Me.txtHost = New TextBox()
			Me.label2 = New Label()
			Me.txtPort = New TextBox()
			Me.btnStart = New Button()
			Me.txtPi = New TextBox()
			Me.label3 = New Label()
			Me.btnCancel = New Button()
			Me.SuspendLayout()
			' 
			' btnConnect
			' 
			Me.btnConnect.Location = New Point(349, 13)
			Me.btnConnect.Name = "btnConnect"
			Me.btnConnect.Size = New Size(75, 23)
			Me.btnConnect.TabIndex = 0
			Me.btnConnect.Text = "Connect"
			Me.btnConnect.UseVisualStyleBackColor = True
'			Me.btnConnect.Click += New System.EventHandler(Me.btnConnect_Click)
			' 
			' btnClose
			' 
			Me.btnClose.Location = New Point(349, 42)
			Me.btnClose.Name = "btnClose"
			Me.btnClose.Size = New Size(75, 23)
			Me.btnClose.TabIndex = 1
			Me.btnClose.Text = "Close"
			Me.btnClose.UseVisualStyleBackColor = True
'			Me.btnClose.Click += New System.EventHandler(Me.btnClose_Click)
			' 
			' label1
			' 
			Me.label1.AutoSize = True
			Me.label1.Location = New Point(13, 4)
			Me.label1.Name = "label1"
			Me.label1.Size = New Size(132, 13)
			Me.label1.TabIndex = 2
			Me.label1.Text = "Address to Load Balancer:"
			' 
			' txtHost
			' 
			Me.txtHost.Location = New Point(16, 21)
			Me.txtHost.Name = "txtHost"
			Me.txtHost.Size = New Size(176, 20)
			Me.txtHost.TabIndex = 3
			Me.txtHost.Text = "localhost"
			' 
			' label2
			' 
			Me.label2.AutoSize = True
			Me.label2.Location = New Point(241, 4)
			Me.label2.Name = "label2"
			Me.label2.Size = New Size(29, 13)
			Me.label2.TabIndex = 4
			Me.label2.Text = "Port:"
			' 
			' txtPort
			' 
			Me.txtPort.Location = New Point(244, 20)
			Me.txtPort.Name = "txtPort"
			Me.txtPort.Size = New Size(97, 20)
			Me.txtPort.TabIndex = 5
			Me.txtPort.Text = "20910"
			' 
			' btnStart
			' 
			Me.btnStart.Enabled = False
			Me.btnStart.Location = New Point(349, 72)
			Me.btnStart.Name = "btnStart"
			Me.btnStart.Size = New Size(75, 23)
			Me.btnStart.TabIndex = 6
			Me.btnStart.Text = "Start"
			Me.btnStart.UseVisualStyleBackColor = True
'			Me.btnStart.Click += New System.EventHandler(Me.btnStart_Click)
			' 
			' txtPi
			' 
			Me.txtPi.Location = New Point(16, 72)
			Me.txtPi.Name = "txtPi"
			Me.txtPi.Size = New Size(325, 20)
			Me.txtPi.TabIndex = 7
			' 
			' label3
			' 
			Me.label3.AutoSize = True
			Me.label3.Location = New Point(16, 51)
			Me.label3.Name = "label3"
			Me.label3.Size = New Size(124, 13)
			Me.label3.TabIndex = 8
			Me.label3.Text = "Computation in Progress:"
			' 
			' btnCancel
			' 
			Me.btnCancel.Location = New Point(349, 102)
			Me.btnCancel.Name = "btnCancel"
			Me.btnCancel.Size = New Size(75, 23)
			Me.btnCancel.TabIndex = 9
			Me.btnCancel.Text = "Cancel"
			Me.btnCancel.UseVisualStyleBackColor = True
'			Me.btnCancel.Click += New System.EventHandler(Me.btnCancel_Click)
			' 
			' frmMain
			' 
			Me.AutoScaleDimensions = New SizeF(6F, 13F)
			Me.AutoScaleMode = AutoScaleMode.Font
			Me.ClientSize = New Size(431, 136)
			Me.Controls.Add(Me.btnCancel)
			Me.Controls.Add(Me.label3)
			Me.Controls.Add(Me.txtPi)
			Me.Controls.Add(Me.btnStart)
			Me.Controls.Add(Me.txtPort)
			Me.Controls.Add(Me.label2)
			Me.Controls.Add(Me.txtHost)
			Me.Controls.Add(Me.label1)
			Me.Controls.Add(Me.btnClose)
			Me.Controls.Add(Me.btnConnect)
			Me.Name = "frmMain"
			Me.Text = "Compute Pi in Parallel with networked machines"
			Me.ResumeLayout(False)
			Me.PerformLayout()

		End Sub

		#End Region

		Private WithEvents btnConnect As Button
		Private WithEvents btnClose As Button
		Private label1 As Label
		Private txtHost As TextBox
		Private label2 As Label
		Private txtPort As TextBox
		Private WithEvents btnStart As Button
		Private txtPi As TextBox
		Private label3 As Label
		Private WithEvents btnCancel As Button
	End Class
End Namespace

