public class RemoteSumConst
{
	//defines for service RemSum
	public const int sidRemSum = ((int)USOCKETLib.tagOtherDefine.odUserServiceIDMin + 10);

	public const short idDoSumRemSum = ((short)USOCKETLib.tagOtherDefine.odUserRequestIDMin + 0);
	public const short idPauseRemSum = (idDoSumRemSum + 1);
	public const short idRedoSumRemSum = (idPauseRemSum + 1);
	public const short idReportProgress = (idRedoSumRemSum + 1);
}