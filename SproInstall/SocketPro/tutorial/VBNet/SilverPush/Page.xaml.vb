Imports Microsoft.VisualBasic
Imports System
Imports System.Collections.Generic
Imports System.Windows
Imports System.Windows.Controls
Imports System.Windows.Browser
Imports SocketProAdapter.ClientSide

Namespace SilverPush
	Partial Public Class Page
		Inherits UserControl
		Private m_lIndex As Long = 0
		Public Sub New()
			InitializeComponent()
			AddHandler UHTTP.UChat.OnChatRequest, AddressOf UChat_OnChatRequest
			AddHandler UHTTP.UChat.OnMessage, AddressOf UChat_OnMessage
		End Sub

        Private Sub UChat_OnMessage(ByVal Request As CRequest, ByVal HttpPush As CHttpPush)
            Select Case HttpPush.Type
                Case MessageType.Normal
                    If True Then
                        Dim str As String = ""
                        For Each cm As CChatMessage In HttpPush.Messages
                            Dim strGroups As String = "["
                            If cm.Groups IsNot Nothing Then
                                For n As Integer = 0 To cm.Groups.Length - 1
                                    If n > 0 Then
                                        strGroups += ", "
                                    End If
                                    strGroups += cm.Groups(n).ToString()
                                Next
                                strGroups += "]"
                            End If
                            If str.Length > 0 Then
                                str += "<-->"
                            End If
                            If cm.MethodName = "enter" Then
                                str += String.Format("Sender = {0}, Message = join groups = {1}", cm.Sender, strGroups)
                            ElseIf cm.MethodName = "exit" Then
                                str += String.Format("Sender = {0}, Message = {1}, groups = {2}", cm.Sender, "exit", strGroups)
                            Else
                                str += String.Format("Sender = {0}, Message = {1}, groups = {2}", cm.Sender, cm.Message, strGroups)
                            End If
                        Next
                        txtMsg.Text = str
                    End If
                    Exit Select
                Case MessageType.ServerShuttingdownGracefully
                    txtMsg.Text = "SocketPro HTTP push server shut down gracefully"
                    Exit Select
                Case MessageType.Timeout
                    txtMsg.Text = "Time out"
                    Exit Select
                Case Else
                    txtMsg.Text = "Unknown message"
                    Exit Select
            End Select
        End Sub


		Private Sub UChat_OnChatRequest(ByVal Request As CRequest, ByVal Result As Object)
			Select Case Request.Name
				Case "enter"
					If UHTTP.UChat.Chatting Then
						btnEnter.IsEnabled = False
						btnSpeak.IsEnabled = True
					End If
				Case "exit"
					btnEnter.IsEnabled = True
					btnSpeak.IsEnabled = False
				Case Else
			End Select
			txtMsg.Text = String.Format("Method = {0}, Result = {1}", Request.Name, Result.ToString())
		End Sub

        Private Sub btnEnter_Click(ByVal sender As Object, ByVal e As RoutedEventArgs)
            Dim Groups As UInteger() = {1, 2}
            UHTTP.UChat.Enter(txtMyUserId.Text, Groups)
        End Sub

		Private Sub btnExit_Click(ByVal sender As Object, ByVal e As RoutedEventArgs)
			UHTTP.UChat.Exit()
		End Sub

		Private Sub btnSpeak_Click(ByVal sender As Object, ByVal e As RoutedEventArgs)
            Dim Groups As UInteger() = {1, 2, 8}
            UHTTP.UChat.Speak(txtMsgOut.Text, Groups)
		End Sub

		Private Sub btnDoMyRequest_Click(ByVal sender As Object, ByVal e As RoutedEventArgs)
			Dim map As New Dictionary(Of String, Object)()

			map.Add("Map0", 123.45)
			map.Add("Map1", Nothing)
			map.Add("Map2", "A sample map string")

			Dim AComplexStructure(6) As Object
			AComplexStructure(0) = map
			AComplexStructure(1) = "MyString"
			AComplexStructure(2) = DateTime.Now.AddDays(-2)
			AComplexStructure(4) = True
			AComplexStructure(6) = 34567

			Dim child(1) As Object
			child(0) = "myDate"
			child(1) = DateTime.Now.AddDays(-5)
			AComplexStructure(3) = child

            Dim reqComplex As CRequest = UHTTP.CreateHttpRequest("MyCall", "/MyChannel")
			reqComplex.Add("Param0", m_lIndex)
			reqComplex.Add("Param1", 23.45)
			reqComplex.Add("MyParam", "A demo string")
			reqComplex.Add("ABoolean", True)
			reqComplex.Add("ADateTime", DateTime.Now)
			reqComplex.Add("AnArray", AComplexStructure)
			reqComplex.Add("CyeMap", map)

			m_lIndex = reqComplex.Invoke(Function(myReq, strResult) AnonymousMethod1(myReq, strResult))
		End Sub
		
		Private Function AnonymousMethod1(ByVal myReq As CRequest, ByVal strResult As String) As Object
			HtmlPage.Window.Alert(strResult)
			Return Nothing
		End Function
	End Class
End Namespace
