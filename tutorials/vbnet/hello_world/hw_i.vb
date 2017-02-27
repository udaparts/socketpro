Public NotInheritable Class hwConst

	Private Sub New()
	End Sub

	'defines for service HelloWorld
	Public Const sidHelloWorld As UInteger = (SocketProAdapter.BaseServiceID.sidReserved + 1)

	Public Const idSayHelloHelloWorld As UShort = (CUShort(SocketProAdapter.tagBaseRequestID.idReservedTwo) + 1)
	Public Const idSleepHelloWorld As UShort = (idSayHelloHelloWorld + 1)
	Public Const idEchoHelloWorld As UShort = (idSleepHelloWorld + 1)
End Class