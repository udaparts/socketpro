using System;
using System.Data;
using System.Configuration;
using System.Collections;
using System.Web;
using System.Web.Security;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Web.UI.WebControls.WebParts;
using System.Web.UI.HtmlControls;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

public partial class _Default : System.Web.UI.Page
{
    protected void Page_Load(object sender, EventArgs e)
    {
        if (AspNetPush.Comet.UrlToComet.Length == 0)
        {
		//make sure you have already started the HTTP COMET server at
		//C:\Program Files\UDAParts\SocketPro\tutorial\CSharp\Chat\Server
            AspNetPush.Comet.UrlToComet = "http://localhost:20901/UCHAT";
        }
    }
    protected void btnEnter_Click(object sender, EventArgs e)
    {
        int[] Groups = { 1, 2, 8 };
        string strEnter = AspNetPush.Comet.Enter(this, txtUserId.Text, Groups, "onMyMessage");
        btnEnter.Enabled = false;
    }
}

