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
    Friend Class Program
        <MTAThread()> _
        Shared Sub Main(ByVal args As String())
            Dim dbPool As CSocketPool(Of CAsynDBLite) = New CSocketPool(Of CAsynDBLite)()
            'start a pool socket with one thread, one socket, and one DB handler.
            If dbPool.StartSocketPool("localhost", 17001, "SocketPro", "PassOne", 1, 2, USOCKETLib.tagEncryptionMethod.NoEncryption, False) Then
                'get an instance of raw COM socket pool object for your debug
                Dim spc As USocketPoolClass = dbPool.GetUSocketPool()

                Dim DBLite0 As CAsynDBLite = dbPool.Lock()
                Dim DBLite1 As CAsynDBLite = dbPool.Lock()

                DBLite0.BeginBatch()
                DBLite0.ConnectDB("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=nwind3.mdb")
                DBLite0.OpenRowset("Select * from Customers", "Customers")
                DBLite0.CommitBatch(Nothing) 'send all of requests in batch for processing

                DBLite1.BeginBatch()
                DBLite1.ConnectDB("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=nwind3.mdb")
                DBLite1.OpenRowset("Select * from Products", "Products")
                DBLite1.CommitBatch(Nothing) 'send all of requests in batch for processing

                'cooperatively blocking
                DBLite0.GetAttachedClientSocket().WaitAll()

                'cooperatively blocking
                DBLite1.GetAttachedClientSocket().WaitAll()


                Dim dt0 As DataTable = DBLite0.CurrentDataTable
                Dim dt1 As DataTable = DBLite1.CurrentDataTable

                'return locked sockets back into pool for reuse
                dbPool.Unlock(DBLite0)
                dbPool.Unlock(DBLite1)

                dbPool.ShutdownPool()
            End If
        End Sub
    End Class
End Namespace
