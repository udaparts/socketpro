
Imports System.Diagnostics
Imports System.Collections.Generic
Imports SocketProAdapter

Namespace TestUQueue

    ' <Serializable()> _
    Public Class CNoSerialization
        Public Data As Long
        Public Str As String
        Public List As List(Of String)
    End Class

    <Serializable()> _
    Public Class CTestItem2
        Public m_dt As DateTime
        Public m_lData As Long
        Public m_strUID As String
        Public m_ts As TimeSpan
        Public m_offset As DateTimeOffset
        Public List As New List(Of String)()
    End Class

    Public Class CTestItem : Implements IUSerializer
        Public Sub SaveTo(ByVal UQueue As CUQueue) Implements IUSerializer.SaveTo
            Dim count As Integer
            If List Is Nothing Then
                List = New List(Of String)()
            End If

            'Make sure both SaveTo and LoadFrom can match each other in sequence for all of members!

            UQueue.Push(m_lData)
            UQueue.Save(m_strUID)
            UQueue.Push(m_dt)

            count = List.Count
            UQueue.Push(count)
            For Each str As String In List
                UQueue.Save(str)
            Next

            UQueue.Push(m_ts)
            UQueue.Push(m_offset)
        End Sub

        Public Function LoadFrom(ByVal UQueue As CUQueue) As Integer Implements IUSerializer.LoadFrom
            Dim count As Integer
            Dim str As String = Nothing
            List.Clear()

            'Make sure both SaveTo and LoadFrom can match each other in sequence for all of members!

            Dim start As Integer = UQueue.GetSize()
            UQueue.Pop(m_lData)
            UQueue.Load(m_strUID)
            UQueue.Pop(m_dt)

            UQueue.Pop(count)
            While count > 0
                UQueue.Load(str)
                List.Add(str)
                count -= 1
            End While

            UQueue.Pop(m_ts)
            UQueue.Pop(m_offset)

            Return start - UQueue.GetSize()
        End Function

        Public m_dt As DateTime
        Public m_lData As Long
        Public m_strUID As String
        Public m_ts As TimeSpan
        Public m_offset As DateTimeOffset
        Public List As New List(Of String)()
    End Class

    Class Program
        Public Sub TestIUSerilizer()
            Dim inItem As New CTestItem()
            Dim outOut As New CTestItem()
            Using su As New CScopeUQueue()
                inItem.List.Add("test")
                inItem.List.Add("me")
                inItem.m_lData = 56781234567L
                inItem.m_strUID = "SoCketpro"
                inItem.m_dt = DateTime.Now
                inItem.m_ts = New TimeSpan(1234567890)
                inItem.m_offset = New DateTimeOffset(5, 2, 4, 10, 50, 40, _
                 New TimeSpan())

                'use IUSerilizer
                su.Save(inItem)
                su.Load(outOut)
                Debug.Assert(su.UQueue.Size = 0)
            End Using

            'Memory released back into memory pool for reuse
        End Sub

        Public Sub TestSerializable()
            Dim inItem As New CTestItem2()
            Dim outOut As New CTestItem2()
            Using su As New CScopeUQueue()
                inItem.List.Add("test")
                inItem.List.Add("me")
                inItem.m_lData = 56781234567L
                inItem.m_strUID = "SoCketpro"
                inItem.m_dt = DateTime.Now
                inItem.m_ts = New TimeSpan(1234567890)
                inItem.m_offset = New DateTimeOffset(5, 2, 4, 10, 50, 40, _
                 New TimeSpan())

                'use MS native serialization
                su.Save(inItem)
                su.Load(outOut)
                Debug.Assert(su.UQueue.Size = 0)
            End Using

            'Memory released back into memory pool for reuse
        End Sub

        Public Sub TestSimpleOnes()
            Dim nOut As Integer = 0, nData As Integer = 1234
            Dim sOut As Short = 0, sData As Short = 256
            Dim dtOut As DateTime = Nothing
            Dim dt As DateTime = DateTime.Now
            Dim strOut As String = Nothing, str As String = "MyTest"
            Dim bOut As Boolean = False, b As Boolean = True
            Dim UQueue As New CUQueue()
            UQueue.Push(nData)
            UQueue.Push(sData)
            UQueue.Push(dt)
            UQueue.Save(str)
            UQueue.Push(b)

            UQueue.Pop(nOut)
            UQueue.Pop(sOut)
            UQueue.Pop(dtOut)
            UQueue.Load(strOut)
            UQueue.Pop(bOut)

            Debug.Assert(UQueue.GetSize() = 0)
        End Sub

        Public Sub TestNotSupport()
            Dim nsOut As CNoSerialization = Nothing, ns As New CNoSerialization()
            ns.Data = 12345
            ns.Str = "Test"
            ns.List = New List(Of String)()
            ns.List.Add("Test")
            ns.List.Add("Me")

            Try
                Using su As New CScopeUQueue()
                    su.Save(ns)
                    su.Load(nsOut)
                End Using
            Catch ex As Exception
                Console.WriteLine(ex.Message)
                Console.WriteLine(ex.StackTrace)
            End Try
        End Sub

        Public Shared Sub Main()
            Dim p As New Program()
            p.TestSimpleOnes()
            p.TestIUSerilizer()
            p.TestSerializable()
            p.TestNotSupport()
        End Sub
    End Class
End Namespace
