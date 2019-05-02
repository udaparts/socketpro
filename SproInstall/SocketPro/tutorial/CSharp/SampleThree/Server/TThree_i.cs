public class TThreeConst
{
	//defines for service CTThree
	public const int sidCTThree = ((int)USOCKETLib.tagOtherDefine.odUserServiceIDMin + 10);

	public const short idGetOneItemCTThree = ((short)USOCKETLib.tagOtherDefine.odUserRequestIDMin + 0);
	public const short idSendOneItemCTThree = (idGetOneItemCTThree + 1);
	public const short idGetManyItemsCTThree = (idSendOneItemCTThree + 1);
	public const short idSendManyItemsCTThree = (idGetManyItemsCTThree + 1);

    public const short idGetBatchItemsCTThree = (idSendManyItemsCTThree + 1);
    public const short idSendBatchItemsCTThree = (idGetBatchItemsCTThree + 1);
}