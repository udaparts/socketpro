public class TOneConst
	'defines for service CTOne
	public const sidCTOne as Integer = (USOCKETLib.tagOtherDefine.odUserServiceIDMin + 20)

	public Const idQueryCountCTOne as Short = (USOCKETLib.tagOtherDefine.odUserRequestIDMin + 0)
	public Const idQueryGlobalCountCTOne as Short = (idQueryCountCTOne + 1)
	public Const idQueryGlobalFastCountCTOne as Short = (idQueryGlobalCountCTOne + 1)
	public Const idSleepCTOne as Short = (idQueryGlobalFastCountCTOne + 1)
	public Const idEchoCTOne as Short = (idSleepCTOne + 1)
End Class