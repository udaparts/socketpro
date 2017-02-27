Imports System
Imports System.Collections.Generic
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide

Friend Class Program
    Private Shared Sub SetWorkDirectory()
        Select Case System.Environment.OSVersion.Platform
            Case PlatformID.Win32NT, PlatformID.Win32S, PlatformID.Win32Windows
                CClientSocket.QueueConfigure.WorkDirectory = "c:\sp_test\"
            Case PlatformID.WinCE
            Case Else
                CClientSocket.QueueConfigure.WorkDirectory = "/home/yye/sp_test/"
        End Select
    End Sub
    Shared Sub Main(ByVal args() As String)
        CClientSocket.QueueConfigure.MessageQueuePassword = "MyQPassword"
        SetWorkDirectory()
        Dim rs As New ReplicationSetting()
        Using hw As New CReplication(Of HelloWorld)(rs)
            Dim ConnQueue As New Dictionary(Of String, CConnectionContext)()
            Dim cc As New CConnectionContext("127.0.0.1", 20901, "replication", "p4localhost")
#If PocketPC Then
#Else
            ConnQueue("Tolocal") = cc
#End If
            cc = New CConnectionContext("192.168.1.109", 20901, "remote_rep", "PassOne")
            ConnQueue("ToLinux") = cc
            Dim ok As Boolean = hw.Start(ConnQueue, "hw_root_queue_name")
            hw.StartJob()
            ok = hw.Send(hwConst.idSayHelloHelloWorld, "David", "Young")
            ok = hw.Send(hwConst.idEchoHelloWorld, CMyStruct.MakeOne())
            hw.EndJob()
            Console.WriteLine("Press key ENTER to shut down the application ......")
            Console.ReadLine()
        End Using
    End Sub
End Class

