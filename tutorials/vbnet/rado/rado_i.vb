Public NotInheritable Class radoConst

	Private Sub New()
	End Sub

	'defines for service RAdo
	Public Const sidRAdo As UInteger = (SocketProAdapter.BaseServiceID.sidReserved + 4)

	Public Const idGetDataSetRAdo As UShort = (CUShort(SocketProAdapter.tagBaseRequestID.idReservedTwo) + 1)
	Public Const idGetDataTableRAdo As UShort = (idGetDataSetRAdo + 1)
End Class