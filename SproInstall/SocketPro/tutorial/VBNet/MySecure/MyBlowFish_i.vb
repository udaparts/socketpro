Imports Microsoft.VisualBasic
Imports System
Public Class MyBlowFishConst
	'defines for service CMySecure
	Public Const sidCMySecure As Integer = (CInt(Fix(USOCKETLib.tagOtherDefine.odUserServiceIDMin)) + 101)

	Public Const idOpenCMySecure As Short = (CShort(Fix(USOCKETLib.tagOtherDefine.odUserRequestIDMin)) + 0)
	Public Const idBeginTransCMySecure As Short = (idOpenCMySecure + 1)
	Public Const idExecuteNoQueryCMySecure As Short = (idBeginTransCMySecure + 1)
	Public Const idCommitCMySecure As Short = (idExecuteNoQueryCMySecure + 1)
End Class