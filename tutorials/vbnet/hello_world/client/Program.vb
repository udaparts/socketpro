Imports SocketProAdapter.ClientSide

Friend Class Program
    Shared Sub Main(ByVal args() As String)
        Dim cc As New CConnectionContext("localhost", 20901, "hwClientUserId", "password4hwClient")
        Using spHw As New CSocketPool(Of HelloWorld)(True) 'true -- automatic reconnecting
            'optionally start a persistent queue at client side to ensure auto failure recovery and once-only delivery
            'spHw.QueueName = "helloworld";
            Dim ok As Boolean = spHw.StartSocketPool(cc, 1, 1)
            Dim hw As HelloWorld = spHw.Seek() 'or HelloWorld hw = spHw.Lock();

            'process requests one by one synchronously
            Console.WriteLine(hw.SayHello("Jone", "Dole"))
            hw.Sleep(5000)
            Dim msOriginal As CMyStruct = CMyStruct.MakeOne()
            Dim ms As CMyStruct = hw.Echo(msOriginal)

            'asynchronously process multiple requests with inline batching for best network efficiency
            ok = hw.SendRequest(hwConst.idSayHelloHelloWorld, "Jack", "Smith", Sub(ar)
                                                                                   Dim ret As String = Nothing
                                                                                   ar.Load(ret)
                                                                                   Console.WriteLine(ret)
                                                                               End Sub)
            Dim arh As CAsyncServiceHandler.DAsyncResultHandler = Nothing
            ok = hw.SendRequest(hwConst.idSleepHelloWorld, CInt(5000), arh)
            ok = hw.SendRequest(hwConst.idEchoHelloWorld, msOriginal, Sub(ar)
                                                                          ar.Load(ms)
                                                                      End Sub)
            ok = hw.WaitAll()
            Console.WriteLine("Press ENTER key to shutdown the demo application ......")
            Console.ReadLine()
        End Using
    End Sub
End Class
