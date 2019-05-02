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
			Me.groupBox1 = New System.Windows.Forms.GroupBox()
			Me.btnMyEcho = New System.Windows.Forms.Button()
			Me.btnSQL = New System.Windows.Forms.Button()
			Me.txtSQL = New System.Windows.Forms.TextBox()
			Me.label5 = New System.Windows.Forms.Label()
			Me.txtTime = New System.Windows.Forms.TextBox()
			Me.groupBox1.SuspendLayout()
			Me.SuspendLayout()
			' 
			' groupBox1
			' 
			Me.groupBox1.Controls.Add(Me.btnMyEcho)
			Me.groupBox1.Controls.Add(Me.btnSQL)
			Me.groupBox1.Controls.Add(Me.txtSQL)
			Me.groupBox1.Controls.Add(Me.label5)
			Me.groupBox1.Controls.Add(Me.txtTime)
			Me.groupBox1.Location = New System.Drawing.Point(22, 12)
			Me.groupBox1.Name = "groupBox1"
			Me.groupBox1.Size = New System.Drawing.Size(646, 110)
			Me.groupBox1.TabIndex = 14
			Me.groupBox1.TabStop = False
			Me.groupBox1.Text = "WCF"
			' 
			' btnMyEcho
			' 
			Me.btnMyEcho.Location = New System.Drawing.Point(8, 67)
			Me.btnMyEcho.Name = "btnMyEcho"
			Me.btnMyEcho.Size = New System.Drawing.Size(516, 24)
			Me.btnMyEcho.TabIndex = 6
			Me.btnMyEcho.Text = "Execute 10000 requests"
'			Me.btnMyEcho.Click += New System.EventHandler(Me.btnMyEcho_Click);
			' 
			' btnSQL
			' 
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
			' frmPerfClient
			' 
			Me.AutoScaleDimensions = New System.Drawing.SizeF(6F, 13F)
			Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
			Me.ClientSize = New System.Drawing.Size(692, 144)
			Me.Controls.Add(Me.groupBox1)
			Me.Name = "frmPerfClient"
			Me.Text = "WCF Performance Investigation"
'			Me.Load += New System.EventHandler(Me.frmPerfClient_Load);
			Me.groupBox1.ResumeLayout(False)
			Me.groupBox1.PerformLayout()
			Me.ResumeLayout(False)

		End Sub

		#End Region

		Private groupBox1 As System.Windows.Forms.GroupBox
		Private WithEvents btnMyEcho As System.Windows.Forms.Button
		Private WithEvents btnSQL As System.Windows.Forms.Button
		Private txtSQL As System.Windows.Forms.TextBox
		Private label5 As System.Windows.Forms.Label
		Private txtTime As System.Windows.Forms.TextBox
	End Class
End Namespace

