Imports SocketProAdapter
Imports SocketProAdapter.ServerSide
Imports System.Runtime.InteropServices
Imports System.Diagnostics
Imports System.Collections
Imports SampleThreeShared

Public Class CSThreeSvs
    Inherits CBaseService

    Protected Overrides Function GetPeerSocket(ByVal hSocket As Integer) As SocketProAdapter.ServerSide.CClientPeer
        Return New CSThreePeer
    End Function

    Public m_Stack As Stack

    Public Sub New()
        m_Stack = New Stack
    End Sub
End Class

Public Class CSThreePeer
    Inherits CClientPeer
    Private m_Stack As Stack

    Protected Overrides Sub OnDispatchingSlowRequest(ByVal sRequestID As Short)
        Select Case sRequestID
            Case CDefines.idGetALotItemsFromServer, CDefines.idGetItemsFromServerByDotNetSerializer
                'move a given number of items from service stack into this stack
                Dim nCount As Integer = 0
                Trace.Assert(CurrentRequestLen >= 4)
                m_UQueue.SetSize(4)
                RetrieveBuffer(m_UQueue.GetBuffer(), 4)
                m_UQueue.Pop(nCount)
                m_Stack.Clear()
                Dim SThreeSvs As CSThreeSvs = CBaseService.GetBaseService(SvsID)
                While (nCount > 0 And SThreeSvs.m_Stack.Count > 0)
                    m_Stack.Push(SThreeSvs.m_Stack.Pop())
                    nCount = nCount - 1
                End While
            Case Else
        End Select
    End Sub

    Protected Overrides Sub OnSlowRequestProcessed(ByVal sRequestID As Short)
        Select Case (sRequestID)
            Case CDefines.idSendItemsToServerByDotNetSerializer
                'move all of items in this stack into service stack
                Dim SThreeSvs As CSThreeSvs = CBaseService.GetBaseService(SvsID)
                While (m_Stack.Count > 0)
                    SThreeSvs.m_Stack.Push(m_Stack.Pop())
                End While
                SendReturnData(sRequestID, Nothing)
            Case Else
        End Select
    End Sub

    Protected Overrides Function OnSlowRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer) As Integer
        Select Case (sRequestID)
            Case CDefines.idSendItemsToServerByDotNetSerializer
                Dim obj As Object = Nothing
                m_Stack.Clear()
                Try
                    m_UQueue.Deserialize(obj)
                Catch eError As Exception
                    Console.WriteLine(eError.Message)
                End Try
                SendReturnData(sRequestID, Nothing)
                If (obj Is Nothing) Then
                Else
                    m_Stack = obj
                End If
            Case CDefines.idGetItemsFromServerByDotNetSerializer
                m_UQueue.SetSize(0)
                m_UQueue.Serialize(m_Stack)
                SendReturnData(sRequestID, m_UQueue)
                m_Stack.Clear()
            Case CDefines.idGetALotItemsFromServer
                Dim nRtn As Integer
                m_UQueue.SetSize(0)
                'tells how many items will be moved from server to client
                m_UQueue.Push(m_Stack.Count)
                nRtn = SendReturnData(CDefines.idStartToGetALotItemsFromServer, m_UQueue)
                m_UQueue.SetSize(0)
                While (m_Stack.Count > 0)
                    'a client may either shut down the socket connection or call IUSocket::Cancel
                    If (nRtn = SOCKET_NOT_FOUND Or nRtn = REQUEST_CANCELED) Then
                        Exit While
                    End If
                    Dim Item As CTestItem = m_Stack.Pop()
                    If Not (Item Is Nothing) Then
                        m_UQueue.Push(Item)
                        'or Item.SaveTo(m_UQueue)
                    End If
                    '20 kbytes per batch at least
                    'also shouldn't be too large. 
                    'If the size is too large, it will cost more memory resource and reduce conccurency if online compressing is enabled.
                    'for an opimal value, you'd better test it by yourself
                    If (m_UQueue.GetSize() > 20 * 1024) Then
                        nRtn = SendReturnData(CDefines.idGetingALotItemsFromServer, m_UQueue)
                        m_UQueue.SetSize(0)
                    End If
                End While
                If Not (nRtn = CClientPeer.SOCKET_NOT_FOUND Or nRtn = CClientPeer.REQUEST_CANCELED) Then
                    nRtn = SendReturnData(sRequestID, m_UQueue)
                End If
            Case CDefines.idSendingALotItemsToServer
                While (m_UQueue.GetSize() > 0)
                    Dim Item As CTestItem = New CTestItem
                    Item.LoadFrom(m_UQueue)
                    m_Stack.Push(Item)
                End While
                SendReturnData(sRequestID, Nothing)
            Case Else
        End Select
        Return 0
    End Function

    Protected Overrides Sub OnFastRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer)
        Select Case (sRequestID)
            Case CDefines.idGetAFewItemsFromServer
                Dim nCount As Integer = 0
                m_UQueue.Pop(nCount)
                m_UQueue.SetSize(0)
                Dim SThreeSvs As CSThreeSvs = CSThreeSvs.GetBaseService(SvsID)
                While (nCount > 0 And SThreeSvs.m_Stack.Count > 0)
                    Dim tItem As CTestItem = SThreeSvs.m_Stack.Pop()
                    If tItem Is Nothing Then
                    Else
                        'm_UQueue.Push(tItem)
                        tItem.SaveTo(m_UQueue)
                    End If
                    nCount = nCount - 1
                End While
                SendReturnData(sRequestID, m_UQueue)
            Case CDefines.idStartToSendALotItemsToServer
                SendReturnData(sRequestID, Nothing)
            Case CDefines.idSendALotItemsToServer
                'move all of items in this stack into service stack
                Dim SThreeSvs As CSThreeSvs = CBaseService.GetBaseService(SvsID)
                While (m_Stack.Count > 0)
                    SThreeSvs.m_Stack.Push(m_Stack.Pop())
                End While
                SendReturnData(sRequestID, Nothing)
            Case CDefines.idGetOneItemFromServer
                Dim SThreeSvs As CSThreeSvs = CSThreeSvs.GetBaseService(SvsID)
                m_UQueue.SetSize(0)
                If SThreeSvs.m_Stack.Count > 0 Then
                    Dim tItem As CTestItem = SThreeSvs.m_Stack.Pop()
                    If tItem Is Nothing Then
                    Else
                        tItem.SaveTo(m_UQueue)
                    End If
                End If
                SendReturnData(sRequestID, m_UQueue)
            Case CDefines.idSendAFewItemsToServer
                Dim SThreeSvs As CSThreeSvs = CSThreeSvs.GetBaseService(SvsID)
                While (m_UQueue.GetSize() > 0)
                    Dim tItem As CTestItem = New CTestItem
                    tItem.LoadFrom(m_UQueue)
                    SThreeSvs.m_Stack.Push(tItem)
                End While
                SendReturnData(sRequestID, Nothing)
            Case CDefines.idSendOneItemToServer
                Dim tItem As CTestItem = New CTestItem
                tItem.LoadFrom(m_UQueue)
                Dim SThreeSvs As CSThreeSvs = CSThreeSvs.GetBaseService(SvsID)
                SThreeSvs.m_Stack.Push(tItem)
                SendReturnData(sRequestID, Nothing)
            Case Else
        End Select
    End Sub

    Public Sub New()
        m_Stack = New Stack
    End Sub

    Protected Overrides Sub OnReleaseResource(ByVal bClosing As Boolean, ByVal nInfo As Integer)

        If bClosing Then
            Console.WriteLine("Socket {0} closed with error code = 0x{1:X}.", Socket, nInfo)
        Else
            Console.WriteLine("Switched to the service ID = 0x{0:X}.", nInfo)
        End If

        'reset it
        m_Stack.Clear()

        'release all of your resources here as early as possible
    End Sub

