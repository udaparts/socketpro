Imports System
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports USOCKETLib

Public Class CTOne
    Inherits CAsyncServiceHandler

    Public Sub New()
        MyBase.New(TOneConst.sidCTOne)
    End Sub

    Public Sub New(ByVal cs As CClientSocket)
        MyBase.New(TOneConst.sidCTOne, cs)
    End Sub

    Public Sub New(ByVal cs As CClientSocket, ByVal DefaultAsyncResultsHandler As IAsyncResultsHandler)
        MyBase.New(TOneConst.sidCTOne, cs, DefaultAsyncResultsHandler)
    End Sub

    Public Function QueryCount() As Integer
        Dim QueryCountRtn As Integer
        Dim bProcessRy As Boolean = ProcessR1(TOneConst.idQueryCountCTOne, QueryCountRtn)
        Return QueryCountRtn
    End Function

    Public Function QueryGlobalCount() As Integer
        Dim QueryGlobalCountRtn As Integer
        Dim bProcessRy As Boolean = ProcessR1(TOneConst.idQueryGlobalCountCTOne, QueryGlobalCountRtn)
        Return QueryGlobalCountRtn
    End Function

    Public Function QueryGlobalFastCount() As Integer
        Dim QueryGlobalFastCountRtn As Integer
        Dim bProcessRy As Boolean = ProcessR1(TOneConst.idQueryGlobalFastCountCTOne, QueryGlobalFastCountRtn)
        Return QueryGlobalFastCountRtn
    End Function

    Public Sub Sleep(ByVal nTime As Integer)
        Dim bProcessRy As Boolean = ProcessR0(TOneConst.idSleepCTOne, nTime)
    End Sub

    Public Function Echo(ByVal objInput As Object) As Object
        Dim EchoRtn As Object = Nothing
        Dim Groups() As Integer = {1, 3}
        'send a text message to groups 1 and 3
        GetAttachedClientSocket().Push.Broadcast("EchoData called", Groups)
        Dim bProcessRy As Boolean = ProcessR1(TOneConst.idEchoCTOne, objInput, EchoRtn)
        Return EchoRtn
    End Function

    Protected m_QueryCountRtn As Integer
    Protected Sub QueryCountAsync()
        SendRequest(TOneConst.idQueryCountCTOne)
    End Sub

    Protected m_QueryGlobalCountRtn As Integer
    Protected Sub QueryGlobalCountAsync()
        SendRequest(TOneConst.idQueryGlobalCountCTOne)
    End Sub

    Protected m_QueryGlobalFastCountRtn As Integer
    Protected Sub QueryGlobalFastCountAsync()
        SendRequest(TOneConst.idQueryGlobalFastCountCTOne)
    End Sub

    'We can process returning results inside the function.
    Protected Overrides Sub OnResultReturned(ByVal sRequestID As Short, ByVal UQueue As CUQueue)
        Select Case (sRequestID)
            Case TOneConst.idQueryCountCTOne
                UQueue.Pop(m_QueryCountRtn)
            Case TOneConst.idQueryGlobalCountCTOne
                UQueue.Pop(m_QueryGlobalCountRtn)
            Case TOneConst.idQueryGlobalFastCountCTOne
                UQueue.Pop(m_QueryGlobalFastCountRtn)
            Case Else
        End Select
    End Sub

    Public Sub GetAllCounts(ByRef nCount As Integer, ByRef nGlobalCount As Integer, ByRef nGlobalFastCount As Integer)
        BeginBatching()
        QueryCountAsync()
        QueryGlobalFastCountAsync()
        QueryGlobalCountAsync()
        CommitBatch(True) 'true -- ask server to send three results back in one batch
        WaitAll()
        nCount = m_QueryCountRtn
        nGlobalCount = m_QueryGlobalCountRtn
        nGlobalFastCount = m_QueryGlobalFastCountRtn
    End Sub

    Public Sub GetAllCounts(ByRef nCount As Integer, ByRef nGlobalCount As Integer)
        BeginBatching()
        QueryCountAsync()
        QueryGlobalCountAsync()
        CommitBatch(True) 'true -- ask server to send two results back in one batch
        WaitAll()
        nCount = m_QueryCountRtn
        nGlobalCount = m_QueryGlobalCountRtn
    End Sub

    Public Sub GetAllCounts(ByRef nCount As Integer)
        nCount = QueryCount()
    End Sub
End Class