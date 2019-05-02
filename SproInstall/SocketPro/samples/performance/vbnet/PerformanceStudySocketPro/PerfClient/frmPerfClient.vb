Imports System.ComponentModel
Imports System.Text

Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports SocketProAdapter.ClientSide.RemoteDB
Imports USOCKETLib


Namespace PerfClient
	Partial Public Class frmPerfClient
		Inherits Form
		Private m_SocketPool As New CSocketPool(Of CPerf)()
        Private m_Perf As CPerf = New CPerf()
		Private m_PerfQuery As New CUPerformanceQuery()
		Private m_lStart As Long
		Private m_nIndex As Integer

		Public Sub New()
			InitializeComponent()
		End Sub

		Private Delegate Sub DOnClosed()
		Private Delegate Sub DOnEchoProcessed()

		Private Sub OnEchoProcessed()
			Dim lDiff As Long = m_PerfQuery.Diff(m_lStart)
			txtTime.Text = lDiff.ToString()
		End Sub

		Private Sub OnRequestProcessed(ByVal hSocket As Integer, ByVal sRequestID As Short, ByVal nLen As Integer, ByVal nLenInBuffer As Integer, ByVal ReturnFlag As USOCKETLib.tagReturnFlag)
			Select Case sRequestID
				Case PerfConst.idMyEchoCPerf
					m_nIndex += 1
					If m_nIndex < 10000 Then
                        m_Perf.MyEchoAsync("TestData")
					Else
						BeginInvoke(New DOnEchoProcessed(AddressOf OnEchoProcessed))
					End If
				Case Else
			End Select
		End Sub

		Private Overloads Sub OnClosed()
			btnEcho.Enabled = False
			btnSQL.Enabled = False
		End Sub

		Private Sub OnSocketClosed(ByVal hSocket As Integer, ByVal nError As Integer)
			BeginInvoke(New DOnClosed(AddressOf OnClosed))
		End Sub

		Private Sub frmPerfClient_Closing(ByVal sender As Object, ByVal e As FormClosingEventArgs) Handles MyBase.FormClosing
			m_SocketPool.ShutdownPool()
		End Sub

		Private Sub frmPerfClient_Load(ByVal sender As Object, ByVal e As EventArgs) Handles MyBase.Load

		End Sub

		Private Sub btnConnect_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnConnect.Click
            If m_SocketPool.StartSocketPool(txtHost.Text, Integer.Parse(txtPort.Text), "SocketPro", "PassOne", 1, 1, USOCKETLib.tagEncryptionMethod.NoEncryption, chkZip.Checked) Then
                m_Perf = m_SocketPool.Lock()
                m_Perf.GetAttachedClientSocket().m_OnSocketClosed = AddressOf OnSocketClosed
                If Not chkBatch.Checked Then
                    m_Perf.GetAttachedClientSocket().m_OnRequestProcessed = AddressOf OnRequestProcessed
                End If
                m_SocketPool.Unlock(m_Perf)
                m_Perf.GetAttachedClientSocket().GetUSocket().TurnOnZipAtSvr(chkZip.Checked)
                m_Perf.GetAttachedClientSocket().GetUSocket().ZipIsOn = chkZip.Checked
                If radioDefault.Checked Then
                    m_Perf.GetAttachedClientSocket().GetUSocket().SetZipLevelAtSvr(USOCKETLib.tagZipLevel.zlDefault)
                    m_Perf.GetAttachedClientSocket().GetUSocket().zipLevel = USOCKETLib.tagZipLevel.zlDefault
                Else
                    m_Perf.GetAttachedClientSocket().GetUSocket().SetZipLevelAtSvr(USOCKETLib.tagZipLevel.zlBestSpeed)
                    m_Perf.GetAttachedClientSocket().GetUSocket().zipLevel = USOCKETLib.tagZipLevel.zlBestSpeed
                End If
                btnEcho.Enabled = True
                btnSQL.Enabled = True
            End If
		End Sub

		Private Sub btnDisconnect_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnDisconnect.Click
			m_Perf.Detach()
			m_SocketPool.ShutdownPool()
		End Sub

		Private Sub chkBatch_CheckedChanged(ByVal sender As Object, ByVal e As EventArgs) Handles chkBatch.CheckedChanged
            If m_Perf Is Nothing Or m_Perf.GetAttachedClientSocket() Is Nothing Then
                Return
            End If
			If Not m_Perf.GetAttachedClientSocket().IsConnected() Then
				Return
			End If
            If chkBatch.Checked Then
                'remove unwanted events
                m_Perf.GetAttachedClientSocket().m_OnRequestProcessed = Nothing
            Else
                m_Perf.GetAttachedClientSocket().m_OnRequestProcessed = AddressOf OnRequestProcessed
            End If
        End Sub

		Private Sub btnEcho_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnEcho.Click
			m_lStart = m_PerfQuery.Now()
			If chkBatch.Checked Then
				Dim n As Integer
				Dim j As Integer
				For n = 0 To 39
                    m_Perf.BeginBatching()
					For j = 0 To 249
                        m_Perf.MyEchoAsync("TestData")
					Next j
                    m_Perf.CommitBatch(True)
                    m_Perf.WaitAll()
				Next n
				Dim lDiff As Long = m_PerfQuery.Diff(m_lStart)
				txtTime.Text = lDiff.ToString()
			Else
				m_nIndex = 0
				m_Perf.MyEcho("TestData")
			End If
		End Sub

		Private Sub btnSQL_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnSQL.Click
			Dim n As Integer
			Dim dt As DataTable
			m_lStart = m_PerfQuery.Now()
			If chkBatch.Checked Then
				For n = 0 To 9
					Dim j As Integer
                    m_Perf.BeginBatching()
					For j = 0 To 9
                        m_Perf.OpenRecordsAsync(txtSQL.Text)
					Next j
                    m_Perf.CommitBatch(True)
                    m_Perf.WaitAll()
					dt = m_Perf.CurrentDataTable
				Next n
			Else
				For n = 0 To 99
					dt = m_Perf.OpenRecords(txtSQL.Text)
				Next n
			End If
			Dim lDiff As Long = m_PerfQuery.Diff(m_lStart)
			txtTime.Text = lDiff.ToString()
		End Sub

		Private Sub radioRealtime_CheckedChanged(ByVal sender As Object, ByVal e As EventArgs) Handles radioRealtime.CheckedChanged
			If (Not m_SocketPool.IsStarted()) OrElse (Not m_Perf.GetAttachedClientSocket().IsConnected()) Then
				Return
			End If
            m_Perf.GetAttachedClientSocket().GetUSocket().SetZipLevelAtSvr(USOCKETLib.tagZipLevel.zlBestSpeed)
            m_Perf.GetAttachedClientSocket().GetUSocket().zipLevel = USOCKETLib.tagZipLevel.zlBestSpeed
		End Sub

		Private Sub radioDefault_CheckedChanged(ByVal sender As Object, ByVal e As EventArgs) Handles radioDefault.CheckedChanged
			If (Not m_SocketPool.IsStarted()) OrElse (Not m_Perf.GetAttachedClientSocket().IsConnected()) Then
				Return
			End If
            m_Perf.GetAttachedClientSocket().GetUSocket().SetZipLevelAtSvr(USOCKETLib.tagZipLevel.zlDefault)
            m_Perf.GetAttachedClientSocket().GetUSocket().zipLevel = USOCKETLib.tagZipLevel.zlDefault
		End Sub

		Private Sub chkZip_CheckedChanged(ByVal sender As Object, ByVal e As EventArgs) Handles chkZip.CheckedChanged
			If (Not m_SocketPool.IsStarted()) OrElse (Not m_Perf.GetAttachedClientSocket().IsConnected()) Then
				Return
			End If
			m_Perf.GetAttachedClientSocket().GetUSocket().TurnOnZipAtSvr(chkZip.Checked)
			m_Perf.GetAttachedClientSocket().GetUSocket().ZipIsOn = chkZip.Checked
		End Sub
	End Class
End Namespace