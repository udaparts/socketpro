Imports Microsoft.VisualBasic
Imports System
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports USOCKETLib
Imports SampleThreeShared
Imports System.Collections

Public Class CTThree : Inherits CAsyncServiceHandler
    Public Sub New()
        MyBase.New(TThreeConst.sidCTThree)
    End Sub

    Public Sub New(ByVal cs As CClientSocket)
        MyBase.New(TThreeConst.sidCTThree, cs)
    End Sub
    Public Sub New(ByVal cs As CClientSocket, ByVal DefaultAsyncResultsHandler As IAsyncResultsHandler)
        MyBase.New(TThreeConst.sidCTThree, cs, DefaultAsyncResultsHandler)
    End Sub

    Public Function GetOneItem() As CTestItem
        Dim ti As CTestItem = Nothing
        Dim b As Boolean = ProcessR1(TThreeConst.idGetOneItemCTThree, ti)
        Return ti
    End Function

    Public Sub SendOneItem(ByVal Item As CTestItem)
        Dim b As Boolean = ProcessR0(TThreeConst.idSendOneItemCTThree, Item)
    End Sub

    Public Function GetManyItems(ByVal nCount As Integer) As Stack
        m_Stack.Clear()
        Dim b As Boolean = ProcessR0(TThreeConst.idGetManyItemsCTThree, nCount)
        Return m_Stack
    End Function
    Public Sub SendManyItems(ByVal outStack As Stack)
        m_OutStack = outStack
        SendBatchItems()
        WaitAll()
        'at last, we inform a server that we finish sending items
        Dim b As Boolean = ProcessR0(TThreeConst.idSendManyItemsCTThree)
    End Sub

    Private Sub SendBatchItems()
        Dim nBatch As Integer = 0
        Dim nSndSize As Integer
        Dim UQueue As CUQueue = New CScopeUQueue().UQueue
        UQueue.SetSize(0)
        nSndSize = GetAttachedClientSocket().GetUSocket().BytesInSndMemory
        Do While nBatch < 200 AndAlso nSndSize < 40960 AndAlso Not m_OutStack Is Nothing AndAlso m_OutStack.Count > 0
            Dim Item As CTestItem = CType(m_OutStack.Pop(), CTestItem)
            UQueue.Push(Item)
            nBatch += 1
            If (nBatch = 200) Then
                SendRequest(TThreeConst.idSendBatchItemsCTThree, UQueue)
                UQueue.SetSize(0)
                nSndSize = GetAttachedClientSocket().GetUSocket().BytesInSndMemory
                If (nSndSize < 40960) Then
                    nBatch = 0
                Else
                    Exit Do
                End If
            End If
        Loop
        If UQueue.GetSize() > 0 Then
            SendRequest(TThreeConst.idSendBatchItemsCTThree, UQueue)
            UQueue.SetSize(0)
        End If
    End Sub
    Private m_OutStack As Stack
    Private m_Stack As Stack = New Stack()

    'We can process returning results inside the function.
    Protected Overrides Sub OnResultReturned(ByVal sRequestID As Short, ByVal UQueue As CUQueue)
        Select Case sRequestID
            Case TThreeConst.idGetBatchItemsCTThree
                Do While UQueue.GetSize() > 0
                    Dim Item As CTestItem = Nothing
                    UQueue.Pop(Item)
                    m_Stack.Push(Item)
                Loop
            Case TThreeConst.idSendBatchItemsCTThree
                SendBatchItems()
            Case Else
        End Select
    End Sub
End Class
