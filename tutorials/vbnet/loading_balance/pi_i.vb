Public NotInheritable Class piConst

	Private Sub New()
	End Sub

	'defines for service Pi
	Public Const sidPi As UInteger = (SocketProAdapter.BaseServiceID.sidReserved + 5)
	Public Const sidPiWorker As UInteger = sidPi + 1

	Public Const idComputePi As UShort = (CUShort(SocketProAdapter.tagBaseRequestID.idReservedTwo) + 1)
End Class