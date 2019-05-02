Imports Microsoft.VisualBasic
Imports System
Imports System.Collections.Generic
Imports System.ComponentModel
Imports System.Data
Imports System.Drawing
Imports System.Text
Imports System.Windows.Forms
Imports System.ServiceModel
Imports SocketProAdapter

Namespace PerfClient
	Public Partial Class frmPerfClient
		Inherits Form
		Private m_PerfQuery As CUPerformanceQuery = New CUPerformanceQuery()
        Private m_dt As DataTable

        Public Sub New()
            InitializeComponent()
        End Sub

        Private Sub frmPerfClient_Load(ByVal sender As Object, ByVal e As EventArgs) Handles MyBase.Load

        End Sub

        Private Sub btnSQL_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnSQL.Click
            Dim n As Integer
            Dim m_myCalls As MyCallsClient = New MyCallsClient()
            Dim lStart As Long = m_PerfQuery.Now()
            For n = 0 To 99
                m_dt = m_myCalls.OpenRowset(txtSQL.Text)
            Next n
            Dim lDiff As Long = m_PerfQuery.Diff(lStart)
            txtTime.Text = lDiff.ToString()
        End Sub

		Private Sub btnMyEcho_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnMyEcho.Click
			Dim n As Integer
			Dim str As String
			Dim m_myCalls As MyCallsClient = New MyCallsClient()
			Dim lStart As Long = m_PerfQuery.Now()
			For n = 0 To 9999
				str = m_myCalls.MyEcho("MyEcho")
			Next n
			Dim lDiff As Long = m_PerfQuery.Diff(lStart)
			txtTime.Text = lDiff.ToString()
		End Sub
	End Class
End Namespace