<%@ Page Language="C#" AutoEventWireup="true" CodeBehind="mma.aspx.cs" Inherits="web_two.CMma" %>
<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
<head runat="server"> 
    <title>Demonstration of querying max, min and avg amounts from payment</title>
</head>
<body>
    <form id="frmMma" runat="server">
    <div>
        Query max, min and avg amounts from table sakila.payment:<br />
        <br />
        Payment_id filter:
        <asp:TextBox ID="txtFilter" runat="server" Width="383px"></asp:TextBox>
        <br />
        Ouput values:<asp:TextBox ID="txtResults" runat="server" Width="406px"></asp:TextBox>
        <asp:Button ID="txtExecute" runat="server" OnClick="txtExecute_Click" Text="Button" Width="104px" />
    </div>
    </form>
</body>
</html>
