using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Suntico;
using Suntico.Client;

namespace FormTest
{
    delegate void DThread2Form(short method);
    delegate void DThread2Form2(short method, string str);

    public partial class frmTest : Form
    {
        public frmTest()
        {
            InitializeComponent();
        }

        
        DThread2Form2 PostEvent2;
        void Thread2Form2(short method, string str)
        {
            switch (method)
            {
                case Suntico.Const.idClientBeginTrans:
                case Suntico.Const.idClientCommit:
                case Suntico.Const.idClientSendObject:
                case Suntico.Const.idClientSendString:
                    txtInfo.Text = str;
                    break;
                default:
                    break;
            }
        }
        DThread2Form PostEvent;
        void Thread2Form(short method)
        {
            switch (method)
            {
                case Suntico.Const.idCloudEndTrans:
                    txtInfo.Text = string.Format("CloudEndTrans: Clue = {0}", m_lClue);
                    break;
                case Suntico.Const.idCloudSendDataReader:
                    txtInfo.Text = string.Format("CloudSendDataReader: DataTable with {0} columns", m_dt.Columns.Count);
                    dgvTable.DataSource = m_dt;
                    break;
                case Suntico.Const.idCloudSendDataSet:
                    {
                        int count = m_ds.Tables.Count;
                        if (count > 0)
                        {
                            DataTable dt = m_ds.Tables[count - 1];
                            dgvTable.DataSource = dt;
                        }
                    }
                    break;
                case Suntico.Const.idCloudSendDataTable:
                    txtInfo.Text = string.Format("CloudSendDataTable: DataTable with {0} columns", m_dt.Columns.Count);
                    dgvTable.DataSource = m_dt;
                    break;
                case Suntico.Const.idCloudSendGeneralMessage:
                    txtInfo.Text = string.Format("CloudSendGeneralMessage: message = {0}", m_msg);
                    break;
                case Suntico.Const.idCloudSendObject:
                    txtInfo.Text = string.Format("CloudSendObject: customer object = {0}", m_TestStructure.ToString());
                    break;
                case Suntico.Const.idCloudSendString:
                    txtInfo.Text = string.Format("CloudSendString: str = {0} with type = {1}", m_str, m_sot);
                    break;
                case Suntico.Const.idCloudStartTrans:
                    txtInfo.Text = string.Format("CloudStartTrans: Clue = {0}", m_lClue);
                    break;
                default:
                    break;
            }
        }

        void Thread2FormClosed()
        {
            btnStart.Enabled = false;
        }

        void Thread2FormOpened()
        {
            btnStart.Enabled = true;
        }

        private void frmTest_Load(object sender, EventArgs e)
        {
            PostEvent = Thread2Form;
            PostEvent2 = Thread2Form2;

            CCloudConnectionContext[] clouds = new CCloudConnectionContext[2];

            //set one master server and one or more backup cloud servers
            clouds[0] = new CCloudConnectionContext()
            {
                IpAddress = "localhost",
                Port = 20901,
                UserId = "SocketPro",
                Password = "PassOne",
                Secure = false,
                Zip = false
            };

            clouds[1] = new CCloudConnectionContext()
            {
                IpAddress = "127.0.0.1",
                Port = 20901,
                UserId = "SocketPro",
                Password = "PassOne",
                Secure = false,
                Zip = true
            };

            m_Client = new CClientPoint(clouds, 1000);

            m_Client.ChannelsClosed += client_ChannelsClosed;
            m_Client.ChannelsOpened += m_Client_ChannelsOpened;
            m_Client.DoSslAuthentication += m_Client_DoSslAuthentication;

            m_Client.CloudMessage.OnDataReader += new DDataReader(CloudMessage_OnDataReader);
            m_Client.CloudMessage.OnDataSet += new DDataSet(CloudMessage_OnDataSet);
            m_Client.CloudMessage.OnDataTable += new DDataTable(CloudMessage_OnDataTable);
            m_Client.CloudMessage.OnEndTrans += new DEndTrans(CloudMessage_OnEndTrans);
            m_Client.CloudMessage.OnGeneralMessage += new DGeneralMessage(CloudMessage_OnGeneralMessage);
            m_Client.CloudMessage.OnGenericObject += new DGenericObject(CloudMessage_OnGenericObject);
            m_Client.CloudMessage.OnStartTrans += new DStartTrans(CloudMessage_OnStartTrans);
            m_Client.CloudMessage.OnStringObject += new DStringObject(CloudMessage_OnStringObject);

            m_Client.Start();
        }

        bool m_Client_DoSslAuthentication(USOCKETLib.IUCert Cert)
        {
            int errCode;

            //check certificate subject here
            string str = Cert.Subject.ToLower();

            Cert.VerifyLocation = "udaparts.com";

            //verify certificates chain here
            str = Cert.Verify(out errCode);


            //for real certificate
            //if (errCode != 0)
            //    return false; //bad certificate

            return true;
        }

        void m_Client_ChannelsOpened()
        {
            BeginInvoke((DChannelsOpened)Thread2FormOpened);
        }

