<%@ Page Language="C#" AutoEventWireup="true" CodeBehind="rdt.aspx.cs" Inherits="web_two.CRdt" Async="true" %>
<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
<head runat="server">
    <title>Demonstration of Async Page with SocketPro</title>
</head>
<body>
    <form id="frmRdt" runat="server">
    <div>
        Demonstration of Async Web Page<br />
        <br />
        Rental_id:<asp:TextBox ID="txtRentalId" runat="server" Width="140px">1</asp:TextBox>
        <asp:Button ID="btnExecute" runat="server" OnClick="btnExecute_Click" Text="Button" Width="89px" />
        <br />
        Result:</div>
        <asp:Label ID="txtResult" runat="server"></asp:Label>
    </form>
</body>
</html>
