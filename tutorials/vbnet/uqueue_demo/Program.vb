Imports System
Imports SocketProAdapter

Friend Class Program
    Private Shared Sub Save(ByVal ms As CMyStruct, ByVal q As CUQueue)
        ms.SaveTo(q)
    End Sub

    Private Shared Function Load(ByVal q As CUQueue) As CMyStruct
        Dim ms As New CMyStruct()
        ms.LoadFrom(q)
        Return ms
    End Function

    Shared Sub Main(ByVal args() As String)
        Using su As New CScopeUQueue()
            Dim msOriginal As CMyStruct = CMyStruct.MakeOne()

            msOriginal.SaveTo(su.UQueue)

            Dim ms As CMyStruct = Load(su.UQueue)
            System.Diagnostics.Debug.Assert(su.UQueue.GetSize() = 0)

            'check if both msOriginal and ms are equal in value.
        End Using
    End Sub
End Class

