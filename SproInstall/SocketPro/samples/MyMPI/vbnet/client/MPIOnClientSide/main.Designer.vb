Namespace PPi
	Partial Public Class main
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
			Me.lbl1 = New Label()
			Me.txtStatus = New TextBox()
			Me.btnStart = New Button()
			Me.btnPause = New Button()
			Me.SuspendLayout()
			' 
			' lbl1
			' 
			Me.lbl1.AutoSize = True
			Me.lbl1.Location = New Point(14, 11)
			Me.lbl1.Name = "lbl1"
			Me.lbl1.Size = New Size(102, 13)
			Me.lbl1.TabIndex = 0
			Me.lbl1.Text = "Computation Status:"
			' 
			' txtStatus
			' 
			Me.txtStatus.Location = New Point(17, 31)
			Me.txtStatus.Name = "txtStatus"
			Me.txtStatus.Size = New Size(502, 20)
			Me.txtStatus.TabIndex = 1
			' 
			' btnStart
			' 
			Me.btnStart.Enabled = False
			Me.btnStart.Location = New Point(330, 55)
			Me.btnStart.Name = "btnStart"
			Me.btnStart.Size = New Size(92, 26)
			Me.btnStart.TabIndex = 2
			Me.btnStart.Text = "Start"
			Me.btnStart.UseVisualStyleBackColor = True
'			Me.btnStart.Click += New System.EventHandler(Me.btnStart_Click)
			' 
			' btnPause
			' 
			Me.btnPause.Enabled = False
			Me.btnPause.Location = New Point(426, 55)
			Me.btnPause.Name = "btnPause"
			Me.btnPause.Size = New Size(92, 26)
			Me.btnPause.TabIndex = 3
			Me.btnPause.Text = "Pause"
			Me.btnPause.UseVisualStyleBackColor = True
'			Me.btnPause.Click += New System.EventHandler(Me.btnPause_Click)
			' 
			' main
			' 
			Me.AutoScaleDimensions = New SizeF(6F, 13F)
			Me.AutoScaleMode = AutoScaleMode.Font
			Me.ClientSize = New Size(531, 92)
			Me.Controls.Add(Me.btnPause)
			Me.Controls.Add(Me.btnStart)
			Me.Controls.Add(Me.txtStatus)
			Me.Controls.Add(Me.lbl1)
			Me.Name = "main"
			Me.Text = "Compute Pi in Parallel on Network"
'			Me.Load += New System.EventHandler(Me.main_Load)
'			Me.FormClosed += New System.Windows.Forms.FormClosedEventHandler(Me.main_FormClosed)
			Me.ResumeLayout(False)
			Me.PerformLayout()

		End Sub
		#End Region

		Private lbl1 As Label
		Private txtStatus As TextBox
		Private WithEvents btnStart As Button
		Private WithEvents btnPause As Button
	End Class
End Namespace

