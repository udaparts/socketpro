Public Class AsyncAdoWebConst
	'defines for service CMyAdoHandler
    Public Const sidCAsyncAdo As Integer = (CInt(USOCKETLib.tagOtherDefine.odUserServiceIDMin) + 303)

    Public Const idGetDataTableCAsyncAdo As Short = (CShort(USOCKETLib.tagOtherDefine.odUserRequestIDMin) + 0)
	Public Const idExecuteNoQueryCAsyncAdo As Short = (idGetDataTableCAsyncAdo + 1)
End Class