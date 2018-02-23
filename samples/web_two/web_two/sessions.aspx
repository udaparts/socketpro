<%@ Page Language="C#" AutoEventWireup="true" CodeBehind="sessions.aspx.cs" Inherits="web_two.Sessions" %>
<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
<head runat="server">
    <title>Test Form -- Get Connections and cached table</title>
</head>
<body>
    <form id="frmSessions" runat="server">
    <div>
        Demonstration of simplest SocketPro requests:<br />
        <br />
        Master Connections:
        <asp:TextBox ID="txtMasterConnections" runat="server" ReadOnly="True" Width="135px"></asp:TextBox>
        <br />
        Slave Connections:<asp:TextBox ID="txtSlaveConnections" runat="server" ReadOnly="True"></asp:TextBox>
        <br />
        <br />
        Cached Tables:
        <asp:ListBox ID="lstTables" runat="server" Height="134px" OnSelectedIndexChanged="lstTables_SelectedIndexChanged" Width="139px" AutoPostBack="True"></asp:ListBox>
    </div>
        <p>
            Table data from web server real-update cache instead of back-end database</p>
        <p>
            <asp:GridView ID="gvTable" runat="server" CellPadding="4" ForeColor="#333333" GridLines="None">
                <AlternatingRowStyle BackColor="White" />
                <EditRowStyle BackColor="#2461BF" />
                <FooterStyle BackColor="#507CD1" Font-Bold="True" ForeColor="White" />
                <HeaderStyle BackColor="#507CD1" Font-Bold="True" ForeColor="White" />
                <PagerStyle BackColor="#2461BF" ForeColor="White" HorizontalAlign="Center" />
                <RowStyle BackColor="#EFF3FB" />
                <SelectedRowStyle BackColor="#D1DDF1" Font-Bold="True" ForeColor="#333333" />
                <SortedAscendingCellStyle BackColor="#F5F7FB" />
                <SortedAscendingHeaderStyle BackColor="#6D95E1" />
                <SortedDescendingCellStyle BackColor="#E9EBEF" />
                <SortedDescendingHeaderStyle BackColor="#4870BE" />
            </asp:GridView>
        </p>
    </form>
</body>
</html>
