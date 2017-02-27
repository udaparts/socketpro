Imports System
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports System.IO

Friend Class Program
    Shared Sub Main(ByVal args() As String)
        Dim cc As New CConnectionContext("localhost", 20901, "MyUserId", "MyPassword")
        Using spRf As New CSocketPool(Of RemotingFile)()
            Do
                Dim ok As Boolean = spRf.StartSocketPool(cc, 1, 1)
                If Not ok Then
                    Console.WriteLine("Can not connect to remote server")
                    Exit Do
                End If

                Dim rf As RemotingFile = spRf.Seek()

                'downloading test
                Dim progress As CStreamHelper.DProgress = Sub(sender, pos)
                                                              Console.WriteLine("Downloading progress = " & (pos * 100) / sender.DownloadingStreamSize)
                                                          End Sub
                AddHandler rf.StreamHelper.Progress, progress

                Console.WriteLine("Input a remote file to download ......")
                Dim RemoteFile As String = Console.ReadLine()
                Dim LocalFile As String = "spfile.test"
                Dim s As Stream = New FileStream(LocalFile, FileMode.Append)
                Dim res As String = rf.StreamHelper.Download(s, RemoteFile)
                If res.Length = 0 AndAlso rf.WaitAll() Then
                    Console.WriteLine("Successful to download file " & RemoteFile)
                Else
                    Console.WriteLine("Failed to download file " & RemoteFile)
                End If
                s.Close()
                RemoveHandler rf.StreamHelper.Progress, progress 'remove the callback

                'uploading test
                RemoteFile = "spfile.testr"
                s = New FileStream(LocalFile, FileMode.Open)
                Dim FileSize As ULong = CULng(s.Length)
                AddHandler rf.StreamHelper.Progress, Sub(sender, pos)
                                                         Console.WriteLine("Uploading progress = " & (pos * 100) / FileSize)
                                                     End Sub
                res = rf.StreamHelper.Upload(s, RemoteFile)
                If res = "" AndAlso rf.WaitAll() Then
                    Console.WriteLine("Successful to upload file " & LocalFile)
                Else
                    Console.WriteLine("Failed to upload file " & LocalFile)
                End If
                s.Close()
            Loop While False
            Console.WriteLine("Press key ENTER to shutdown the demo application ......")
            Console.ReadLine()
        End Using
    End Sub
End Class

