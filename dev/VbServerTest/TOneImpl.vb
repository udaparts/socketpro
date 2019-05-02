' **** including all of defines, service id(s) and request id(s) *****
Imports System
Imports SocketProAdapter
Imports SocketProAdapter.ServerSide

'server implementation for service CTOne
Public class CTOnePeer : Inherits CClientPeer
	Protected Overrides Sub OnSwitchFrom(ByVal nServiceID As UInteger)
		'initialize the object here
	End Sub

	Protected Overrides Sub OnReleaseResource(ByVal closing As Boolean, ByVal nInfo As UInteger)
		If closing Then
			'closing the socket with error code = nInfo
		Else
			'switch to a new service with the service id = nInfo
		End If

		'release all of your resources here as early as possible
	End Sub

    Private m_Count As Integer = 0
    <RequestAttr(TOneConst.idQueryCountCTOne)> Private Function QueryCount() As Integer
        m_Count = m_Count + 1
        m_GlobalFastCount = m_GlobalFastCount + 1
        m_GlobalCount = m_GlobalCount + 1
        Return m_Count
    End Function

    Private Shared m_cs As New Object
    Private Shared m_GlobalCount As Integer = 0
    <RequestAttr(TOneConst.idQueryGlobalCountCTOne)> Private Function QueryGlobalCount() As Integer
        m_Count = m_Count + 1
        m_GlobalFastCount = m_GlobalFastCount + 1
        SyncLock (m_cs)
            m_GlobalCount = m_GlobalCount + 1
            Return m_GlobalCount
        End SyncLock
    End Function

    Private Shared m_GlobalFastCount As Integer = 0
    <RequestAttr(TOneConst.idQueryGlobalFastCountCTOne)> Private Function QueryGlobalFastCount() As Integer
        m_Count = m_Count + 1
        m_GlobalFastCount = m_GlobalFastCount + 1
        SyncLock (m_cs)
            m_GlobalCount = m_GlobalCount + 1
        End SyncLock
        Return m_GlobalFastCount
    End Function

    <RequestAttr(TOneConst.idSleepCTOne, True)> Private Sub Sleep(ByVal nTime As Integer)
        m_Count = m_Count + 1
        SyncLock (m_cs)
            m_GlobalCount = m_GlobalCount + 1
        End SyncLock
        System.Threading.Thread.Sleep(nTime)
    End Sub

    <RequestAttr(TOneConst.idEchoCTOne)> Private Function Echo(ByVal objInput As Object) As Object
        m_Count = m_Count + 1
        m_GlobalFastCount = m_GlobalFastCount + 1
        SyncLock (m_cs)
            m_GlobalCount = m_GlobalCount + 1
        End SyncLock
        Return objInput
    End Function

    <RequestAttr(TOneConst.idEchoExCTOne)> Private Function EchoEx(ByVal str As SByte(), ByVal wstr As String, ByRef strOut As SByte(), ByRef wstrOut As String) As Boolean
        m_Count = m_Count + 1
        m_GlobalFastCount = m_GlobalFastCount + 1
        SyncLock (m_cs)
            m_GlobalCount = m_GlobalCount + 1
        End SyncLock
        strOut = str
        wstrOut = wstr
        Return True
    End Function


End Class

public class CMySocketProServer : Inherits CSocketProServer

	Protected Overrides Function OnSettingServer() As Boolean
		'amIntegrated and amMixed not supported yet
		Config.AuthenticationMethod = tagAuthenticationMethod.amOwn

		Return True 'true -- ok; and false -- no listening socket
	End Function

	Protected Overrides Sub OnAccept(ByVal hSocket as ULong, ByVal nError as Integer)
		'when a socket is initially established
	End Sub

	Protected Overrides Function OnIsPermitted(ByVal hSocket As ULong, ByVal userId as String, ByVal password as String, ByVal nSvsID As UInteger) As Boolean
		'give permission to all
		return true
	End Function

	Protected Overrides Sub OnClose(ByVal hSocket as ULong, ByVal nError as Integer)
		'when a socket is closed
	End Sub

    <ServiceAttr(TOneConst.sidCTOne)> Private m_CTOne As CSocketProService(Of CTOnePeer) = New CSocketProService(Of CTOnePeer)()
	'One SocketPro server supports any number of services. You can list them here!

End Class

