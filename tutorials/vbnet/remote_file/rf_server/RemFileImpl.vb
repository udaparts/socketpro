' **** including all of defines, service id(s) and request id(s) ***** 
Imports System
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide
Imports System.IO

'server implementation for service RemotingFile
Public Class RemotingFilePeer
	Inherits CClientPeer

	Protected Overrides Sub OnReleaseResource(ByVal closing As Boolean, ByVal nInfo As UInteger)
		CleanTarget()
		CleanSource()
	End Sub

	Private m_fsSource As FileStream = Nothing
	<RequestAttr(CStreamSerializationHelper.idStartDownloading, True)>
	Private Function StartDownloadingFile(ByVal RemoteFilePath As String, ByRef fileSize As ULong) As String
		Dim errMsg As String = Nothing
		m_fsSource = CStreamHelper.DownloadFile(Handle, RemoteFilePath, fileSize, errMsg)
		Return errMsg
	End Function
	<RequestAttr(CStreamSerializationHelper.idReadDataFromServerToClient, True)>
	Private Sub MoveDataFromServerToClient()
		CStreamHelper.ReadDataFromServerToClient(Handle, m_fsSource)
	End Sub
	<RequestAttr(CStreamSerializationHelper.idDownloadCompleted)>
	Private Sub WaitDownloadCompleted()
		CleanSource()
	End Sub
	Private Sub CleanSource()
		If m_fsSource IsNot Nothing Then
			m_fsSource.Close()
			m_fsSource = Nothing
		End If
	End Sub


	Private m_fsReceiver As FileStream = Nothing
	<RequestAttr(CStreamSerializationHelper.idStartUploading, True)>
	Private Function StartUploadingFile(ByVal RemoteFilePath As String) As String
		Try
			m_fsReceiver = New FileStream(RemoteFilePath, FileMode.Append)
		Catch err As Exception
			Return err.Message
		End Try
		Return ""
	End Function
	<RequestAttr(CStreamSerializationHelper.idWriteDataFromClientToServer, True)>
	Private Sub MoveDataFromClientToServer()
		CStreamHelper.WriteDataFromClientToServer(UQueue, m_fsReceiver)
	End Sub
	<RequestAttr(CStreamSerializationHelper.idUploadCompleted)>
	Private Sub WaitUploadingCompleted()
		CleanTarget()
	End Sub
	Private Sub CleanTarget()
		If m_fsReceiver IsNot Nothing Then
			m_fsReceiver.Close()
			m_fsReceiver = Nothing
		End If
	End Sub
End Class
