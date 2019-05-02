public static class TOneConst
{
	//defines for service CTOne
	public const int sidCTOne = ((int)USOCKETLib.tagOtherDefine.odUserServiceIDMin + 20);

	public const short idQueryCountCTOne = ((short)USOCKETLib.tagOtherDefine.odUserRequestIDMin + 0);
	public const short idQueryGlobalCountCTOne = (idQueryCountCTOne + 1);
	public const short idQueryGlobalFastCountCTOne = (idQueryGlobalCountCTOne + 1);
	public const short idSleepCTOne = (idQueryGlobalFastCountCTOne + 1);
	public const short idEchoCTOne = (idSleepCTOne + 1);
}