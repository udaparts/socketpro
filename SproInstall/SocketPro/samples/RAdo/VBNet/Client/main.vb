'#define USE_SQLCLIENT

#If USE_SQLCLIENT Then
Imports System.Data.SqlClient
#Else
Imports System.Data.OleDb
#End If

Imports System.Collections
Imports System.ComponentModel
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports USOCKETLib

Namespace RAdo
	''' <summary>
	''' Summary description for Form1.
	''' </summary>
	Public Class frmRemotingAdoNet
		Inherits Form
		Private WithEvents btnConnect As Button
		Private WithEvents btnDisconnect As Button
		Private label1 As Label
		Private txtHost As TextBox
		Private label2 As Label
		Private txtPort As TextBox
		Private chkSSL As CheckBox
		Private chkZip As CheckBox
		''' <summary>
		''' Required designer variable.
		''' </summary>
		Private components As System.ComponentModel.Container = Nothing

		Private m_ClientSocket As New CClientSocket()
        Private WithEvents btnGetDR As Button
		Private WithEvents btnGetDS As Button
		Private WithEvents btnSendDR As Button
        Private WithEvents btnSendDS As Button
        Friend WithEvents dgTable As System.Windows.Forms.DataGridView
		Private m_RAdo As New CRAdo()

		Public Sub New()
			'
			' Required for Windows Form Designer support
			'
			InitializeComponent()

			'
			' TODO: Add any constructor code after InitializeComponent call
			'
		End Sub

		''' <summary>
		''' Clean up any resources being used.
		''' </summary>
		Protected Overrides Overloads Sub Dispose(ByVal disposing As Boolean)
			If disposing Then
				If components IsNot Nothing Then
					components.Dispose()
				End If
			End If
			MyBase.Dispose(disposing)
		End Sub

		#Region "Windows Form Designer generated code"
		''' <summary>
		''' Required method for Designer support - do not modify
		''' the contents of this method with the code editor.
		''' </summary>
		Private Sub InitializeComponent()
            Me.btnConnect = New System.Windows.Forms.Button
            Me.btnDisconnect = New System.Windows.Forms.Button
            Me.label1 = New System.Windows.Forms.Label
            Me.txtHost = New System.Windows.Forms.TextBox
            Me.label2 = New System.Windows.Forms.Label
            Me.txtPort = New System.Windows.Forms.TextBox
            Me.chkSSL = New System.Windows.Forms.CheckBox
            Me.chkZip = New System.Windows.Forms.CheckBox
            Me.btnGetDR = New System.Windows.Forms.Button
            Me.btnGetDS = New System.Windows.Forms.Button
            Me.btnSendDR = New System.Windows.Forms.Button
            Me.btnSendDS = New System.Windows.Forms.Button
            Me.dgTable = New System.Windows.Forms.DataGridView
            CType(Me.dgTable, System.ComponentModel.ISupportInitialize).BeginInit()
            Me.SuspendLayout()
            '
            'btnConnect
            '
            Me.btnConnect.Location = New System.Drawing.Point(352, 0)
            Me.btnConnect.Name = "btnConnect"
            Me.btnConnect.Size = New System.Drawing.Size(88, 24)
            Me.btnConnect.TabIndex = 0
            Me.btnConnect.Text = "Connect"
            '
            'btnDisconnect
            '
            Me.btnDisconnect.Location = New System.Drawing.Point(352, 32)
            Me.btnDisconnect.Name = "btnDisconnect"
            Me.btnDisconnect.Size = New System.Drawing.Size(88, 23)
            Me.btnDisconnect.TabIndex = 1
            Me.btnDisconnect.Text = "Disconnect"
            '
            'label1
            '
            Me.label1.Location = New System.Drawing.Point(16, 8)
            Me.label1.Name = "label1"
            Me.label1.Size = New System.Drawing.Size(100, 24)
            Me.label1.TabIndex = 2
            Me.label1.Text = "Remote Host:"
            '
            'txtHost
            '
            Me.txtHost.Location = New System.Drawing.Point(16, 32)
            Me.txtHost.Name = "txtHost"
            Me.txtHost.Size = New System.Drawing.Size(160, 20)
            Me.txtHost.TabIndex = 3
            Me.txtHost.Text = "localhost"
            '
            'label2
            '
            Me.label2.Location = New System.Drawing.Point(192, 8)
            Me.label2.Name = "label2"
            Me.label2.Size = New System.Drawing.Size(56, 24)
            Me.label2.TabIndex = 4
            Me.label2.Text = "Port #:"
            '
            'txtPort
            '
            Me.txtPort.Location = New System.Drawing.Point(192, 32)
            Me.txtPort.Name = "txtPort"
            Me.txtPort.Size = New System.Drawing.Size(56, 20)
            Me.txtPort.TabIndex = 5
            Me.txtPort.Text = "20901"
            '
            'chkSSL
            '
            Me.chkSSL.Location = New System.Drawing.Point(272, 8)
            Me.chkSSL.Name = "chkSSL"
            Me.chkSSL.Size = New System.Drawing.Size(56, 24)
            Me.chkSSL.TabIndex = 6
            Me.chkSSL.Text = "SSL"
            '
            'chkZip
            '
            Me.chkZip.Location = New System.Drawing.Point(272, 32)
            Me.chkZip.Name = "chkZip"
            Me.chkZip.Size = New System.Drawing.Size(56, 24)
            Me.chkZip.TabIndex = 7
            Me.chkZip.Text = "Zip"
            '
            'btnGetDR
            '
            Me.btnGetDR.Enabled = False
            Me.btnGetDR.Location = New System.Drawing.Point(16, 64)
            Me.btnGetDR.Name = "btnGetDR"
            Me.btnGetDR.Size = New System.Drawing.Size(96, 23)
            Me.btnGetDR.TabIndex = 9
            Me.btnGetDR.Text = "Get DataReader"
            '
            'btnGetDS
            '
            Me.btnGetDS.Enabled = False
            Me.btnGetDS.Location = New System.Drawing.Point(128, 64)
            Me.btnGetDS.Name = "btnGetDS"
            Me.btnGetDS.Size = New System.Drawing.Size(88, 23)
            Me.btnGetDS.TabIndex = 10
            Me.btnGetDS.Text = "Get DataSet"
            '
            'btnSendDR
            '
            Me.btnSendDR.Enabled = False
            Me.btnSendDR.Location = New System.Drawing.Point(232, 64)
            Me.btnSendDR.Name = "btnSendDR"
            Me.btnSendDR.Size = New System.Drawing.Size(104, 23)
            Me.btnSendDR.TabIndex = 11
            Me.btnSendDR.Text = "Send DataReader"
            '
            'btnSendDS
            '
            Me.btnSendDS.Enabled = False
            Me.btnSendDS.Location = New System.Drawing.Point(352, 64)
            Me.btnSendDS.Name = "btnSendDS"
            Me.btnSendDS.Size = New System.Drawing.Size(88, 23)
            Me.btnSendDS.TabIndex = 12
            Me.btnSendDS.Text = "Send DataSet"
            '
            'dgTable
            '
            Me.dgTable.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize
            Me.dgTable.Location = New System.Drawing.Point(19, 94)
            Me.dgTable.Name = "dgTable"
            Me.dgTable.Size = New System.Drawing.Size(421, 242)
            Me.dgTable.TabIndex = 13
            '
            'frmRemotingAdoNet
            '
            Me.AutoScaleBaseSize = New System.Drawing.Size(5, 13)
            Me.ClientSize = New System.Drawing.Size(456, 348)
            Me.Controls.Add(Me.dgTable)
            Me.Controls.Add(Me.btnSendDS)
            Me.Controls.Add(Me.btnSendDR)
            Me.Controls.Add(Me.btnGetDS)
            Me.Controls.Add(Me.btnGetDR)
            Me.Controls.Add(Me.chkZip)
            Me.Controls.Add(Me.chkSSL)
            Me.Controls.Add(Me.txtPort)
            Me.Controls.Add(Me.txtHost)
            Me.Controls.Add(Me.label2)
            Me.Controls.Add(Me.label1)
            Me.Controls.Add(Me.btnDisconnect)
            Me.Controls.Add(Me.btnConnect)
            Me.Name = "frmRemotingAdoNet"
            Me.Text = "Bi-directional Remoting ADO.NET Objects"
            CType(Me.dgTable, System.ComponentModel.ISupportInitialize).EndInit()
            Me.ResumeLayout(False)
            Me.PerformLayout()

        End Sub
		#End Region

		''' <summary>
		''' The main entry point for the application.
		''' </summary>
        <STAThread()> _
  Shared Sub Main()
            Application.Run(New frmRemotingAdoNet())
        End Sub

		Private Sub OnSocketClosed(ByVal hSocket As Integer, ByVal nError As Integer)
			btnGetDS.Enabled = False
			btnGetDR.Enabled = False
			btnSendDS.Enabled = False
			btnSendDR.Enabled = False
			If nError <> 0 Then
				MessageBox.Show(m_ClientSocket.GetErrorMsg())
			End If
		End Sub

		Private Sub OnSocketConnected(ByVal hSokcet As Integer, ByVal nError As Integer)
			If nError = 0 Then
				btnGetDS.Enabled = True
				btnGetDR.Enabled = True
				btnSendDS.Enabled = True
				btnSendDR.Enabled = True

				m_ClientSocket.SetUID("SocketPro")
				m_ClientSocket.SetPassword("PassOne")
				m_ClientSocket.BeginBatching()
				m_ClientSocket.SwitchTo(m_RAdo, True)
				If chkZip.Checked Then
					m_ClientSocket.GetUSocket().TurnOnZipAtSvr(True)
				End If

				'Incerasing TCP sending and receiving buffer sizes will help performance
				'when there is a lot of data transferred if your network bandwidth is over 10 mbps
                m_ClientSocket.GetUSocket().SetSockOpt(CInt(USOCKETLib.tagSocketOption.soSndBuf), 116800, CInt(USOCKETLib.tagSocketLevel.slSocket))
                m_ClientSocket.GetUSocket().SetSockOpt(CInt(USOCKETLib.tagSocketOption.soRcvBuf), 116800, CInt(USOCKETLib.tagSocketLevel.slSocket))
                m_ClientSocket.GetUSocket().SetSockOptAtSvr(CInt(USOCKETLib.tagSocketOption.soSndBuf), 116800, CInt(USOCKETLib.tagSocketLevel.slSocket))
                m_ClientSocket.GetUSocket().SetSockOptAtSvr(CInt(USOCKETLib.tagSocketOption.soRcvBuf), 116800, CInt(USOCKETLib.tagSocketLevel.slSocket))

				'turn off Nagel algorithm for both sides
                m_ClientSocket.GetUSocket().SetSockOpt(CInt(USOCKETLib.tagSocketOption.soTcpNoDelay), 1, CInt(USOCKETLib.tagSocketLevel.slTcp))
                m_ClientSocket.GetUSocket().SetSockOptAtSvr(CInt(USOCKETLib.tagSocketOption.soTcpNoDelay), 1, CInt(USOCKETLib.tagSocketLevel.slTcp))
				m_ClientSocket.Commit(False) 'must be false for the first switch
			End If
		End Sub
		Private Sub OnRequestProcessed(ByVal hSocket As Integer, ByVal sRequestID As Short, ByVal nLen As Integer, ByVal nLenInBuffer As Integer, ByVal ReturnFlag As USOCKETLib.tagReturnFlag)
			If ReturnFlag <> USOCKETLib.tagReturnFlag.rfCompleted Then
				Return
			End If
			Select Case sRequestID
                Case CShort(CAsyncAdoSerializationHelper.idDataReaderRecordsArrive), CShort(CAsyncAdoSerializationHelper.idDataTableRowsArrive)
                    If Not m_bUpdate Then
                        m_bUpdate = True
                        'show the first batch of records for fast response
                        dgTable.DataSource = m_RAdo.CurrentDataTable.Copy()
                        dgTable.Update()
                    End If
                Case Else
            End Select
		End Sub
		Private Sub frmRemotingAdoNet_Load(ByVal sender As Object, ByVal e As EventArgs) Handles MyBase.Load
			m_RAdo.Attach(m_ClientSocket)
            m_ClientSocket.m_OnSocketClosed = AddressOf OnSocketClosed
            m_ClientSocket.m_OnSocketConnected = AddressOf OnSocketConnected
            m_ClientSocket.m_OnRequestProcessed = AddressOf OnRequestProcessed
		End Sub

		Private Sub btnConnect_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnConnect.Click
			If chkSSL.Checked Then
				m_ClientSocket.EncryptionMethod = USOCKETLib.tagEncryptionMethod.MSTLSv1
			Else
				m_ClientSocket.EncryptionMethod = USOCKETLib.tagEncryptionMethod.NoEncryption
			End If
			m_ClientSocket.Connect(txtHost.Text, Integer.Parse(txtPort.Text))
		End Sub

		Private Sub btnDisconnect_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnDisconnect.Click
			m_ClientSocket.Disconnect()
		End Sub

        Private m_bUpdate As Boolean = False
		Private Sub btnGetDR_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnGetDR.Click
            m_bUpdate = False
            Dim dt As DataTable = m_RAdo.GetDataReader("Select * from Products, Orders")

            'You can use the below code to display result if data record set size is not large.
            'You should consider latency and use delegate OnRequestProcessed instead if either record set is large or network bandwith is small.

            dgTable.DataSource = dt
        End Sub

		Private Sub btnGetDS_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnGetDS.Click
            m_bUpdate = False
            Dim ds As DataSet = m_RAdo.GetDataSet("Select * from employees", "Select * from Customers")
            'You can use the below code to display result if data record set size is not large.
            'You should consider latency and use delegate OnRequestProcessed instead if either record set is large or network bandwith is small.
            If Not (ds Is Nothing) And ds.Tables.Count > 1 Then
                dgTable.DataSource = ds.Tables(1)
            End If
        End Sub

		Private Sub btnSendDR_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnSendDR.Click
			Dim strSQL As String = "Select * from Customers"
			Dim dr As IDataReader = Nothing
