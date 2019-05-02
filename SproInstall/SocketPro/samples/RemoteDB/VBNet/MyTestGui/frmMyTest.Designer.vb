Imports Microsoft.VisualBasic
Imports System
Namespace MyTest
	Public Partial Class frmMyTest
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
			Me.btnDoSQL = New System.Windows.Forms.Button()
			Me.dgvTable = New System.Windows.Forms.DataGridView()
			Me.txtSQL = New System.Windows.Forms.TextBox()
			Me.btnConnect = New System.Windows.Forms.Button()
			Me.btnDisconnect = New System.Windows.Forms.Button()
			Me.btnNextBatch = New System.Windows.Forms.Button()
			Me.btnPrev = New System.Windows.Forms.Button()
			Me.btnLast = New System.Windows.Forms.Button()
			Me.btnFirst = New System.Windows.Forms.Button()
			Me.txtSkip = New System.Windows.Forms.TextBox()
			Me.btnAdd = New System.Windows.Forms.Button()
			CType(Me.dgvTable, System.ComponentModel.ISupportInitialize).BeginInit()
			Me.SuspendLayout()
			' 
			' btnDoSQL
			' 
			Me.btnDoSQL.Enabled = False
			Me.btnDoSQL.Location = New System.Drawing.Point(194, 39)
			Me.btnDoSQL.Name = "btnDoSQL"
			Me.btnDoSQL.Size = New System.Drawing.Size(103, 23)
			Me.btnDoSQL.TabIndex = 0
			Me.btnDoSQL.Text = "Execute SQL"
			Me.btnDoSQL.UseVisualStyleBackColor = True
'			Me.btnDoSQL.Click += New System.EventHandler(Me.btnDoSQL_Click);
			' 
			' dgvTable
			' 
			Me.dgvTable.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize
			Me.dgvTable.Location = New System.Drawing.Point(12, 68)
			Me.dgvTable.Name = "dgvTable"
			Me.dgvTable.Size = New System.Drawing.Size(622, 370)
			Me.dgvTable.TabIndex = 1
			' 
			' txtSQL
			' 
			Me.txtSQL.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, (CByte(0)))
			Me.txtSQL.Location = New System.Drawing.Point(13, 13)
			Me.txtSQL.Name = "txtSQL"
			Me.txtSQL.Size = New System.Drawing.Size(621, 20)
			Me.txtSQL.TabIndex = 2
			Me.txtSQL.Text = "Select * from Orders"
			' 
			' btnConnect
			' 
			Me.btnConnect.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, (CByte(0)))
			Me.btnConnect.Location = New System.Drawing.Point(13, 39)
			Me.btnConnect.Name = "btnConnect"
			Me.btnConnect.Size = New System.Drawing.Size(93, 23)
			Me.btnConnect.TabIndex = 3
			Me.btnConnect.Text = "Connect"
			Me.btnConnect.UseVisualStyleBackColor = True
'			Me.btnConnect.Click += New System.EventHandler(Me.btnConnect_Click);
			' 
			' btnDisconnect
			' 
			Me.btnDisconnect.Location = New System.Drawing.Point(113, 39)
			Me.btnDisconnect.Name = "btnDisconnect"
			Me.btnDisconnect.Size = New System.Drawing.Size(75, 23)
			Me.btnDisconnect.TabIndex = 4
			Me.btnDisconnect.Text = "Disconnect"
			Me.btnDisconnect.UseVisualStyleBackColor = True
'			Me.btnDisconnect.Click += New System.EventHandler(Me.btnDisconnect_Click);
			' 
			' btnNextBatch
			' 
			Me.btnNextBatch.Enabled = False
			Me.btnNextBatch.Location = New System.Drawing.Point(367, 39)
			Me.btnNextBatch.Name = "btnNextBatch"
			Me.btnNextBatch.Size = New System.Drawing.Size(31, 23)
			Me.btnNextBatch.TabIndex = 5
			Me.btnNextBatch.Text = ">"
			Me.btnNextBatch.UseVisualStyleBackColor = True
'			Me.btnNextBatch.Click += New System.EventHandler(Me.btnNextBatch_Click);
			' 
			' btnPrev
			' 
			Me.btnPrev.Enabled = False
			Me.btnPrev.Location = New System.Drawing.Point(454, 39)
			Me.btnPrev.Name = "btnPrev"
			Me.btnPrev.Size = New System.Drawing.Size(31, 23)
			Me.btnPrev.TabIndex = 6
			Me.btnPrev.Text = "<"
			Me.btnPrev.UseVisualStyleBackColor = True
