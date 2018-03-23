<%@ Page Language="C#" AutoEventWireup="true" CodeBehind="uploads.aspx.cs" Inherits="web_two.CUploads" Async="true" %>
<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
<head runat="server">
    <title>Demonstration of uploading employees into master database</title>
</head>
<body>
    <form id="frmUpload" runat="server">
    <div>
    
        Result:
        <asp:TextBox ID="txtResult" runat="server" Width="449px"></asp:TextBox>
        <asp:Button ID="btnDoit" runat="server" OnClick="btnDoit_Click" Text="Button" Width="98px" />
    
    </div>
    </form>
</body>
</html>
