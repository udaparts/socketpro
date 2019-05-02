Public Module TOneConst
	'defines for service CTOne
	public const sidCTOne as UInteger = (SocketProAdapter.BaseServiceID.sidReserved + 30)

	public Const idQueryCountCTOne as UShort = (SocketProAdapter.tagBaseRequestID.idReservedTwo + 1)
	public Const idQueryGlobalCountCTOne as UShort = (idQueryCountCTOne + 1)
	public Const idQueryGlobalFastCountCTOne as UShort = (idQueryGlobalCountCTOne + 1)
	public Const idSleepCTOne as UShort = (idQueryGlobalFastCountCTOne + 1)
	public Const idEchoCTOne as UShort = (idSleepCTOne + 1)
	public Const idEchoExCTOne as UShort = (idEchoCTOne + 1)
End Module