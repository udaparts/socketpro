Imports Microsoft.VisualBasic
Imports System
Imports System.Collections.Generic
Imports System.ComponentModel
Imports System.Data
Imports System.Drawing
Imports System.Text
Imports System.Windows.Forms
Imports UDBLib
Imports USOCKETLib
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports SocketProAdapter.ClientSide.RemoteDB

Namespace MyTest
	Public Partial Class frmMyTest
		Inherits Form
		Private m_AsynDBLite As CAsynDBLiteEx
		Private m_ClientSocket As CClientSocket
		Public Sub New()
			InitializeComponent()
		End Sub

		Private Sub btnDoSQL_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnDoSQL.Click
			'diable this button temporarily so that one SQL query is executed only
			btnDoSQL.Enabled = False

			'SubBatchSize reduces latency
			m_AsynDBLite.SubBatchSize = 20

			'Open one rowset and set generated DataTable with the name "Table1"
            m_AsynDBLite.OpenRowset(txtSQL.Text, "Table1", UDBLib.tagCursorType.ctStatic, CAsynDBLite.Scrollable, 100, -1)

			'wait until all of requests are executed
            m_AsynDBLite.GetAttachedClientSocket().WaitAll()
            btnDoSQL.Enabled = True

            'enable or disable buttons according to rowset properties (readonly and scrollable)
            If m_AsynDBLite.IsRowsetOpened Then
                btnFirst.Enabled = True
                btnNextBatch.Enabled = btnFirst.Enabled
                btnAdd.Enabled = ((Not m_AsynDBLite.IsRowsetReadonly))
            Else
                btnAdd.Enabled = False
                btnNextBatch.Enabled = btnAdd.Enabled
                btnFirst.Enabled = btnNextBatch.Enabled
            End If
            btnLast.Enabled = m_AsynDBLite.IsRowsetScrollable
            btnPrev.Enabled = btnLast.Enabled
        End Sub

        Private Sub OnSocketClosed(ByVal hSocket As Integer, ByVal nError As Integer)
            btnDoSQL.Enabled = False
            btnFirst.Enabled = False
            btnNextBatch.Enabled = False
            btnPrev.Enabled = False
            btnLast.Enabled = False
            btnAdd.Enabled = False
            Text = "AsynDBLite Demo"
        End Sub

        Private Sub OnSocketConnected(ByVal hSocket As Integer, ByVal nError As Integer)
            If nError = 0 Then
                Dim lPeerPort As Integer = 0
                Dim lHandle As Integer
                Dim strAlias As String
                btnDoSQL.Enabled = True

                m_ClientSocket.SetUID("SocketPro")
                m_ClientSocket.SetPassword("PassOne")

                'ask for remote DB service
                m_ClientSocket.SwitchTo(m_AsynDBLite)

                'get remote host ip adress and host name
                Dim strIPAddr As String = m_ClientSocket.GetUSocket().GetPeerName(lPeerPort)
                Dim strHostName As String = m_ClientSocket.GetUSocket().GetHostByAddr(strIPAddr, CInt(Fix(USOCKETLib.tagAddressFamily.afINet)), True, lHandle, strAlias)
                Text &= " ("
                Text &= strHostName
                Text &= ":"
                Text &= lPeerPort.ToString()
                Text &= ")"

                '                m_AsynDBLite.ConnectDB("Provider=sqlncli;Data Source=localhost\\sqlexpress;Initial Catalog=northwind;Integrated Security=SSPI");

                'connect to database
                m_AsynDBLite.ConnectDB("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=nwind3.mdb")
            End If
        End Sub

        Private Sub frmMyTest_FormClosed(ByVal sender As Object, ByVal args As FormClosedEventArgs) Handles MyBase.FormClosed

            m_ClientSocket.DestroyUSocket()
        End Sub

        Private Sub frmMyTest_Load(ByVal sender As Object, ByVal e As EventArgs) Handles MyBase.Load
            m_ClientSocket = New CClientSocket()
            m_AsynDBLite = New CAsynDBLiteEx()
            m_AsynDBLite.Attach(m_ClientSocket)

            m_ClientSocket.m_OnSocketClosed = m_ClientSocket.m_OnSocketClosed.Combine(m_ClientSocket.m_OnSocketClosed, New DOnSocketClosed(AddressOf OnSocketClosed))
            m_ClientSocket.m_OnSocketConnected = m_ClientSocket.m_OnSocketConnected.Combine(m_ClientSocket.m_OnSocketConnected, New DOnSocketConnected(AddressOf OnSocketConnected))

            'Bind a DataGridView control with an instance of CAsynDBLite
            m_AsynDBLite.AttachedDataGridView = dgvTable
        End Sub

        Private Sub btnConnect_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnConnect.Click
            'Connect to a remote SocketPro server
            m_ClientSocket.Connect("localhost", 17001)
        End Sub

        Private Sub btnDisconnect_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnDisconnect.Click
            m_ClientSocket.Disconnect()
        End Sub

        Private Sub btnNextBatch_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnNextBatch.Click
            Dim nSkip As Integer = 0
            Try
                nSkip = Integer.Parse(txtSkip.Text)
            Catch myError As Exception
                myError = Nothing
            End Try
            m_AsynDBLite.NextBatch(nSkip)
        End Sub

        Private Sub btnPrev_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnPrev.Click
            m_AsynDBLite.PrevBatch()
        End Sub

        Private Sub btnLast_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnLast.Click
            m_AsynDBLite.LastBatch()
        End Sub

        Private Sub btnFirst_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnFirst.Click
            m_AsynDBLite.FirstBatch()
        End Sub

        Private Sub btnAdd_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnAdd.Click
            'There are many ways to insert rows onto a database within remote database service.
            'Here we add rows in batch using cursor.
            m_AsynDBLite.DBErrors.Clear()
            m_AsynDBLite.GetAttachedClientSocket().BeginBatching()
            For Each row As DataGridViewRow In dgvTable.Rows
                Dim nDataGridViewRowIndex As Integer = row.Cells(0).RowIndex
                Dim nRecordRowIndex As Integer = m_AsynDBLite.GetRecordRowIndex(nDataGridViewRowIndex)
                If nRecordRowIndex >= m_AsynDBLite.RowsFetched Then
                    m_AsynDBLite.AddRecord(row)
                End If
            Next row
            m_AsynDBLite.GetAttachedClientSocket().Commit(True)
        End Sub
	End Class
End Namespace