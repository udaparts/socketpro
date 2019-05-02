public class MyBlowFishConst
{
	//defines for service CMySecure
	public const int sidCMySecure = ((int)USOCKETLib.tagOtherDefine.odUserServiceIDMin + 101);

	public const short idOpenCMySecure = ((short)USOCKETLib.tagOtherDefine.odUserRequestIDMin + 0);
	public const short idBeginTransCMySecure = (idOpenCMySecure + 1);
	public const short idExecuteNoQueryCMySecure = (idBeginTransCMySecure + 1);
	public const short idCommitCMySecure = (idExecuteNoQueryCMySecure + 1);
}