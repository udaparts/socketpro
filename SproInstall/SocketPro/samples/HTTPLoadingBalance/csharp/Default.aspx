<%@ Page Language="C#" AutoEventWireup="true"  CodeFile="Default.aspx.cs" Inherits="_Default" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" >
<head runat="server">
    <title>HTTP Loading Balance by SocketPro</title>
</head>
<body>
    <form id="form1" runat="server">
    <div>
        Process requests in parallel with loading balance and disaster recovery &nbsp; &nbsp;
        <asp:Button ID="btnExecute" runat="server" OnClick="btnExecute_Click" Text="Execute SQLs in parallel with loading balance and diaster recovery"
            Width="422px" /><br />
        <table style="width: 887px; height: 76px">
            <tr>
                <td style="width: 327px">
                    Query 1:</td>
                <td style="width: 238px">
                    Query Two:</td>
                <td style="width: 284px">
                    Query Three:</td>
            </tr>
            <tr>
                <td style="width: 327px; height: 32px">
                    <asp:TextBox ID="txtSQL1" runat="server" Height="80px" TextMode="MultiLine" Width="267px">Select * from Shippers</asp:TextBox></td>
                <td style="width: 238px; height: 32px">
                    <asp:TextBox ID="txtSQL2" runat="server" Height="84px" TextMode="MultiLine" Width="378px">Select ProductID, ProductName, UnitPrice from Products Where ProductID between 5 and 10</asp:TextBox></td>
                <td style="width: 284px; height: 32px">
                    <asp:TextBox ID="txtSQL3" runat="server" Height="81px" TextMode="MultiLine" Width="273px">Select CustomerID, CompanyName, ContactName from Customers Where Country='USA'</asp:TextBox></td>
            </tr>
            <tr>
                <td style="width: 327px; height: 128px">
                    <asp:GridView ID="gvSQL1" runat="server" Height="191px" Width="274px">
                    </asp:GridView>
                </td>
                <td style="width: 238px; height: 128px">
                    <asp:GridView ID="gvSQL2" runat="server" Height="195px" Width="381px">
                    </asp:GridView>
                </td>
                <td style="width: 284px; height: 128px">
                    <asp:GridView ID="gvSQL3" runat="server" Height="187px" Width="277px">
                    </asp:GridView>
                </td>
            </tr>
        </table>
    
    </div>
    </form>
</body>
</html>
