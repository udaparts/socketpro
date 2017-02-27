Namespace win_async
	Partial Public Class AsyncTest
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
			Me.btnTest = New System.Windows.Forms.Button()
			Me.txtRes = New System.Windows.Forms.TextBox()
			Me.SuspendLayout()
			' 
			' btnTest
			' 
			Me.btnTest.Location = New System.Drawing.Point(34, 31)
			Me.btnTest.Name = "btnTest"
			Me.btnTest.Size = New System.Drawing.Size(84, 35)
			Me.btnTest.TabIndex = 0
			Me.btnTest.Text = "Test"
			Me.btnTest.UseVisualStyleBackColor = True
'INSTANT VB NOTE: The following InitializeComponent event wireup was converted to a 'Handles' clause:
'ORIGINAL LINE: this.btnTest.Click += new System.EventHandler(this.btnTest_Click);
			' 
			' txtRes
			' 
			Me.txtRes.Location = New System.Drawing.Point(138, 31)
			Me.txtRes.Name = "txtRes"
			Me.txtRes.Size = New System.Drawing.Size(254, 20)
			Me.txtRes.TabIndex = 1
			' 
			' async
			' 
			Me.AutoScaleDimensions = New System.Drawing.SizeF(6F, 13F)
			Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
			Me.ClientSize = New System.Drawing.Size(428, 113)
			Me.Controls.Add(Me.txtRes)
			Me.Controls.Add(Me.btnTest)
			Me.Name = "async"
			Me.Text = "Async Wait Demo"
'INSTANT VB NOTE: The following InitializeComponent event wireup was converted to a 'Handles' clause:
'ORIGINAL LINE: this.Load += new System.EventHandler(this.async_Load);
			Me.ResumeLayout(False)
			Me.PerformLayout()

		End Sub

		#End Region

		Private WithEvents btnTest As System.Windows.Forms.Button
		Private txtRes As System.Windows.Forms.TextBox
	End Class
End Namespace

