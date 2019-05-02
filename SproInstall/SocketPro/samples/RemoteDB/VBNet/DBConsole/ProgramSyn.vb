Imports Microsoft.VisualBasic
Imports System
Imports System.Collections.Generic
Imports System.Text
Imports System.Data

Imports UDBLib
Imports USOCKETLib
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports SocketProAdapter.ClientSide.RemoteDB

Namespace DBConsole
	Friend Class CMyDBSyn
		Inherits CAsynDBLite
		Public Function Connect(ByVal strConn As String) As Boolean
            ConnectDB(strConn)
            GetAttachedClientSocket().WaitAll()
            Return DBConnected
        End Function

        Public Sub Disconnect()
            DisconnectDB()
            GetAttachedClientSocket().WaitAll()
        End Sub

        Public Overloads Function OpenRowset(ByVal strSQL As String) As Boolean
            OpenRowset(strSQL, "MyTable", UDBLib.tagCursorType.ctStatic, CAsynDBLite.Readonly, 20, -1)
            GetAttachedClientSocket().WaitAll()
            Return (CurrentDataTable.Columns.Count <> 0)
        End Function

        Public Function IsEOF() As Boolean
            Return (CurrentDataTable.Rows.Count = 0)
        End Function

        Public Sub MoveFirst()
            FirstBatch()
            GetAttachedClientSocket().WaitAll()
        End Sub

        Public Sub MoveNext()
            MoveNext(0)
        End Sub

        Public Sub MoveNext(ByVal nSkip As Integer)
            NextBatch(nSkip)
            GetAttachedClientSocket().WaitAll()
        End Sub

        Public Sub MovePrev()
            MoveNext(-2)
        End Sub

        Public Sub MoveLast()
            LastBatch()
            GetAttachedClientSocket().WaitAll()
        End Sub
	End Class

	Friend Class Program
		<MTAThread> _
		Shared Sub Main(ByVal args As String())
			Dim dbPool As CSocketPool(Of CMyDBSyn) = New CSocketPool(Of CMyDBSyn)()

			'start a pool socket with one thread, one socket, and one DB handler.
            If dbPool.StartSocketPool("localhost", 17001, "SocketPro", "PassOne", 1, 1, USOCKETLib.tagEncryptionMethod.NoEncryption, False) Then
                Dim myDB As CMyDBSyn = dbPool.Lock()
                Dim nCount As Integer = 0
                '                myDB.ConnectDB("Provider=sqlncli;Data Source=localhost\\sqlexpress;Initial Catalog=northwind;Integrated Security=SSPI");
                If myDB.Connect("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=nwind3.mdb") Then
                    If myDB.OpenRowset("Select * from Orders") Then
                        Do While Not myDB.IsEOF()
                            nCount += myDB.CurrentDataTable.Rows.Count

                            ' process your batch records here

                            myDB.MoveNext()
                        Loop
                    End If
                End If
                dbPool.ShutdownPool()
                Console.WriteLine("Recods fetched = " & nCount.ToString())
                Console.WriteLine("Presss the key <ENTER> to exit the application")
                Console.ReadLine()
            End If
		End Sub
	End Class
End Namespace
