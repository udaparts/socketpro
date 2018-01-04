Imports System
Imports SocketProAdapter.ServerSide
Imports System.Diagnostics

Public Class CMyHttpPeer
	Inherits CHttpPeerBase

	Protected Overrides Sub OnSubscribe(ByVal groups() As UInteger)
		Console.WriteLine(UID & " subscribes for groups " & HelloWorldPeer.ToString(groups))
	End Sub

	Protected Overrides Sub OnUnsubscribe(ByVal groups() As UInteger)
		Console.WriteLine(UID & " unsubscribes for groups " & HelloWorldPeer.ToString(groups))
	End Sub

	Protected Overrides Sub OnPublish(ByVal message As Object, ByVal groups() As UInteger)
        Console.WriteLine(UID & " publishes a message (" & message.ToString() & ") to groups " & HelloWorldPeer.ToString(groups))
	End Sub

	Protected Overrides Sub OnSendUserMessage(ByVal receiver As String, ByVal message As Object)
        Console.WriteLine(UID & " sends a message (" & message.ToString() & ") to " & receiver)
	End Sub

	Protected Overrides Function DoAuthentication(ByVal userId As String, ByVal password As String) As Boolean
		Push.Subscribe(1, 2, 7)
		Console.Write("User id = " & userId)
		Console.WriteLine(", password = " & password)
		Return True 'true -- permitted; and false -- denied
	End Function

	Protected Overrides Sub OnGet()
		If Path.LastIndexOf("."c) <> 1 Then
			DownloadFile(Path.Substring(1))
		Else
			SendResult("test result --- GET ---")
		End If
	End Sub

	Protected Overrides Sub OnPost()
		Dim res As UInteger = SendResult("+++ POST +++ test result")
	End Sub

	Protected Overrides Sub OnUserRequest()
		Select Case RequestName
			Case "sleep"
				Dim ms As Integer = Integer.Parse(Args(0).ToString())
				Sleep(ms)
				SendResult("")
			Case "sayHello"
				SendResult(SayHello(Args(0).ToString(), Args(1).ToString()))
			Case Else
		End Select
	End Sub

	Private Function SayHello(ByVal firstName As String, ByVal lastName As String) As String
		'notify a message to groups [2, 3] at server side
		Push.Publish("Say hello from " & firstName & " " & lastName, 2, 3)

		Return "Hello " & firstName & " " & lastName
	End Function

	Private Sub Sleep(ByVal ms As Integer)
		System.Threading.Thread.Sleep(ms)
	End Sub
End Class