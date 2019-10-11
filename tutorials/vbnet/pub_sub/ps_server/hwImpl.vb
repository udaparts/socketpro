' **** including all of defines, service id(s) and request id(s) ***** 
Imports SocketProAdapter.ServerSide

'server implementation for service HelloWorld
Public Class HelloWorldPeer
	Inherits CClientPeer

	Public Shared Overloads Function ToString(ByVal groups() As UInteger) As String
		Dim n As Integer = 0
		Dim s As String = "["
		For Each id As UInteger In groups
			If n <> 0 Then
				s &= ", "
			End If
			s &= id
			n += 1
		Next id
		s &= "]"
		Return s
	End Function

	Protected Overrides Sub OnSwitchFrom(ByVal oldServiceId As UInteger)
		Dim chat_groups() As UInteger = { 1, 3 }
		Push.Subscribe(chat_groups)
	End Sub

	Protected Overrides Sub OnSubscribe(ByVal groups() As UInteger)
		Console.WriteLine(UID & " subscribes for groups " & ToString(groups))
	End Sub

	Protected Overrides Sub OnUnsubscribe(ByVal groups() As UInteger)
		Console.WriteLine(UID & " unsubscribes for groups " & ToString(groups))
	End Sub

	Protected Overrides Sub OnPublish(ByVal message As Object, ByVal groups() As UInteger)
        Console.WriteLine(UID & " publishes a message (" & message.ToString() & ") to groups " & ToString(groups))
	End Sub

	Protected Overrides Sub OnSendUserMessage(ByVal receiver As String, ByVal message As Object)
        Console.WriteLine(UID & " sends a message (" & message.ToString() & ") to " & receiver)
	End Sub

	<RequestAttr(hwConst.idSayHelloHelloWorld)>
	Private Function SayHello(ByVal firstName As String, ByVal lastName As String) As String
		'processed within main thread
		System.Diagnostics.Debug.Assert(CSocketProServer.IsMainThread)

		'notify a message to groups [2, 3] at server side
		Push.Publish("Say hello from " & firstName & " " & lastName, 2, 3)

		Dim res As String = "Hello " & firstName & " " & lastName
		Console.WriteLine(res)
		Return res
	End Function

	<RequestAttr(hwConst.idSleepHelloWorld, True)>
	Private Sub Sleep(ByVal ms As Integer) 'true -- slow request
		'processed within a worker thread
		System.Diagnostics.Debug.Assert(Not CSocketProServer.IsMainThread)

		System.Threading.Thread.Sleep(ms)
	End Sub

	<RequestAttr(hwConst.idEchoHelloWorld)>
	Private Function Echo(ByVal ms As CMyStruct) As CMyStruct
		Return ms
	End Function
End Class

