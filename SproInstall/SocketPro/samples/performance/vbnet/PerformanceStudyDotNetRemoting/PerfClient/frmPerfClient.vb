Imports Microsoft.VisualBasic
Imports System
Imports System.Collections.Generic
Imports System.ComponentModel
Imports System.Data
Imports System.Drawing
Imports System.Text
Imports System.Windows.Forms
Imports SocketProAdapter
Imports DefMyCallsInterface

Namespace PerfClient
	Public Partial Class frmPerfClient
		Inherits Form
        Private m_dt As DataTable
        Private m_PerfQuery As CUPerformanceQuery = New CUPerformanceQuery()
        Private m_MyCalls As IMyCalls

        Public Sub New()
            InitializeComponent()
        End Sub

        Private Sub frmPerfClient_Load(ByVal sender As Object, ByVal e As EventArgs) Handles MyBase.Load

        End Sub

        Private Sub btnConnect_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnConnect.Click
            btnSQL.Enabled = False
            btnMyEcho.Enabled = False
            Dim strHost As String = txtHost.Text
            Dim strDestination As String = strHost & ":" & Int32.Parse(txtNetPort.Text)
            Dim strURL As String = "tcp://" & strDestination & "/MyCalls"
            Try
                m_MyCalls = CType(Activator.GetObject(GetType(DefMyCallsInterface.CMyCallsImpl), strURL), IMyCalls)
            Catch eEx As Exception
                MessageBox.Show(eEx.Message)
                m_MyCalls = Nothing
                Return
            End Try
            btnMyEcho.Enabled = True
            btnSQL.Enabled = True
        End Sub

        Private Sub btnSQL_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnSQL.Click
            Dim n As Integer
            Dim lStart As Long = m_PerfQuery.Now()
            For n = 0 To 99
                Try
                    m_dt = m_MyCalls.OpenRowset(txtSQL.Text)
                Catch myError As Exception
                    MessageBox.Show(myError.Message)
                    Return
                End Try
            Next n
            Dim lDiff As Long = m_PerfQuery.Diff(lStart)
            txtTime.Text = lDiff.ToString()
        End Sub

		Private Sub btnMyEcho_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnMyEcho.Click
			Dim n As Integer
			Dim strEcho As String
			Dim lStart As Long = m_PerfQuery.Now()
			For n = 0 To 9999
				strEcho = m_MyCalls.MyEcho("MyEcho")
			Next n
			Dim lDiff As Long = m_PerfQuery.Diff(lStart)
			txtTime.Text = lDiff.ToString()
		End Sub
	End Class
End Namespace