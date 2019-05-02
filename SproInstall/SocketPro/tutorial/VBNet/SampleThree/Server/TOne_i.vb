Public Class TOneConst
	'defines for service CTOne
    Public Const sidCTOne As Integer = (CInt(USOCKETLib.tagOtherDefine.odUserServiceIDMin) + 20)
    Public Const idQueryCountCTOne As Short = (CShort(USOCKETLib.tagOtherDefine.odUserRequestIDMin) + 0)
	Public Const idQueryGlobalCountCTOne As Short = (idQueryCountCTOne + 1)
	Public Const idQueryGlobalFastCountCTOne As Short = (idQueryGlobalCountCTOne + 1)
	Public Const idSleepCTOne As Short = (idQueryGlobalFastCountCTOne + 1)
	Public Const idEchoCTOne As Short = (idSleepCTOne + 1)
End Class