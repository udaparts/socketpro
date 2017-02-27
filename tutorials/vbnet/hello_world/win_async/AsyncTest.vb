Imports System
Imports System.Collections.Generic
Imports System.Drawing
Imports System.Threading.Tasks
Imports System.Windows.Forms
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide

Namespace win_async
    Partial Public Class AsyncTest
        Inherits Form

        Public Sub New()
            InitializeComponent()
        End Sub

        Private Function GetTask() As Task(Of String)
            Dim hw As HelloWorld = m_spHw.AsyncHandlers(0)
            Return hw.Async(Of String, String, String)(hwConst.idSayHelloHelloWorld, "Jack", "Smith")
        End Function

        Private Function GetTasksInBatch() As Task(Of String)
            Dim hw As HelloWorld = m_spHw.AsyncHandlers(0)
            Dim ok As Boolean = hw.SendRequest(hwConst.idSleepHelloWorld, 5000, Sub(ar)
                                                                 End Sub)
            Dim task As Task(Of String) = hw.Async(Of String, String, String)(hwConst.idSayHelloHelloWorld, "Jone", "Don")
            Return task
        End Function

        Private m_spHw As New CSocketPool(Of HelloWorld)(True)

        Private Sub async_Load(ByVal sender As Object, ByVal e As EventArgs) Handles MyBase.Load
            Dim cc As New CConnectionContext("127.0.0.1", 20901, "MyUserId", "MyPassword")
            btnTest.Enabled = m_spHw.StartSocketPool(cc, 1, 1)
        End Sub

        Private Async Sub btnTest_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnTest.Click
            If m_spHw.ConnectedSockets = 0 Then
                txtRes.Text = "No connection"
                Return
            End If
            btnTest.Enabled = False
            Try
                'execute one request asynchronously
                txtRes.Text = Await GetTask()

                'execute multiple requests asynchronously in batch
                txtRes.Text = Await GetTasksInBatch()
                btnTest.Enabled = True
            Catch err As Exception
                txtRes.Text = err.Message
            End Try
        End Sub
    End Class
End Namespace
