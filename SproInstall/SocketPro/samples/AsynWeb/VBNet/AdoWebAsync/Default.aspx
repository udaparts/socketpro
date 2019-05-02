<%@ Page Language="vb" Async="true" AutoEventWireup="true" CodeBehind="Default.aspx.vb" Inherits="AdoWebAsync._Default" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" >
<head runat="server">
	<title>Asynchronously execute a set of requests in batch</title>
</head>
<body>
	<form id="form1" runat="server">
	<div>
		&nbsp; <strong><span style="font-size: 16pt">Testing Asynchronous Web Page with SocketPro<br />
		</span></strong>
		<br />
		&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;
		&nbsp; Your new input for a shipper entry:<br />
		Company Name:
		<asp:TextBox ID="txtCompany" runat="server" Width="222px">A company Name</asp:TextBox><br />
		&nbsp; Phone Number:
		<asp:TextBox ID="txtPhoneNumber" runat="server" Width="105px">(777) 888-9999</asp:TextBox><br />
		<br />
		List of Shippers:<br />
		<asp:GridView ID="gvRowset" runat="server" Width="240px">
		</asp:GridView>
		<br />
		<asp:Button ID="btnExecute" runat="server" Height="28px" OnClick="btnExecute_Click"
			Text="Execute" Width="99px" /><br />
		<br />
		To test parallel processing with multiple socket pools with waiting,
		<asp:HyperLink ID="hlParallel" runat="server" NavigateUrl="~/Parallel1.aspx" Width="72px">click here.</asp:HyperLink><br />
		To see multiple asynchronous processing without waiting,
		<asp:HyperLink ID="HyperLink2" runat="server" NavigateUrl="~/ParallelPage2.aspx">click here</asp:HyperLink>.

	</div>
	</form>
</body>
</html>