Imports System.Text
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide
Imports System.IO

Friend Class CHttpPeer
	Inherits CHttpPeerBase
	Private m_nIndex As Integer
	Private Sub DownloadFile(ByVal strFile As String)
		Dim res As Integer
		Dim nBufferSize As Integer = 10240
		Dim readIn As New FileStream(strFile, FileMode.Open, FileAccess.Read)
		'tell a client the size of the coming file
        Dim ok As Boolean = SetResponseHeader("Content-Length", readIn.Length.ToString())

		Dim buffer(nBufferSize - 1) As Byte
		Dim nRead As Integer = readIn.Read(buffer, 0, nBufferSize)
		Do While nRead > 0
			res = SendReturnData(CShort(Fix(USOCKETLib.tagHttpRequestID.idGet)), buffer, nRead)
			If res < 0 Then 'client cancels downloading or shuts down TCP/IP connection
				Exit Do
			End If
			nRead = readIn.Read(buffer, 0, nBufferSize)
		Loop
		readIn.Close()

	End Sub
	Private Sub GetFile(ByVal strFile As String)
		m_UQueue.SetHeadPosition()
		Dim readIn As New FileStream(strFile, FileMode.Open, FileAccess.Read)
		Dim buffer(10239) As Byte
		Dim nRead As Integer = readIn.Read(buffer, 0, 10240)
		Do While nRead > 0
			m_UQueue.Push(buffer, nRead)
			nRead = readIn.Read(buffer, 0, 10240)
		Loop
		readIn.Close()
	End Sub

	Protected Overrides Sub OnFastRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer)
		MyBase.OnFastRequestArrive(sRequestID, nLen)
		Select Case sRequestID
			Case CShort(Fix(USOCKETLib.tagHttpRequestID.idHeader))
				m_nIndex = 0
			Case Else
				m_nIndex = 0
		End Select
	End Sub
	Protected Overrides Function OnSlowRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer) As Integer
		Dim enc As Encoding = System.Text.UTF8Encoding.UTF8
		SetResponseHeader("MyOwnHeader", "Sometext")
		Select Case sRequestID
			Case CShort(Fix(USOCKETLib.tagHttpRequestID.idMultiPart))
				m_nIndex += 1
				Console.Write("Size = ")
				Console.Write(nLen.ToString())
				Console.Write(", index = ")
				Console.WriteLine(m_nIndex.ToString())
			Case CShort(Fix(USOCKETLib.tagHttpRequestID.idGet))

				Select Case PathName
					Case "/", "/udemo.htm", ""
						SetResponseHeader("Content-Type", "text/html")
						m_UQueue.SetSize(0)
						GetFile("udemo.htm")
						SendResult(sRequestID, m_UQueue)
					Case "/ujsonxml.js"
						SetResponseHeader("Content-Type", "application/x-javascript")

						'trun off connection right after sending response
						SetResponseHeader("Connection", "close")

						m_UQueue.SetSize(0)
						GetFile("ujsonxml.js")
						SendResult(sRequestID, m_UQueue)
					Case "/multipart.htm"
						SetResponseHeader("Content-Type", "text/html; application/x-javascript")

						'trun off connection right after sending response
						SetResponseHeader("Connection", "close")

						m_UQueue.SetSize(0)
						GetFile("multipart.htm")
						SendResult(sRequestID, m_UQueue)
					Case "/fupload.htm"
						SetResponseHeader("Content-Type", "text/html; application/javascript")

						'trun off connection right after sending response
						SetResponseHeader("Connection", "close")

						m_UQueue.SetSize(0)
						GetFile("fupload.htm")
						SendResult(sRequestID, m_UQueue)
					Case "/chunked.htm"
						SetResponseHeader("Content-Type", "text/html")
						SetResponseHeader("Transfer-Encoding", "chunked")

						m_UQueue.SetSize(0)
						m_UQueue.Push(enc.GetBytes("<script type='text/javascript'>"))
						GetFile("ujsonxml.js")
						m_UQueue.Push(enc.GetBytes("</script>"))
						SendResult(sRequestID, m_UQueue)

						m_UQueue.SetSize(0)
						GetFile("chunked.htm")
						SendResult(sRequestID, m_UQueue)

						'tell browser the chunked response is ended
						SendResult(sRequestID)
					Case "/mpart"
						SetResponseHeader("Keep-Alive", "10") '10 seconds
						SetResponseHeader("Content-Type", "multipart/x-mixed-replace; boundary=rnA00A")

						m_UQueue.SetSize(0)
						SetResponseHeader("Content-Type", "application/xml; charset=utf-8")
						m_UQueue.Push(enc.GetBytes("<?xml version='1.0'?>"))
						m_UQueue.Push(enc.GetBytes("<content>First Part</content>"))
						SendResult(sRequestID, m_UQueue)
						System.Threading.Thread.Sleep(5000)

						m_UQueue.SetSize(0)
						SetResponseHeader("Content-Type", "application/xml; charset=utf-8")
						m_UQueue.Push(enc.GetBytes("<?xml version='1.0'?>"))
						m_UQueue.Push(enc.GetBytes("<content>Second Part</content>"))
						SendResult(sRequestID, m_UQueue)
						System.Threading.Thread.Sleep(5000)

						m_UQueue.SetSize(0)
						SetResponseHeader("Content-Type", "application/xml; charset=utf-8")
						m_UQueue.Push(enc.GetBytes("<?xml version='1.0'?>"))
						m_UQueue.Push(enc.GetBytes("<content>Third Part</content>"))
						SendResult(sRequestID, m_UQueue)
						System.Threading.Thread.Sleep(5000)

						m_UQueue.SetSize(0)
						SetResponseHeader("Content-Type", "application/xml; charset=utf-8")
						m_UQueue.Push(enc.GetBytes("<?xml version='1.0'?>"))
						m_UQueue.Push(enc.GetBytes("<content>Fourth Part</content>"))
						SendResult(sRequestID, m_UQueue)
						System.Threading.Thread.Sleep(5000)

						m_UQueue.SetSize(0)
						SetResponseHeader("Content-Type", "application/xml; charset=utf-8")
						m_UQueue.Push(enc.GetBytes("<?xml version='1.0'?>"))
						m_UQueue.Push(enc.GetBytes("<content>Ended</content>"))
						SendResult(sRequestID, m_UQueue)

						'tell browser the multipart response is ended
						SendResult(sRequestID)
					Case "/sampledownload.dll"
						SetResponseHeader("Connection", "close")
						DownloadFile("sampledownload.dll")
					Case "/processing"
						SetResponseCode(500) 'not implemented
						SendResult(sRequestID)
					Case Else
						SetResponseCode(404) '404 Not Found
						SendResult(sRequestID)
				End Select
			Case CShort(Fix(USOCKETLib.tagHttpRequestID.idPost))
				Select Case PathName
					Case "/processing"
						'trun off connection right after sending response
						'SetResponseHeader("Connection", "close");

						SetResponseHeader("Content-Type", "application/json")

						'echo back to client
						SendResult(sRequestID, m_UQueue)
					Case "/uploadfile"
						SetResponseHeader("Content-Type", "text/html; application/javascript")

						'trun off connection right after sending response
						SetResponseHeader("Connection", "close")

						m_UQueue.SetSize(0)
						GetFile("fupload.htm")
						SendResult(sRequestID, m_UQueue)
					Case Else
						'trun off connection right after sending response
						SetResponseHeader("Connection", "close")

						SetResponseCode(404) '404 Not Found
						SendResult(sRequestID)
				End Select
			Case Else
		End Select
		Return 0
	End Function
End Class

