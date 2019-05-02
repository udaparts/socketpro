public class RAdoConst
{
	//defines for service CRAdo
	public const int sidCRAdo = ((int)USOCKETLib.tagOtherDefine.odUserServiceIDMin + 202);

	public const short idGetDataSetCRAdo = ((short)USOCKETLib.tagOtherDefine.odUserRequestIDMin + 0);
	public const short idGetDataReaderCRAdo = (idGetDataSetCRAdo + 1);
	public const short idSendDataSetCRAdo = (idGetDataReaderCRAdo + 1);
	public const short idSendDataReaderCRAdo = (idSendDataSetCRAdo + 1);
}