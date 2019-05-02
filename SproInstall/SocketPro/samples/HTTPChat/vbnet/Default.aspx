<%@ Page Language="VB" AutoEventWireup="true" CodeFile="Default.aspx.vb" Inherits="_Default" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" >
<head runat="server">
    <title>Untitled Page</title>

<script type="text/javascript">

var onMyMessage = function(req, msg){
    document.getElementById('txtMsgIn').value = req.method + " result: " + UJSON.stringify(msg);
    if(req.method == 'exit')
        myInit();
};

function btnSpeak_onclick() {
    var groups = [1, 2, 8]; //chat groups (1 + 2 + 8)
    UChat.speak(document.getElementById('txtMsg').value, //message
    groups
    );
}

function btnSendUserMessage_onclick() {
    UChat.sendUserMessage(document.getElementById('txtMsg').value, //message
    document.getElementById('txtUser').value //user id
    );
}

function btnExit_onclick() {
    UChat.exit(); 
}

function myInit() {
    //make sure controls have a correct initial state
    var ctrl = document.getElementById("btnSendUserMessage");
    ctrl.disabled = 'disabled';
    ctrl = document.getElementById("btnSpeak");
    ctrl.disabled = 'disabled';
    ctrl = document.getElementById("btnEnter");
    ctrl.disabled = '';
}
</script>
</head>
<body>
    <form id="form1" runat="server">
    <div>
        <asp:Button ID="btnEnter" runat="server" OnClick="btnEnter_Click" Text="Enter" Width="147px" Height="33px" />
        <asp:TextBox ID="txtUserId" runat="server" Height="23px" Width="114px">WebUser</asp:TextBox>
        <input id="btnExit" style="width: 132px; height: 30px" type="button" value="Exit" onclick="return btnExit_onclick()" /><br />
        <input id="btnSpeak" style="width: 145px; height: 32px" type="button" value="Speak" onclick="return btnSpeak_onclick()" /><asp:TextBox ID="txtMsg" runat="server" Width="653px" Height="22px">Test Message From Asp.NET Push</asp:TextBox><br />
        <input id="btnSendUserMessage" style="width: 143px; height: 30px" type="button" value="Send User Message" onclick="return btnSendUserMessage_onclick()" />
        <asp:TextBox ID="txtUser" runat="server" Width="96px" Height="21px">SocketPro</asp:TextBox><br />
        <asp:TextBox ID="txtMsgIn" runat="server" Height="100px" Width="803px" TextMode="MultiLine"></asp:TextBox></div>
    </form>
</body>
</html>
