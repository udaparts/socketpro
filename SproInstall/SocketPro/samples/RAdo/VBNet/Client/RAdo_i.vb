Public Class RAdoConst
	'defines for service CRAdo
    Public Const sidCRAdo As Integer = (CInt(USOCKETLib.tagOtherDefine.odUserServiceIDMin) + 202)

    Public Const idGetDataSetCRAdo As Short = (CShort(USOCKETLib.tagOtherDefine.odUserRequestIDMin) + 0)
	Public Const idGetDataReaderCRAdo As Short = (idGetDataSetCRAdo + 1)
	Public Const idSendDataSetCRAdo As Short = (idGetDataReaderCRAdo + 1)
	Public Const idSendDataReaderCRAdo As Short = (idSendDataSetCRAdo + 1)
End Class