        StringObjectType m_sot;
        string m_str;
        void CloudMessage_OnStringObject(StringObjectType sot, string str)
        {
            m_sot = sot;
            m_str = str;
            BeginInvoke(PostEvent, Suntico.Const.idCloudEndTrans);
        }

        long m_lTransIndex = 0;
        long m_lClue;
        void CloudMessage_OnStartTrans(long Clue)
        {
            m_lClue = Clue;
            BeginInvoke(PostEvent, Suntico.Const.idCloudStartTrans);
        }

        CloudClientSharedForUnitTest.TestStructure m_TestStructure; 
        void CloudMessage_OnGenericObject(long Clue, SocketProAdapter.CUQueue Queue)
        {
            switch (Clue)
            {
                case 1:
                    {
                        Queue.Load(out m_TestStructure);
                        BeginInvoke(PostEvent, Suntico.Const.idCloudSendObject);
                    }
                    break;
                default:
                    break;
            }
        }

        string m_msg;
        int m_group;
        int m_serviceId;
        void CloudMessage_OnGeneralMessage(string msg, int Group, int ServiceId)
        {
            m_msg = msg;
            m_group = Group;
            m_serviceId = ServiceId;
        }

        void CloudMessage_OnEndTrans(long Clue)
        {
            m_lClue = Clue;
            BeginInvoke(PostEvent, Suntico.Const.idCloudEndTrans);
        }

        DataTable m_dt;
        void CloudMessage_OnDataTable(DataTable dt)
        {
            m_dt = dt;
            BeginInvoke(PostEvent, Suntico.Const.idCloudSendDataTable);
        }

        DataSet m_ds;
        void CloudMessage_OnDataSet(DataSet ds)
        {
            m_ds = ds;
            BeginInvoke(PostEvent, Suntico.Const.idCloudSendDataSet);
        }

        void CloudMessage_OnDataReader(DataTable dt)
        {
            m_dt = dt;
            BeginInvoke(PostEvent, Suntico.Const.idCloudSendDataReader);
        }

        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            m_Client.Close();
            base.OnFormClosed(e);
        }

        CClientPoint m_Client;

        void client_ChannelsClosed()
        {
            BeginInvoke((DChannelsClosed)Thread2FormClosed);
        }

        private void btnStart_Click(object sender, EventArgs e)
        {
            if (m_Client.ConnectionStatus != ConnectionStatus.Opened)
                return;

            CloudClientSharedForUnitTest.TestStructure ts = new CloudClientSharedForUnitTest.TestStructure()
            {
                n = 10,
                str = "ClientTest"
            };

            DataSet ds = new DataSet("MyDataSet");
            ds.ReadXmlSchema("C:\\chaohu\\suntico_consulting\\SComm\\Customer.xsd");
            ds.ReadXml("C:\\chaohu\\suntico_consulting\\SComm\\Customer.xml");

            DataTable dt = ds.Tables[0];
            ICustomerMessage cm = m_Client.CustomerMessage;

            do
            {
                Suntico.ConnectionStatus cs;
                try
                {
                    cs = cm.BeginTrans(m_lTransIndex,
                        delegate(long res)
                        {
                            string str = "A remote Suntico cloud server just started a transaction with res = " + res.ToString();
                            BeginInvoke(PostEvent2, Suntico.Const.idClientBeginTrans, str);
                        }
                        , 3000
                    );
                }
                catch (Exception ex)
                {
                    txtInfo.Text = ex.Message + "|" + ex.StackTrace;
                    break;
                }
                if (cs == Suntico.ConnectionStatus.Closed)
                {
                    txtInfo.Text = "Remote Suntico cloud servers not available!";
                    break;
                }

                cm.Send("{\"AInt\":12345,\"AStr\":\"test\",\"ABool\":true}",
                    Suntico.StringObjectType.Json,
                    delegate()
                    {
                        string str = "A remote Suntico cloud processed JSON text string";
                        BeginInvoke(PostEvent2, Suntico.Const.idClientSendString, str);
                    }
                );

                cm.Send(1, ts, delegate()
                {
                    string str = "A remote Suntico cloud processed a customer defined object = " + ts.ToString();
                    BeginInvoke(PostEvent2, Suntico.Const.idClientSendObject, str);
                }
                );

                //You can send ADO.NET objects through load balancing now.
                cm.Send(ds, delegate()
                    {
                        string str = "A remote Suntico cloud processed a dataset object = ";
                        BeginInvoke(PostEvent2, Suntico.Const.idClientSendObject, str);
                    }
                );

                cs = cm.Commit(m_lTransIndex,
                    delegate(long res)
                    {
                        string str = "A remote Suntico cloud server just committed a transaction with res = " + res.ToString();
                        BeginInvoke(PostEvent2, Suntico.Const.idClientCommit, str);
                    }
                );

                if (cs == Suntico.ConnectionStatus.Closed)
                {
                    txtInfo.Text = "Remote Suntico cloud servers not available!";
                    break;
                }

                int timeout = 300;
                if (!cm.Wait(timeout))
                {
                    txtInfo.Text = string.Format("Remote Suntico cloud server failed in processing requests batch in {0}!", timeout); ;
                    break;
                }

            } while (false);

            ++m_lTransIndex;
        }
    }
}
