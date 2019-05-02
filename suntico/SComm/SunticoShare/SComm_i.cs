using System.Data;

namespace Suntico
{
    public enum StringObjectType
    {
        RegularText = 0,
        Json,
        Xml,
    }

    public enum ConnectionStatus
    {
        Closed = 0,
        Opened
    }

    public delegate void DChannelsClosed();
    public delegate void DChannelsOpened();
    public delegate void DFailover();
    public delegate void DBeginTransAtCloud(long Clue);
    public delegate void DCommitAtCloud(long Clue);
    public delegate void DRequestCompletedAtCloud();
   
    public delegate void DStartTrans(long Clue);
    public delegate void DEndTrans(long Clue);
    public delegate void DGeneralMessage(string msg, int Group, int ServiceId);
    public delegate void DDataReader(DataTable dt);
    public delegate void DDataTable(DataTable dt);
    public delegate void DDataSet(DataSet ds);
    public delegate void DStringObject(StringObjectType sot, string str);
    public delegate bool DDoSslAuthentication(USOCKETLib.IUCert Cert);

    public static class Const
    {
        //defines for service CSunticoAsyncHandler
        public const int sidSunticoComm = ((int)USOCKETLib.tagOtherDefine.odUserServiceIDMin + 2112);
        
        //BeginTrans and commit from client side
        public const short idClientBeginTrans = ((short)USOCKETLib.tagOtherDefine.odUserRequestIDMin + 0);
        public const short idClientCommit = idClientBeginTrans + 1;

        //BeginTrans and commit from Suntico server side
        public const short idCloudStartTrans = idClientCommit + 1;
        public const short idCloudEndTrans = idCloudStartTrans + 1;

        //object from client to Suntico server
        public const short idClientSendString = idCloudEndTrans + 1;
        public const short idClientSendObject = idClientSendString + 1;

        //object from Suntico cloud server to client
        public const short idCloudSendString = idClientSendObject + 1;
        public const short idCloudSendObject = idCloudSendString + 1;

        //client confirmation
        public const short idClientConfirmation = idCloudSendObject + 1;
        public const short idClientFakeSlow = idClientConfirmation + 1;

        public const short idCloudSendDataReader = idClientFakeSlow + 1;
        public const short idCloudSendDataTable = idCloudSendDataReader + 1;
        public const short idCloudSendDataSet = idCloudSendDataTable + 1;
        public const short idCloudSendGeneralMessage = idCloudSendDataSet + 1;
    }
}