Imports System
Imports SocketProAdapter.ClientSide


Friend Class Program
    Shared Sub Main(ByVal args() As String)
        Console.WriteLine("Remote SocketPro file streaming server:")
        Dim cc As New CConnectionContext(Console.ReadLine(), 20901, "MyUserId", "MyPassword")
        Using spRf As New CSocketPool(Of CStreamingFile)()
            Dim ok As Boolean = spRf.StartSocketPool(cc, 1, 1)
            If Not ok Then
                Console.WriteLine("Can not connect to remote server, and press key ENTER to shutdown the demo application ......")
                Console.ReadLine()
                Return
            End If
            Dim rf As CStreamingFile = spRf.Seek()
            Console.WriteLine("Input a remote file to download ......")
            Dim RemoteFile As String = Console.ReadLine()
            Dim LocalFile As String = "spfile.test"
            Dim cbDownload As CStreamingFile.DDownload = Sub(file, res, errMsg)
                                                             If res <> 0 Then
                                                                 Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg)
                                                             Else
                                                                 Console.WriteLine("Downloading {0} completed", file.RemoteFile)
                                                             End If
                                                         End Sub
            Dim cbDProgress As CStreamingFile.DTransferring = Sub(file, downloaded)
                                                                  'downloading progress
                                                                  Console.WriteLine("Downloading rate: {0}%", (downloaded * 100) / file.FileSize)
                                                              End Sub
            ok = rf.Download(LocalFile, RemoteFile, cbDownload, cbDProgress)
            'ok = rf.WaitAll()

            Dim cbUpload As CStreamingFile.DUpload = Sub(file, res, errMsg)
                                                         If res <> 0 Then
                                                             Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg)
                                                         Else
                                                             Console.WriteLine("Uploading {0} completed", file.RemoteFile)
                                                         End If
                                                     End Sub
            Dim cbUProgress As CStreamingFile.DTransferring = Sub(file, uploaded)
                                                                  'uploading progress
                                                                  Console.WriteLine("Uploading rate: {0}%", (uploaded * 100) / file.FileSize)
                                                              End Sub
            RemoteFile += ".copy"
            ok = rf.Upload(LocalFile, RemoteFile, cbUpload, cbUProgress)
            ok = rf.WaitAll()
            Console.WriteLine("Press key ENTER to shutdown the demo application ......")
            Console.ReadLine()
        End Using
    End Sub
End Class

