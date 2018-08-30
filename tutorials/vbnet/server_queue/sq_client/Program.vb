Imports System
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide

Class Program
    Shared TEST_QUEUE_KEY As Byte()
    Const idMessage0 As UShort = CUShort(tagBaseRequestID.idReservedTwo) + 100
    Const idMessage1 As UShort = CUShort(tagBaseRequestID.idReservedTwo) + 101
    Const idMessage2 As UShort = CUShort(tagBaseRequestID.idReservedTwo) + 102

    Shared Sub New()
        TEST_QUEUE_KEY = System.Text.Encoding.UTF8.GetBytes("queue_name_0")
    End Sub

    Private Shared Function TestEnqueue(aq As CAsyncQueue) As Boolean
        Dim ok As Boolean = True
        Console.WriteLine("Going to enqueue 1024 messages ......")
        For n As Integer = 0 To 1023
            Dim str As String = n & " Object test"
            Dim idMessage As UShort
            Select Case n Mod 3
                Case 0
                    idMessage = idMessage0
                Case 1
                    idMessage = idMessage1
                Case Else
                    idMessage = idMessage2
            End Select
            'enqueue two unicode strings and one int
            ok = aq.Enqueue(TEST_QUEUE_KEY, idMessage, "SampleName", str, n)
            If Not ok Then
                Exit For
            End If
        Next
        Return ok
    End Function

    Private Shared Sub TestDequeue(aq As CAsyncQueue)
        'prepare callback for parsing messages dequeued from server side
        AddHandler aq.ResultReturned, Function(sender, idReq, q)
                                          Dim processed As Boolean = True
                                          Select Case idReq
                                              Case idMessage0, idMessage1, idMessage2
                                                  Console.Write("message id={0}", idReq)
                                                  If True Then
                                                      Dim name As String = "", str As String = ""
                                                      Dim index As Integer = 0
                                                      'parse a dequeued message which should be the same as the above enqueued message (two unicode strings and one int)
                                                      q.Load(name).Load(str).Load(index)
                                                      Console.WriteLine(", name={0}, str={1}, index={2}", name, str, index)
                                                  End If
                                              Case Else
                                                  processed = False
                                          End Select
                                          Return processed
                                      End Function

        'prepare a callback for processing returned result of dequeue request
        Dim d As CAsyncQueue.DDequeue = Sub(asyncqueue, messageCount, fileSize, messages, bytes)
                                            Console.WriteLine("Total message count={0}, queue file size={1}, messages dequeued={2}, message bytes dequeued={3}", messageCount, fileSize, messages, bytes)
                                            If messageCount > 0 Then
                                                'there are more messages left at server queue, we re-send a request to dequeue
                                                asyncqueue.Dequeue(TEST_QUEUE_KEY, asyncqueue.LastDequeueCallback)
                                            End If
                                        End Sub

        Console.WriteLine("Going to dequeue messages ......")
        Dim ok As Boolean = aq.Dequeue(TEST_QUEUE_KEY, d)

        'optionally, add one extra to improve processing concurrency at both client and server sides for better performance and through-output
        ok = aq.Dequeue(TEST_QUEUE_KEY, d)
    End Sub

    Shared Sub Main(ByVal args() As String)
        Console.WriteLine("Remote host: ")
        Dim host As String = Console.ReadLine()
        Dim cc As New CConnectionContext(host, 20901, "async_queue_client", "pwd_for_async_queue")
        Using spAq As New CSocketPool(Of CAsyncQueue)()
            If Not spAq.StartSocketPool(cc, 1, 1) Then
                Console.WriteLine("Failed in connecting to remote async queue server")
                Console.WriteLine("Press any key to close the application ......")
                Console.Read()
                Return
            End If
            Dim aq As CAsyncQueue = spAq.Seek()

            TestEnqueue(aq)
            TestDequeue(aq)

            Console.WriteLine("Press any key to close the application ......")
            Console.Read()
        End Using
    End Sub
End Class