End Class

Public Class CSOnePeer
    Inherits CClientPeer

    <DllImport("Kernel32.dll")> _
    Private Shared Function GetCurrentThreadId() As Int32
    End Function

    'm_nCount doesn't need to be synchronized 
    'because it is always accessed from one socket client only, 
    'even though it may be accessed from different threads.
    Private m_nCount As Integer = 0

    Protected Overrides Sub OnReleaseResource(ByVal bClosing As Boolean, ByVal nInfo As Integer)

        'always processed within main thread
        Trace.Assert(GetCurrentThreadId() = CSocketProServer.MainThreadID)

        If bClosing Then
            Console.WriteLine("Socket {0} closed with error code = 0x{1:X}.", Socket, nInfo)
        Else
            Console.WriteLine("Switched to the service ID = 0x{0:X}.", nInfo)
        End If

        'reset it
        m_nCount = 0

        'release all of your resources here as early as possible
    End Sub


    Protected Overrides Sub OnFastRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer)
        'always processed within main thread
        Trace.Assert(CSocketProServer.MainThreadID = GetCurrentThreadId())
        Dim SOneSvs As CSOneSvs = CBaseService.GetBaseService(SvsID)

        m_nCount = m_nCount + 1
        SOneSvs.m_nGlobalFastCount = SOneSvs.m_nGlobalFastCount + 1

        SOneSvs.m_CS.Lock()
        SOneSvs.m_nGlobalCount = SOneSvs.m_nGlobalCount + 1
        SOneSvs.m_CS.Unlock()

        Select Case sRequestID
            Case CSOneSvs.idEchoData
                Dim nData As Integer
                Dim obj As Object = Nothing

                'sizeof(nData) + sizeof(vt)
                Trace.Assert(m_UQueue.GetSize() >= 6)

                m_UQueue.Pop(nData)
                m_UQueue.Pop(obj)

                Trace.Assert(m_UQueue.GetSize() = 0)

                'pack original data back into memory queue
                m_UQueue.Push(nData)
                m_UQueue.Push(obj)

                SendReturnData(sRequestID, m_UQueue)
            Case CSOneSvs.idQueryCount
                Trace.Assert(m_UQueue.GetSize() = 0)

                m_UQueue.Push(m_nCount)

                SendReturnData(sRequestID, m_UQueue)
            Case CSOneSvs.idQueryGlobalCount
                Trace.Assert(m_UQueue.GetSize() = 0)

                SOneSvs.m_CS.Lock()
                m_UQueue.Push(SOneSvs.m_nGlobalCount)
                SOneSvs.m_CS.Unlock()

                SendReturnData(sRequestID, m_UQueue)
            Case CSOneSvs.idQueryGlobalFastCount
                Trace.Assert(m_UQueue.GetSize() = 0)

                m_UQueue.Push(SOneSvs.m_nGlobalFastCount)

                SendReturnData(sRequestID, m_UQueue)
            Case Else
                SendErrorMessage(sRequestID, -1, "Unknown request")
        End Select
    End Sub

    Protected Overrides Function OnSlowRequestArrive(ByVal sRequestID As Short, ByVal nLen As Integer) As Integer
        'always processed within worker thread
        Trace.Assert(CSocketProServer.MainThreadID <> GetCurrentThreadId())
        m_nCount = m_nCount + 1
        Dim SOneSvs As CSOneSvs = CBaseService.GetBaseService(SvsID)

        SOneSvs.m_CS.Lock()
        SOneSvs.m_nGlobalCount = SOneSvs.m_nGlobalCount + 1
        SOneSvs.m_CS.Unlock()

        Select Case sRequestID
            Case CSOneSvs.idSleep
                Dim nSleep As Integer
                Dim bBatching As Boolean = IsBatching
                m_UQueue.Pop(nSleep)
                System.Threading.Thread.Sleep(nSleep)

                'make sure that two results sent to a client in one packet
                If Not bBatching Then
                    StartBatching()
                End If

                SendReturnData(sRequestID, Nothing)

                Dim groups() As Integer = {1, 2}

                'inform all of joined clients that idSleep is called
                Push.Broadcast("Sleep called", groups)

                If Not bBatching Then
                    CommitBatching()
                End If
            Case Else
                'never come to here
                Trace.Assert(False)
        End Select
        OnSlowRequestArrive = 0
    End Function
