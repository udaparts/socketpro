using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Windows.Browser;
using SocketProAdapter.ClientSide;

namespace SilverPush
{
    public partial class MainPage : UserControl
    {
        long m_lIndex = 0;
        public MainPage()
        {
            InitializeComponent();
            UHTTP.UChat.OnChatRequest += new DOnChatRequest(UChat_OnChatRequest);
            UHTTP.UChat.OnMessage += new DOnChatNotification(UChat_OnMessage);
        }

        void UChat_OnMessage(CRequest Request, CHttpPush HttpPush)
        {
            switch (HttpPush.Type)
            {
                case MessageType.Normal:
                    {
                        string str = "";
                        foreach (CChatMessage cm in HttpPush.Messages)
                        {
                            string strGroups = "[";
                            if (cm.Groups != null)
                            {
                                for (int n = 0; n < cm.Groups.Length; n++)
                                {
                                    if (n > 0) strGroups += ", ";
                                    strGroups += cm.Groups[n].ToString();
                                }
                                strGroups += "]";
                            }
                            if (str.Length > 0)
                                str += "<-->";
                            if (cm.MethodName == "enter")
                            {
                                str += string.Format("Sender = {0}, Message = join groups = {1}", cm.Sender, strGroups);
                            }
                            else if (cm.MethodName == "exit")
                            {
                                str += string.Format("Sender = {0}, Message = {1}, groups = {2}", cm.Sender, "exit", strGroups);
                            }
                            else
                                str += string.Format("Sender = {0}, Message = {1}, groups = {2}", cm.Sender, cm.Message, strGroups);
                        }
                        txtMsg.Text = str;
                    }
                    break;
                case MessageType.ServerShuttingdownGracefully:
                    txtMsg.Text = "SocketPro HTTP push server shut down gracefully";
                    break;
                case MessageType.Timeout:
                    txtMsg.Text = "Time out";
                    break;
                default:
                    txtMsg.Text = "Unknown message";
                    break;
            }
        }

        void UChat_OnChatRequest(CRequest Request, object Result)
        {
            switch (Request.Name)
            {
                case "enter":
                    if (UHTTP.UChat.Chatting)
                    {
                        btnEnter.IsEnabled = false;
                        btnSpeak.IsEnabled = true;
                    }
                    break;
                case "exit":
                    btnEnter.IsEnabled = true;
                    btnSpeak.IsEnabled = false;
                    break;
                default:
                    break;
            }
            txtMsg.Text = string.Format("Method = {0}, Result = {1}", Request.Name, Result.ToString());
        }

        private void btnEnter_Click(object sender, RoutedEventArgs e)
        {
            uint[] Groups = { 1, 2 };
            UHTTP.UChat.Enter(txtMyUserId.Text, Groups);
        }

        private void btnExit_Click(object sender, RoutedEventArgs e)
        {
            UHTTP.UChat.Exit();
        }

        private void btnSpeak_Click(object sender, RoutedEventArgs e)
        {
            uint[] Groups = { 1, 2, 8 };
            UHTTP.UChat.Speak(txtMsgOut.Text, Groups);
        }

        private void btnDoMyRequest_Click(object sender, RoutedEventArgs e)
        {
            Dictionary<string, object> map = new Dictionary<string, object>();

            map.Add("Map0", 123.45);
            map.Add("Map1", null);
            map.Add("Map2", "A sample map string");

            object[] AComplexStructure = new object[7];
            AComplexStructure[0] = map;
            AComplexStructure[1] = "MyString";
            AComplexStructure[2] = DateTime.Now.AddDays(-2);
            AComplexStructure[4] = true;
            AComplexStructure[6] = 34567;

            object[] child = new object[2];
            child[0] = "myDate";
            child[1] = DateTime.Now.AddDays(-5);
            AComplexStructure[3] = child;

            CRequest reqComplex = UHTTP.CreateHttpRequest("MyCall", "/MyChannel");
            reqComplex.Add("Param0", m_lIndex);
            reqComplex.Add("Param1", 23.45);
            reqComplex.Add("MyParam", "A demo string");
            reqComplex.Add("ABoolean", true);
            reqComplex.Add("ADateTime", DateTime.Now);
            reqComplex.Add("AnArray", AComplexStructure);
            reqComplex.Add("CyeMap", map);

            m_lIndex = reqComplex.Invoke(delegate(CRequest myReq, string strResult)
            {
                HtmlPage.Window.Alert(strResult);
            });
        }
    }
}
