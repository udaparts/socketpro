<%@ Page Async="true" Language="C#" AutoEventWireup="true" CodeBehind="TestIp.aspx.cs" Inherits="WebForm.TestIp" %>

<!DOCTYPE html>

<html xmlns="http://www.w3.org/1999/xhtml">
<head runat="server">
    <title>Prescision Design Solutions iCOS Application Web Client Demonstration</title>
</head>
<body>
    <p>
        Prescision Design Solutions iCOS Application Web Client Demonstration</p>
    <form id="form1" runat="server">
    <div>
    
        &nbsp;</div>
        <asp:TextBox ID="txtCountries" runat="server" Width="391px"></asp:TextBox>
        <br />
        <asp:Button ID="btnLookup" runat="server" OnClick="btnLookup_Click" Text="Ip lookup" Width="185px" />
    </form>
</body>
</html>
