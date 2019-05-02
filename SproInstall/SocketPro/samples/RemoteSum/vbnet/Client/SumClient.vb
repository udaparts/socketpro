Imports System.ComponentModel
Imports System.Text
Imports SocketProAdapter.ClientSide

Namespace Client
	Partial Public Class SumClient
		Inherits Form
		Implements IAsyncResultsHandler
		Public Sub New()
			InitializeComponent()
		End Sub

		Private m_cs As New CClientSocket()
        Private m_ash As CAsyncServiceHandler

        Private Sub OnConnected(ByVal hSocket As Integer, ByVal nErrorCode As Integer)
            If nErrorCode = 0 Then
                btnDoSum.Enabled = True
                btnPause.Enabled = True
                btnRedoSum.Enabled = True
                m_cs.SwitchTo(m_ash)
            Else
                MessageBox.Show(m_cs.GetErrorMsg())
            End If
        End Sub

        Private Sub OnSocketClosed(ByVal hSocket As Integer, ByVal nErrorCode As Integer)
            btnDoSum.Enabled = False
            btnPause.Enabled = False
            btnRedoSum.Enabled = False
        End Sub

        Private Sub SumClient_Load(ByVal sender As Object, ByVal e As EventArgs) Handles MyBase.Load
            m_ash = New CAsyncServiceHandler(RemoteSumConst.sidRemSum, m_cs, Me)
            m_cs.m_OnSocketConnected = AddressOf OnConnected
            m_cs.m_OnSocketClosed = AddressOf OnSocketClosed
            m_cs.Connect("localhost", 20901)
        End Sub

#Region "IAsyncResultsHandler Members"

        Public Sub OnExceptionFromServer(ByVal AsyncServiceHandler As CAsyncServiceHandler, ByVal Exception As SocketProAdapter.CSocketProServerException) Implements IAsyncResultsHandler.OnExceptionFromServer
            Throw New Exception("The method or operation is not implemented.")
        End Sub

        Public Sub Process(ByVal AsyncResult As CAsyncResult) Implements IAsyncResultsHandler.Process
            Select Case AsyncResult.RequestId
                Case RemoteSumConst.idPauseRemSum, RemoteSumConst.idRedoSumRemSum, RemoteSumConst.idDoSumRemSum
                    Dim rtn As Integer = 0
                    AsyncResult.UQueue.Pop(rtn)
                    txtSum.Text = rtn.ToString()
                Case RemoteSumConst.idReportProgress
                    Dim nWhere As Integer = 0
                    Dim nSum As Integer = 0
                    AsyncResult.UQueue.Pop(nWhere)
                    AsyncResult.UQueue.Pop(nSum)
                    txtSum.Text = "Where = " & nWhere.ToString() & ", Sum = " & nSum.ToString()
                Case Else
            End Select
        End Sub

#End Region

        Private Sub btnPause_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnPause.Click
            m_cs.BeginBatching()

            'send a cancel request to a remote server. Here is a big secret from SocketPro!
            m_cs.Cancel()

            m_ash.SendRequest(RemoteSumConst.idPauseRemSum)
            m_cs.Commit(True) 'make two requests in one shot
        End Sub

        Private Sub btnDoSum_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnDoSum.Click
            Dim start As Integer = 100
            Dim last As Integer = 400
            m_ash.SendRequest(RemoteSumConst.idDoSumRemSum, start, last)
        End Sub

        Private Sub btnRedoSum_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnRedoSum.Click
            m_ash.SendRequest(RemoteSumConst.idRedoSumRemSum)
        End Sub
    End Class
End Namespace