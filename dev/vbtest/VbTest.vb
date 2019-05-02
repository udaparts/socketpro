
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide

Module VbTest

    Sub Main()
        Dim cc As CConnectionContext = New CConnectionContext()
        'cc.EncrytionMethod = tagEncryptionMethod.TLSv1;
        cc.Port = 20901
        cc.UserId = ".NetUserId"
        cc.Password = "NetSecret"
        Console.WriteLine("Input a host .....")
        cc.Host = Console.ReadLine()
        Using spOne As New CSocketPool(Of CTOne)
            Dim ok As Boolean = spOne.StartSocketPool(cc, 1, 1)
            Dim handle As CTOne = spOne.Lock()
            Dim obj As Object = handle.Echo(25.57)
            Dim count As Integer = handle.QueryCount()
            count = handle.QueryGlobalCount()
            count = handle.QueryGlobalFastCount()

        End Using

    End Sub

End Module
