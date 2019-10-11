Imports SocketProAdapter
Imports SocketProAdapter.ServerSide

Public Class DBPushPeer
	Inherits CAdoClientPeer

	Public Sub New()
		AddHandler OnAdonetLoaded, AddressOf OnAdoNet
	End Sub

	<RequestAttr(CAdoSerializationHelper.idDmlTriggerMessage, True)>
	Private Sub OnDmlTriggerMessage(ByVal triggerEvent As Integer, ByVal fullDbObjectName As String, ByVal param As Object)
		Console.WriteLine("FullDbTableName = {0}, event = {1}, param = {2}", fullDbObjectName, triggerEvent, param)
	End Sub

	<RequestAttr(CAdoSerializationHelper.idRecordsetName, True)>
	Private Sub OnQueryMessage(ByVal recordsetName As String)
		Console.WriteLine("Recordset = " & recordsetName)
	End Sub

	<RequestAttr(CAdoSerializationHelper.idDbEventTriggerMessage, True)>
	Private Sub OnDbTriggerMessage(ByVal triggerEvent As Integer, ByVal instance As String, ByVal eventData As String)
		Console.WriteLine("DB Logon event = " & instance & ", eventData = " & eventData)
	End Sub

	Private Sub OnAdoNet(ByVal peer As CAdoClientPeer, ByVal reqId As UShort)
		Select Case reqId
			Case CAdoSerializationHelper.idDataReaderRecordsArrive
				If AdoSerializer.CurrentDataTable.Rows.Count > 10 * 1024 Then
					AdoSerializer.CurrentDataTable.Clear()
				End If
			Case Else
		End Select

		If reqId = CAdoSerializationHelper.idEndDataReader Then
			If AdoSerializer.CurrentDataTable.Rows.Count > 100 Then
				Console.WriteLine("Table rowset size = " & AdoSerializer.CurrentDataTable.Rows.Count)
			Else
				For Each dr As DataRow In AdoSerializer.CurrentDataTable.Rows
					Dim n As Integer = 0
					For Each obj As Object In dr.ItemArray
						If n > 0 Then
							Console.Write("," & ControlChars.Tab)
						End If
						Console.Write(obj.ToString())
						n += 1
					Next obj
					Console.WriteLine()
				Next dr
			End If
		End If
	End Sub
End Class