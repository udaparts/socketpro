Public Class RemoteSumConst
	'defines for service RemSum
    Public Const sidRemSum As Integer = (USOCKETLib.tagOtherDefine.odUserServiceIDMin + 10)

    Public Const idDoSumRemSum As Short = (USOCKETLib.tagOtherDefine.odUserRequestIDMin + 0)
    Public Const idPauseRemSum As Short = (idDoSumRemSum + 1)
    Public Const idRedoSumRemSum As Short = (idPauseRemSum + 1)
	Public Const idReportProgress As Short = (idRedoSumRemSum + 1)
End Class