'			Me.btnPrev.Click += New System.EventHandler(Me.btnPrev_Click);
			' 
			' btnLast
			' 
			Me.btnLast.Enabled = False
			Me.btnLast.Location = New System.Drawing.Point(491, 39)
			Me.btnLast.Name = "btnLast"
			Me.btnLast.Size = New System.Drawing.Size(31, 23)
			Me.btnLast.TabIndex = 7
			Me.btnLast.Text = ">>|"
			Me.btnLast.UseVisualStyleBackColor = True
'			Me.btnLast.Click += New System.EventHandler(Me.btnLast_Click);
			' 
			' btnFirst
			' 
			Me.btnFirst.Enabled = False
			Me.btnFirst.Location = New System.Drawing.Point(330, 39)
			Me.btnFirst.Name = "btnFirst"
			Me.btnFirst.Size = New System.Drawing.Size(31, 23)
			Me.btnFirst.TabIndex = 8
			Me.btnFirst.Text = "|<<"
			Me.btnFirst.UseVisualStyleBackColor = True
'			Me.btnFirst.Click += New System.EventHandler(Me.btnFirst_Click);
			' 
			' txtSkip
			' 
			Me.txtSkip.Location = New System.Drawing.Point(405, 42)
			Me.txtSkip.Name = "txtSkip"
			Me.txtSkip.Size = New System.Drawing.Size(43, 20)
			Me.txtSkip.TabIndex = 9
			Me.txtSkip.Text = "0"
			Me.txtSkip.TextAlign = System.Windows.Forms.HorizontalAlignment.Center
			' 
			' btnAdd
			' 
			Me.btnAdd.Enabled = False
			Me.btnAdd.Location = New System.Drawing.Point(539, 39)
			Me.btnAdd.Name = "btnAdd"
			Me.btnAdd.Size = New System.Drawing.Size(94, 23)
			Me.btnAdd.TabIndex = 10
			Me.btnAdd.Text = "Add Records"
			Me.btnAdd.UseVisualStyleBackColor = True
'			Me.btnAdd.Click += New System.EventHandler(Me.btnAdd_Click);
			' 
			' frmMyTest
			' 
			Me.AutoScaleDimensions = New System.Drawing.SizeF(6F, 13F)
			Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
			Me.ClientSize = New System.Drawing.Size(646, 450)
			Me.Controls.Add(Me.btnAdd)
			Me.Controls.Add(Me.txtSkip)
			Me.Controls.Add(Me.btnFirst)
			Me.Controls.Add(Me.btnLast)
			Me.Controls.Add(Me.btnPrev)
			Me.Controls.Add(Me.btnNextBatch)
			Me.Controls.Add(Me.btnDisconnect)
			Me.Controls.Add(Me.btnConnect)
			Me.Controls.Add(Me.txtSQL)
			Me.Controls.Add(Me.dgvTable)
			Me.Controls.Add(Me.btnDoSQL)
			Me.Name = "frmMyTest"
			Me.Text = "AsynDBLite Demo"
'			Me.FormClosed += New System.Windows.Forms.FormClosedEventHandler(Me.frmMyTest_FormClosed);
'			Me.Load += New System.EventHandler(Me.frmMyTest_Load);
			CType(Me.dgvTable, System.ComponentModel.ISupportInitialize).EndInit()
			Me.ResumeLayout(False)
			Me.PerformLayout()

		End Sub

		#End Region

		Private WithEvents btnDoSQL As System.Windows.Forms.Button
		Private dgvTable As System.Windows.Forms.DataGridView
		Private txtSQL As System.Windows.Forms.TextBox
		Private WithEvents btnConnect As System.Windows.Forms.Button
		Private WithEvents btnDisconnect As System.Windows.Forms.Button
		Private WithEvents btnNextBatch As System.Windows.Forms.Button
		Private WithEvents btnPrev As System.Windows.Forms.Button
		Private WithEvents btnLast As System.Windows.Forms.Button
		Private WithEvents btnFirst As System.Windows.Forms.Button
		Private txtSkip As System.Windows.Forms.TextBox
		Private WithEvents btnAdd As System.Windows.Forms.Button
	End Class
End Namespace

