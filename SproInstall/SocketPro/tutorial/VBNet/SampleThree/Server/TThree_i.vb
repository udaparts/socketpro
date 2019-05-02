Public Class TThreeConst
	'defines for service CTThree
    Public Const sidCTThree As Integer = (CInt(USOCKETLib.tagOtherDefine.odUserServiceIDMin) + 10)

    Public Const idGetOneItemCTThree As Short = (CShort(USOCKETLib.tagOtherDefine.odUserRequestIDMin) + 0)
	Public Const idSendOneItemCTThree As Short = (idGetOneItemCTThree + 1)
	Public Const idGetManyItemsCTThree As Short = (idSendOneItemCTThree + 1)
	Public Const idSendManyItemsCTThree As Short = (idGetManyItemsCTThree + 1)

	Public Const idGetBatchItemsCTThree As Short = (idSendManyItemsCTThree + 1)
	Public Const idSendBatchItemsCTThree As Short = (idGetBatchItemsCTThree + 1)
End Class