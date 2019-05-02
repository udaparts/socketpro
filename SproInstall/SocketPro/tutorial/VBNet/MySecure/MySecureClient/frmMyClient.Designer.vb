Namespace MySecureClient
	Partial Public Class frmMySecure
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
			Me.btnDisconnect = New Button()
			Me.txtAddress = New TextBox()
			Me.label1 = New Label()
			Me.label2 = New Label()
			Me.txtPort = New TextBox()
			Me.label3 = New Label()
			Me.txtUserID = New TextBox()
			Me.label4 = New Label()
			Me.txtPassword = New TextBox()
			Me.btnToDB = New Button()
			Me.label5 = New Label()
			Me.txtSQL = New TextBox()
			Me.btnExecuteSQL = New Button()
			Me.SuspendLayout()
			' 
			' btnConnect
			' 
			Me.btnConnect.Location = New Point(311, 12)
			Me.btnConnect.Name = "btnConnect"
			Me.btnConnect.Size = New Size(93, 29)
			Me.btnConnect.TabIndex = 0
			Me.btnConnect.Text = "Connect"
			Me.btnConnect.UseVisualStyleBackColor = True
'			Me.btnConnect.Click += New System.EventHandler(Me.btnConnect_Click)
			' 
			' btnDisconnect
			' 
			Me.btnDisconnect.Location = New Point(311, 48)
			Me.btnDisconnect.Name = "btnDisconnect"
			Me.btnDisconnect.Size = New Size(93, 31)
			Me.btnDisconnect.TabIndex = 1
			Me.btnDisconnect.Text = "Disconnect"
			Me.btnDisconnect.UseVisualStyleBackColor = True
'			Me.btnDisconnect.Click += New System.EventHandler(Me.btnDisconnect_Click)
			' 
			' txtAddress
			' 
			Me.txtAddress.Location = New Point(12, 25)
			Me.txtAddress.Name = "txtAddress"
			Me.txtAddress.Size = New Size(142, 20)
			Me.txtAddress.TabIndex = 2
			Me.txtAddress.Text = "localhost"
			' 
			' label1
			' 
			Me.label1.AutoSize = True
			Me.label1.Location = New Point(12, 9)
			Me.label1.Name = "label1"
			Me.label1.Size = New Size(113, 13)
			Me.label1.TabIndex = 3
			Me.label1.Text = "Remote Host Address:"
			' 
			' label2
			' 
			Me.label2.AutoSize = True
			Me.label2.Location = New Point(160, 9)
			Me.label2.Name = "label2"
			Me.label2.Size = New Size(39, 13)
			Me.label2.TabIndex = 4
			Me.label2.Text = "Port #:"
			' 
			' txtPort
			' 
			Me.txtPort.Location = New Point(160, 25)
			Me.txtPort.Name = "txtPort"
			Me.txtPort.Size = New Size(75, 20)
			Me.txtPort.TabIndex = 5
			Me.txtPort.Text = "20901"
			' 
			' label3
			' 
			Me.label3.AutoSize = True
			Me.label3.Location = New Point(7, 60)
			Me.label3.Name = "label3"
			Me.label3.Size = New Size(46, 13)
			Me.label3.TabIndex = 6
			Me.label3.Text = "User ID:"
			' 
			' txtUserID
			' 
			Me.txtUserID.Location = New Point(54, 56)
			Me.txtUserID.Name = "txtUserID"
			Me.txtUserID.Size = New Size(74, 20)
			Me.txtUserID.TabIndex = 7
			Me.txtUserID.Text = "SocketPro"
			' 
			' label4
			' 
			Me.label4.AutoSize = True
			Me.label4.Location = New Point(135, 61)
			Me.label4.Name = "label4"
			Me.label4.Size = New Size(56, 13)
			Me.label4.TabIndex = 8
			Me.label4.Text = "Password:"
			' 
			' txtPassword
			' 
			Me.txtPassword.Location = New Point(196, 56)
			Me.txtPassword.Name = "txtPassword"
			Me.txtPassword.PasswordChar = "#"c
			Me.txtPassword.Size = New Size(109, 20)
			Me.txtPassword.TabIndex = 9
			Me.txtPassword.Text = "PassOne"
			' 
			' btnToDB
			' 
			Me.btnToDB.Enabled = False
			Me.btnToDB.Location = New Point(311, 98)
			Me.btnToDB.Name = "btnToDB"
			Me.btnToDB.Size = New Size(93, 29)
			Me.btnToDB.TabIndex = 10
			Me.btnToDB.Text = "To DB"
			Me.btnToDB.UseVisualStyleBackColor = True
'			Me.btnToDB.Click += New System.EventHandler(Me.btnToDB_Click)
			' 
			' label5
			' 
			Me.label5.AutoSize = True
			Me.label5.Location = New Point(10, 119)
			Me.label5.Name = "label5"
			Me.label5.Size = New Size(80, 13)
			Me.label5.TabIndex = 11
			Me.label5.Text = "SQL statement:"
			' 
			' txtSQL
			' 
			Me.txtSQL.Location = New Point(10, 136)
			Me.txtSQL.Name = "txtSQL"
			Me.txtSQL.Size = New Size(391, 20)
			Me.txtSQL.TabIndex = 12
			Me.txtSQL.Text = "Delete from Shippers Where ShipperID > 3"
			' 
			' btnExecuteSQL
			' 
			Me.btnExecuteSQL.Enabled = False
			Me.btnExecuteSQL.Location = New Point(311, 163)
			Me.btnExecuteSQL.Name = "btnExecuteSQL"
			Me.btnExecuteSQL.Size = New Size(90, 29)
			Me.btnExecuteSQL.TabIndex = 13
			Me.btnExecuteSQL.Text = "Execute SQL"
			Me.btnExecuteSQL.UseVisualStyleBackColor = True
'			Me.btnExecuteSQL.Click += New System.EventHandler(Me.btnExecuteSQL_Click)
			' 
			' frmMySecure
			' 
			Me.AutoScaleDimensions = New SizeF(6F, 13F)
			Me.AutoScaleMode = AutoScaleMode.Font
			Me.ClientSize = New Size(416, 202)
			Me.Controls.Add(Me.btnExecuteSQL)
			Me.Controls.Add(Me.txtSQL)
			Me.Controls.Add(Me.label5)
			Me.Controls.Add(Me.btnToDB)
			Me.Controls.Add(Me.txtPassword)
			Me.Controls.Add(Me.label4)
			Me.Controls.Add(Me.txtUserID)
			Me.Controls.Add(Me.label3)
			Me.Controls.Add(Me.txtPort)
			Me.Controls.Add(Me.label2)
			Me.Controls.Add(Me.label1)
			Me.Controls.Add(Me.txtAddress)
			Me.Controls.Add(Me.btnDisconnect)
			Me.Controls.Add(Me.btnConnect)
			Me.Name = "frmMySecure"
			Me.Text = "My way for securing data communication"
'			Me.Load += New System.EventHandler(Me.frmMySecure_Load)
			Me.ResumeLayout(False)
			Me.PerformLayout()

		End Sub

		#End Region

		Private WithEvents btnConnect As Button
		Private WithEvents btnDisconnect As Button
		Private txtAddress As TextBox
		Private label1 As Label
		Private label2 As Label
		Private txtPort As TextBox
		Private label3 As Label
		Private txtUserID As TextBox
		Private label4 As Label
		Private txtPassword As TextBox
		Private WithEvents btnToDB As Button
		Private label5 As Label
		Private txtSQL As TextBox
		Private WithEvents btnExecuteSQL As Button
	End Class
End Namespace

