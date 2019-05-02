Imports Microsoft.VisualBasic
Imports System
Namespace MultiParam
	Public Partial Class frmMulti
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
			Me.btnConnect = New System.Windows.Forms.Button()
			Me.btnDisconnect = New System.Windows.Forms.Button()
			Me.btnInsert = New System.Windows.Forms.Button()
			Me.SuspendLayout()
			' 
			' btnConnect
			' 
			Me.btnConnect.Location = New System.Drawing.Point(12, 12)
			Me.btnConnect.Name = "btnConnect"
			Me.btnConnect.Size = New System.Drawing.Size(89, 31)
			Me.btnConnect.TabIndex = 0
			Me.btnConnect.Text = "Connect"
			Me.btnConnect.UseVisualStyleBackColor = True
'			Me.btnConnect.Click += New System.EventHandler(Me.btnConnect_Click);
			' 
			' btnDisconnect
			' 
			Me.btnDisconnect.Location = New System.Drawing.Point(108, 12)
			Me.btnDisconnect.Name = "btnDisconnect"
			Me.btnDisconnect.Size = New System.Drawing.Size(99, 30)
			Me.btnDisconnect.TabIndex = 1
			Me.btnDisconnect.Text = "Disconnect"
			Me.btnDisconnect.UseVisualStyleBackColor = True
'			Me.btnDisconnect.Click += New System.EventHandler(Me.btnDisconnect_Click);
			' 
			' btnInsert
			' 
			Me.btnInsert.Enabled = False
			Me.btnInsert.Location = New System.Drawing.Point(214, 12)
			Me.btnInsert.Name = "btnInsert"
			Me.btnInsert.Size = New System.Drawing.Size(430, 29)
			Me.btnInsert.TabIndex = 2
			Me.btnInsert.Text = "Insert Multiple Sets of Parameter Data"
			Me.btnInsert.UseVisualStyleBackColor = True
'			Me.btnInsert.Click += New System.EventHandler(Me.btnInsert_Click);
			' 
			' frmMulti
			' 
			Me.AutoScaleDimensions = New System.Drawing.SizeF(6F, 13F)
			Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
			Me.ClientSize = New System.Drawing.Size(661, 59)
			Me.Controls.Add(Me.btnInsert)
			Me.Controls.Add(Me.btnDisconnect)
			Me.Controls.Add(Me.btnConnect)
			Me.Name = "frmMulti"
			Me.Text = "Demonstration of fast insert using parameterized statement with multiple sets of " & "parameter data"
'			Me.Load += New System.EventHandler(Me.frmMulti_Load);
			Me.ResumeLayout(False)

		End Sub

		#End Region

		Private WithEvents btnConnect As System.Windows.Forms.Button
		Private WithEvents btnDisconnect As System.Windows.Forms.Button
		Private WithEvents btnInsert As System.Windows.Forms.Button
	End Class
End Namespace

