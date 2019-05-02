using System;
using System.Collections.Generic;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

namespace ReplicationTest
{
    class CRAdo : CAsyncAdohandler
    {
        public CRAdo()
            : base(RAdoConst.sidRAdo)
        {
        }
    }

    class Program
    {
        static void SetWorkDirectory()
        {
            switch (System.Environment.OSVersion.Platform)
            {
                case PlatformID.Win32NT:
                case PlatformID.Win32S:
                case PlatformID.Win32Windows:
                    CClientSocket.QueueConfigure.WorkDirectory = "c:\\cyetest\\";
                    break;
                case PlatformID.WinCE:
                    break;
                default:
                    CClientSocket.QueueConfigure.WorkDirectory = "/home/yye/cyetest/";
                    break;
            }
        }
        static void Main(string[] args)
        {
            SetWorkDirectory();
            CClientSocket.QueueConfigure.MessageQueuePassword = "MyQPassword";
            ReplicationSetting rs = new ReplicationSetting();
            Dictionary<string, CConnectionContext> ConnQueue = new Dictionary<string, CConnectionContext>();

            CConnectionContext cc = new CConnectionContext("127.0.0.1", 20901, "SocketPro", "PassOne", tagEncryptionMethod.TLSv1);
            ConnQueue["Tolocal"] = cc;

            cc = new CConnectionContext("CYEWIN8", 20901, "SocketPro", "PassOne", tagEncryptionMethod.TLSv1);
            ConnQueue["Tolinux"] = cc;

            CReplication<CRAdo> ado = new CReplication<CRAdo>(rs);

            bool ok = ado.Start(ConnQueue, "rt_root");

            ok = ado.StartJob();
            ado.Send(RAdoConst.idRep0);
            ado.Send(RAdoConst.idRep1);
            ok = ado.EndJob();

            ok = ado.DoReplication();
            ok = false;
        }
    }
}
