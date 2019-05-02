Imports Microsoft.VisualBasic
Imports System
Namespace ParameterizedSQL
	Public Partial Class frmPSQL
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
			Me.btnTestPSQL = New System.Windows.Forms.Button()
			Me.btnConnect = New System.Windows.Forms.Button()
			Me.btnDisconnect = New System.Windows.Forms.Button()
			Me.dgvRowset = New System.Windows.Forms.DataGridView()
			CType(Me.dgvRowset, System.ComponentModel.ISupportInitialize).BeginInit()
			Me.SuspendLayout()
			' 
			' btnTestPSQL
			' 
			Me.btnTestPSQL.Enabled = False
			Me.btnTestPSQL.Location = New System.Drawing.Point(259, 12)
			Me.btnTestPSQL.Name = "btnTestPSQL"
			Me.btnTestPSQL.Size = New System.Drawing.Size(240, 33)
			Me.btnTestPSQL.TabIndex = 0
			Me.btnTestPSQL.Text = "Test PSQL"
			Me.btnTestPSQL.UseVisualStyleBackColor = True
'			Me.btnTestPSQL.Click += New System.EventHandler(Me.btnTestPSQL_Click);
			' 
			' btnConnect
			' 
			Me.btnConnect.Location = New System.Drawing.Point(12, 12)
			Me.btnConnect.Name = "btnConnect"
			Me.btnConnect.Size = New System.Drawing.Size(111, 31)
			Me.btnConnect.TabIndex = 1
			Me.btnConnect.Text = "Connect"
			Me.btnConnect.UseVisualStyleBackColor = True
'			Me.btnConnect.Click += New System.EventHandler(Me.btnConnect_Click);
			' 
			' btnDisconnect
			' 
			Me.btnDisconnect.Location = New System.Drawing.Point(129, 12)
			Me.btnDisconnect.Name = "btnDisconnect"
			Me.btnDisconnect.Size = New System.Drawing.Size(106, 31)
			Me.btnDisconnect.TabIndex = 2
			Me.btnDisconnect.Text = "Disconnect"
			Me.btnDisconnect.UseVisualStyleBackColor = True
'			Me.btnDisconnect.Click += New System.EventHandler(Me.btnDisconnect_Click);
			' 
			' dgvRowset
			' 
			Me.dgvRowset.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize
			Me.dgvRowset.Location = New System.Drawing.Point(12, 51)
			Me.dgvRowset.Name = "dgvRowset"
			Me.dgvRowset.Size = New System.Drawing.Size(487, 256)
			Me.dgvRowset.TabIndex = 3
			' 
			' frmPSQL
			' 
			Me.AutoScaleDimensions = New System.Drawing.SizeF(6F, 13F)
			Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
			Me.ClientSize = New System.Drawing.Size(511, 319)
			Me.Controls.Add(Me.dgvRowset)
			Me.Controls.Add(Me.btnDisconnect)
			Me.Controls.Add(Me.btnConnect)
			Me.Controls.Add(Me.btnTestPSQL)
			Me.Name = "frmPSQL"
			Me.Text = "Demo to PSQL"
'			Me.Load += New System.EventHandler(Me.frmPSQL_Load);
			CType(Me.dgvRowset, System.ComponentModel.ISupportInitialize).EndInit()
			Me.ResumeLayout(False)

		End Sub

		#End Region

		Private WithEvents btnTestPSQL As System.Windows.Forms.Button
		Private WithEvents btnConnect As System.Windows.Forms.Button
		Private WithEvents btnDisconnect As System.Windows.Forms.Button
		Private dgvRowset As System.Windows.Forms.DataGridView
	End Class
End Namespace