#If USE_SQLCLIENT Then
		Dim conn As New SqlConnection("server=localhost\sqlexpress;Integrated Security=SSPI;database=northwind")
#Else
            Dim conn As New OleDbConnection("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=c:\Program Files\udaparts\SocketPro\bin\nwind3.mdb")
#End If
			Try
				conn.Open()
			Catch err As Exception
				Console.WriteLine(err.Message)
				Return
			End Try

#If USE_SQLCLIENT Then
		Dim cmd As New SqlCommand(strSQL, conn)
#Else
			Dim cmd As New OleDbCommand(strSQL, conn)
#End If
			Try
				dr = cmd.ExecuteReader()
			Catch err As Exception
				Console.WriteLine(err.Message)
				conn.Close()
				Return
			End Try
            Dim ok As Boolean = m_RAdo.SendDataReader(dr)
            conn.Close()
        End Sub

        Private Sub btnSendDS_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnSendDS.Click
            Dim strSQL0 As String = "Select * from Shippers"
            Dim strSQL1 As String = "Select * from Products"

            Dim ds As New DataSet("MyDataSet")
#If USE_SQLCLIENT Then
		Dim conn As New SqlConnection("server=localhost\sqlexpress;Integrated Security=SSPI;database=northwind")
#Else
            Dim conn As New OleDbConnection("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=c:\Program Files\udaparts\SocketPro\bin\nwind3.mdb")
#End If
            Try
                conn.Open()
            Catch err As Exception
                Console.WriteLine(err.Message)
                Return
            End Try
#If USE_SQLCLIENT Then
		Dim cmd As New SqlCommand(strSQL0, conn)
		Dim adapter As New SqlDataAdapter(cmd)
		Dim cmd1 As New SqlCommand(strSQL1, conn)
		Dim adapter1 As New SqlDataAdapter(cmd1)
#Else
            Dim cmd As New OleDbCommand(strSQL0, conn)
            Dim adapter As New OleDbDataAdapter(cmd)
            Dim cmd1 As New OleDbCommand(strSQL1, conn)
            Dim adapter1 As New OleDbDataAdapter(cmd1)
#End If
            Try
                adapter.Fill(ds, "Table1")
                adapter1.Fill(ds, "Table2")
            Catch err As Exception
                Console.WriteLine(err.Message)
                conn.Close()
                Return
            End Try
            Dim ok As Boolean = m_RAdo.SendDataSet(ds)
            conn.Close()
        End Sub
	End Class
End Namespace