End Class

Public Class CSOneSvs
    Inherits CBaseService

    'define your own service id starting from 0x10000001
    'udaparts reserves service ids from 0x00000000 through 0x10000000
    Public Const sidSOneSvs As Integer = &H1F011011

    'define your request ids starting from 0x2001
    'udaparts reserves service ids from 0x0000 through 0x2000
    Public Const idEchoData As Short = &H2001
    Public Const idQueryCount As Short = &H2002
    Public Const idQueryGlobalCount As Short = &H2003
    Public Const idQueryGlobalFastCount As Short = &H2004
    Public Const idSleep As Short = &H2005

    'm_GlobalCount must be synchronized 
    'because it is accessed within different threads from different socket clients
    Public m_nGlobalCount As Integer
    Public m_CS As CCriticalSection = New CCriticalSection()

    'm_nGlobalFastCount doesn't need to be synchronized 
    'because it is accessed within main thread only, 
    'although is is accessed from different socket clients
    Public m_nGlobalFastCount As Integer = 0


    <DllImport("Kernel32.dll")> _
    Private Shared Function GetCurrentThreadId() As Int32
    End Function

    Public Sub New()
        'using pool is good to reuse objects, especially when you have to close socket connections repeatedly
        m_bUsePool = True
        m_nGlobalCount = 0
    End Sub

    'SocketPro will call this function to create a ClientPeer if neccessary.
    'you must override the function
    Protected Overrides Function GetPeerSocket(ByVal hSocket As Integer) As CClientPeer
        Trace.Assert(GetCurrentThreadId() = CSocketProServer.MainThreadID)

        'associate a CSOnePeer object into this service
        GetPeerSocket = New CSOnePeer
    End Function
