' **** including all of defines, service id(s) and request id(s) ***** 
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide
Imports USOCKETLib 'you may need it for accessing various constants
Imports SampleThreeShared
Imports System.Collections

'server implementation for service CTThree
Public Class CTThreePeer
	Inherits CClientPeer
	Protected Overrides Sub OnSwitchFrom(ByVal nServiceID As Integer)
		m_TThreeSvs = CType(GetBaseService(), CTThreeSvs)
		Console.WriteLine("Socket is switched for the service CTThreeSvs")
	End Sub

	Protected Overrides Sub OnReleaseResource(ByVal bClosing As Boolean, ByVal nInfo As Integer)
		If bClosing Then
			Console.WriteLine("Socket is closed with error code = " & nInfo)
		Else
			Console.WriteLine("Socket is going to be switched to new service with service id = " & nInfo)
		End If
		m_Stack.Clear()
	End Sub

	Protected Sub GetOneItem(ByRef GetOneItemRtn As CTestItem)
		If m_TThreeSvs.m_Stack.Count > 0 Then
			GetOneItemRtn = CType(m_TThreeSvs.m_Stack.Pop(), CTestItem)
		Else
            GetOneItemRtn = Nothing
		End If
	End Sub

	Protected Sub SendOneItem(ByVal Item As CTestItem)
		m_TThreeSvs.m_Stack.Push(Item)
	End Sub

	Protected Sub GetManyItems()
		Dim nRtn As Integer = 0
		m_UQueue.SetSize(0)
		Do While m_Stack.Count > 0
			'a client may either shut down the socket connection or call IUSocket::Cancel
			If nRtn = SOCKET_NOT_FOUND OrElse nRtn = REQUEST_CANCELED Then
				Exit Do
			End If
			Dim Item As CTestItem = CType(m_Stack.Pop(), CTestItem)
            m_UQueue.Push(Item)

			'20 kbytes per batch at least
			'also shouldn't be too large. 
			'If the size is too large, it will cost more memory resource and reduce conccurency if online compressing is enabled.
			'for an opimal value, you'd better test it by yourself
			If m_UQueue.GetSize() > 20480 Then
				nRtn = SendResult(TThreeConst.idGetBatchItemsCTThree, m_UQueue)
				m_UQueue.SetSize(0)
			End If
		Loop
		If nRtn = SOCKET_NOT_FOUND OrElse nRtn = REQUEST_CANCELED Then

		ElseIf m_UQueue.GetSize() > Len(New Integer) Then
			nRtn = SendResult(TThreeConst.idGetBatchItemsCTThree, m_UQueue)
		End If
	End Sub

	Protected Sub SendManyItems()
		Do While m_Stack.Count > 0
			m_TThreeSvs.m_Stack.Push(m_Stack.Pop())
		Loop
	End Sub

	Protected Sub SendBatchItems()
		Do While m_UQueue.GetSize() > 0
            Dim Item As CTestItem = Nothing
            m_UQueue.Pop(Item)
			m_Stack.Push(Item)
		Loop
	End Sub

	Protected Overrides Sub OnFastRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer)
		Select Case sRequestID
		Case TThreeConst.idGetOneItemCTThree
			M_I0_R1(Of CTestItem)(AddressOf GetOneItem)
		Case TThreeConst.idSendOneItemCTThree
			M_I1_R0(Of CTestItem)(AddressOf SendOneItem)
		Case TThreeConst.idSendManyItemsCTThree
			M_I0_R0(AddressOf SendManyItems)
		Case Else
		End Select
	End Sub

	Private Function RetrieveCount() As Integer
		Dim nCount As Integer = 0
		m_UQueue.Pop(nCount)
		Return nCount
	End Function

	Protected Overrides Sub OnDispatchingSlowRequest(ByVal sRequestID As Short)
		If sRequestID = TThreeConst.idGetManyItemsCTThree Then
			Dim nCount As Integer = RetrieveCount()
			m_Stack.Clear()
			Do While m_TThreeSvs.m_Stack.Count > 0 AndAlso nCount > 0
				m_Stack.Push(m_TThreeSvs.m_Stack.Pop())
				nCount -= 1
			Loop
		End If
	End Sub

	Protected Overrides Function OnSlowRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer) As Integer
		Select Case sRequestID

		Case TThreeConst.idGetManyItemsCTThree
			M_I0_R0(AddressOf GetManyItems)
		Case TThreeConst.idSendBatchItemsCTThree
			M_I0_R0(AddressOf SendBatchItems)
		Case Else
		End Select
		Return 0
	End Function
	Private m_Stack As New Stack()
	Private m_TThreeSvs As CTThreeSvs
End Class

Public Class CTThreeSvs
    Inherits CSocketProService(Of CTThreePeer)

	Friend m_Stack As New Stack()
End Class

