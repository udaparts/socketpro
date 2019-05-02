Public Class PerfConst
	'defines for service CPerf
    Public Const sidCPerf As Integer = (CInt(USOCKETLib.tagOtherDefine.odUserServiceIDMin) + 1111)

    Public Const idMyEchoCPerf As Short = (CShort(USOCKETLib.tagOtherDefine.odUserRequestIDMin) + 0)
	Public Const idOpenRecords As Short = idMyEchoCPerf + 1
End Class