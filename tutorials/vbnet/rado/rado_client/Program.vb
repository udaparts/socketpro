Imports SocketProAdapter.ClientSide

Friend Class Program
    Shared Sub Main(ByVal args() As String)
        'set a two-dimensional array of socket connection contexts
#If PocketPC Then
		Dim ccs(0, 0) As CConnectionContext
#Else
        Dim ccs(System.Environment.ProcessorCount - 1, 0) As CConnectionContext
#End If
        Dim threads As Integer = ccs.GetLength(0)
        Dim sockets_per_thread As Integer = ccs.GetLength(1)
        For n As Integer = 0 To threads - 1
            For j As Integer = 0 To sockets_per_thread - 1
                Dim ipAddress As String
                If j = 0 Then
                    ipAddress = "192.168.1.109"
                Else
                    ipAddress = "localhost"
                End If
                ccs(n, j) = New CConnectionContext(ipAddress, 20901, "adoclient", "password4AdoClient")
            Next j
        Next n

        Using spAdo As New CSocketPool(Of RAdo)(True) 'true -- automatic reconnecting
            'start a pool of sockets
            If Not spAdo.StartSocketPool(ccs) Then
                Console.WriteLine("No socket connection")
                Return
            End If

            Dim ado As RAdo = spAdo.Seek()

            'process two requests one by one with synchronous communication style
            Dim ds As DataSet = ado.GetDataSet("select * from dimProduct", "select * from dimAccount")
            Console.WriteLine("Dataset returned with {0} tables", ds.Tables.Count)
            Dim dt As DataTable = ado.GetDataTable("select * from dimCustomer")
            Console.WriteLine("Datatable returned with columns = {0}, rows = {1}", dt.Columns.Count, dt.Rows.Count)

            'send two requests in parallel with asynchronous communication style
            Dim ado1 As RAdo = spAdo.Seek()
            Dim ok As Boolean = ado1.SendRequest(radoConst.idGetDataTableRAdo, "select * from dimCustomer", Sub(ar) Console.WriteLine("Datatable returned with columns = {0}, rows = {1}", ado1.CurrentDataTable.Columns.Count, ado1.CurrentDataTable.Rows.Count))

            Dim ado2 As RAdo = spAdo.Seek()
            ok = ado2.SendRequest(radoConst.idGetDataSetRAdo, "select * from dimProduct", "select * from dimAccount", Sub(ar) Console.WriteLine("Dataset returned with {0} tables", ado2.CurrentDataSet.Tables.Count))
            'ok = ado1.WaitAll() && ado2.WaitAll();
            Console.WriteLine("Press key ENTER to shutdown the demo application ......")
            Console.ReadLine()
        End Using
    End Sub
End Class

