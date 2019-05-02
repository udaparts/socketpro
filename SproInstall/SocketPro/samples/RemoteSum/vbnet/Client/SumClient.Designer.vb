Namespace Client
	Partial Public Class SumClient
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
			Me.txtSum = New TextBox()
			Me.label1 = New Label()
			Me.btnDoSum = New Button()
			Me.btnPause = New Button()
			Me.btnRedoSum = New Button()
			Me.SuspendLayout()
			' 
			' txtSum
			' 
			Me.txtSum.Location = New Point(48, 31)
			Me.txtSum.Name = "txtSum"
			Me.txtSum.Size = New Size(152, 20)
			Me.txtSum.TabIndex = 0
			' 
			' label1
			' 
			Me.label1.AutoSize = True
			Me.label1.Location = New Point(48, 12)
			Me.label1.Name = "label1"
			Me.label1.Size = New Size(64, 13)
			Me.label1.TabIndex = 1
			Me.label1.Text = "Sum Result:"
			' 
			' btnDoSum
			' 
			Me.btnDoSum.Enabled = False
			Me.btnDoSum.Location = New Point(202, 29)
			Me.btnDoSum.Name = "btnDoSum"
			Me.btnDoSum.Size = New Size(84, 22)
			Me.btnDoSum.TabIndex = 2
			Me.btnDoSum.Text = "Do Sum"
			Me.btnDoSum.UseVisualStyleBackColor = True
'			Me.btnDoSum.Click += New System.EventHandler(Me.btnDoSum_Click)
			' 
			' btnPause
			' 
			Me.btnPause.Enabled = False
			Me.btnPause.Location = New Point(113, 58)
			Me.btnPause.Name = "btnPause"
			Me.btnPause.Size = New Size(83, 23)
			Me.btnPause.TabIndex = 3
			Me.btnPause.Text = "Pause"
			Me.btnPause.UseVisualStyleBackColor = True
'			Me.btnPause.Click += New System.EventHandler(Me.btnPause_Click)
			' 
			' btnRedoSum
			' 
			Me.btnRedoSum.Enabled = False
			Me.btnRedoSum.Location = New Point(202, 57)
			Me.btnRedoSum.Name = "btnRedoSum"
			Me.btnRedoSum.Size = New Size(84, 23)
			Me.btnRedoSum.TabIndex = 4
			Me.btnRedoSum.Text = "Redo Sum"
			Me.btnRedoSum.UseVisualStyleBackColor = True
'			Me.btnRedoSum.Click += New System.EventHandler(Me.btnRedoSum_Click)
			' 
			' SumClient
			' 
			Me.AutoScaleDimensions = New SizeF(6F, 13F)
			Me.AutoScaleMode = AutoScaleMode.Font
			Me.ClientSize = New Size(386, 97)
			Me.Controls.Add(Me.btnRedoSum)
			Me.Controls.Add(Me.btnPause)
			Me.Controls.Add(Me.btnDoSum)
			Me.Controls.Add(Me.label1)
			Me.Controls.Add(Me.txtSum)
			Me.Name = "SumClient"
			Me.Text = "Cancel Demo with Keeping Stateful Members"
'			Me.Load += New System.EventHandler(Me.SumClient_Load)
			Me.ResumeLayout(False)
			Me.PerformLayout()

		End Sub

		#End Region

		Private txtSum As TextBox
		Private label1 As Label
		Private WithEvents btnDoSum As Button
		Private WithEvents btnPause As Button
		Private WithEvents btnRedoSum As Button
	End Class
End Namespace