End Class

Public Class CSOne
    Inherits CSocketProServer

    <DllImport("Kernel32.dll")> _
    Private Shared Function GetCurrentThreadId() As Int32
    End Function

    Private m_SOneSvs As CSOneSvs
    Private m_SThreeSvs As CSThreeSvs

    Protected Overrides Sub OnAccept(ByVal hSocket As Integer, ByVal nError As Integer)
        Trace.Assert(GetCurrentThreadId() = MainThreadID)
        Dim strMsg As String = "Socket accepted, and its handle = " & hSocket & ", Clients = " & CountOfClients & ", Error code = " & nError
        Console.WriteLine(strMsg)
    End Sub

    Protected Overrides Sub OnClose(ByVal hSocket As Integer, ByVal nError As Integer)
        Trace.Assert(GetCurrentThreadId() = MainThreadID)

        Console.WriteLine("Socket connection closed, and its handle = {0}. Error code = 0x{1:X}.", hSocket, nError)
    End Sub

    Protected Overrides Function OnIsPermitted(ByVal hSocket As Integer, ByVal lSvsID As Integer) As Boolean
        Trace.Assert(GetCurrentThreadId() = MainThreadID)

        Dim strUID As String = CBaseService.GetUserID(hSocket)

        'password is available ONLY IF authentication method to either amOwn or amMixed
        Dim strPassword As String = CBaseService.GetPassword(hSocket)

        Console.WriteLine("For service = {0}, User ID = {1}, Password = {2}", lSvsID, strUID, strPassword)

        Dim am As USOCKETLib.tagAuthenticationMethod = AuthenticationMethod

        If am = USOCKETLib.tagAuthenticationMethod.amOwn Or am = USOCKETLib.tagAuthenticationMethod.amMixed Then
            'do my own authentication
            Return IsAllowed(strUID, strPassword)
        End If

        OnIsPermitted = True
    End Function

    Private Sub ReuseLibraries()

        'those libraries are distributed in the directory ..\bin
        Dim hInst As IntPtr = CBaseService.AddALibrary("uodbsvr.dll", 0)
        If hInst.ToInt32() = 0 Then
            Console.WriteLine("library uodbsvr.dll not available.")
        End If

        hInst = CBaseService.AddALibrary("ufilesvr.dll", 0)
        If hInst.ToInt32() = 0 Then
            Console.WriteLine("library ufilesvr.dll not available.")
        End If

        hInst = CBaseService.AddALibrary("udemo.dll", 0)
        If hInst.ToInt32() = 0 Then
            Console.WriteLine("library udemo.dll not available.")
        End If
    End Sub

    Private Sub SetBuiltinChatService()
        Dim ok As Boolean = AddAChatGroup(1, "Group for SOne")

        Trace.Assert(ok)

        'Other clients will be notified when a new client join the group 1
        GroupsNotifiedWhenEntering = 1

        'Other clients will be notified when a new client leave the group 1
        GroupsNotifiedWhenExiting = 1

        '-1 means all of 32 groups
    End Sub

    Private Function IsAllowed(ByRef strUserID As String, ByRef strPassword As String) As Boolean
        If strPassword <> "PassOne" Then
            Return False
        End If
        Return (strUserID.ToLower() = "socketpro")
    End Function

    Private Sub SetSThreeSvs()

        m_SThreeSvs = New CSThreeSvs

        'Add a service into the SocketPro with threading type and what events are subscribed
        Dim ok As Boolean = m_SThreeSvs.AddMe(CDefines.sidSThreeSvs, tagEvent.eOnSlowRequestProcessed, tagThreadApartment.taNone)

        'Add slow requests
        ok = m_SThreeSvs.AddSlowRequest(CDefines.idGetALotItemsFromServer)
        ok = m_SThreeSvs.AddSlowRequest(CDefines.idSendingALotItemsToServer)
        ok = m_SThreeSvs.AddSlowRequest(CDefines.idGetItemsFromServerByDotNetSerializer)
        ok = m_SThreeSvs.AddSlowRequest(CDefines.idSendItemsToServerByDotNetSerializer)
    End Sub

    Private Sub SetSOneSvs()

        m_SOneSvs = New CSOneSvs

        'Add a service into the SocketPro with threading type and what events are subscribed
        Dim ok As Boolean = m_SOneSvs.AddMe(CSOneSvs.sidSOneSvs, 0, tagThreadApartment.taNone)

        'Add one slow request
        ok = m_SOneSvs.AddSlowRequest(CSOneSvs.idSleep)

    End Sub

    Public Sub Run()
        Do
            SharedAM = True

            'amMixed
            AuthenticationMethod = USOCKETLib.tagAuthenticationMethod.amMixed

            'limit the max number of connections to 2 for a client machine
            MaxConnectionsPerClient = 2

            'add one service into SocketPro server
            SetSOneSvs()

            SetSThreeSvs()

            SetBuiltinChatService()

            'use TLSv1 to secure all of data communication between a client and a SocketPro server
            'udacert.pem contains both key and certificate, which is distributed in the ..\bin as sample certificate and key
            'udacert.pem contains both key and certificate
            'UseSSL(USOCKETLib.tagEncryptionMethod.TLSv1, "udacert.pem", "udacert.pem")

            'reuse my high performance C/C++ libraries
            ReuseLibraries()

            'set for global callbacks for the events OnAccept, OnClose, OnIsPermitted
            AskForEvents(tagEvent.eOnAccept + tagEvent.eOnClose + tagEvent.eOnIsPermitted)

            'start listening socket
            If Not StartSocketProServer(20001, 64) Then
                Console.WriteLine("Can't start SocketPro server!")
                Console.WriteLine("Check if the port 20001 is available, please.")
                Console.WriteLine("Check if certificate and key files are available if SSL/TLSv1 is enabled.")
                Console.WriteLine("Error code = " & CClientPeer.LastSocketError())
                Exit Do
            Else
                Console.WriteLine("SocketPro successfully started at the port 20001")

                'start message pump. SocketPro requires it and runs at 100% non-blocking fashion
                'if a message already exist, don't call the following function
                StartMessagePump()

                'stop listening socket, and shutdown all of socket connections
                StopSocketProServer()
            End If
        Loop Until False
    End Sub

End Class




