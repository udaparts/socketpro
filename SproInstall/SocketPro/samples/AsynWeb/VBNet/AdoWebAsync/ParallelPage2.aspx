<%@ Page Language="vb" Async="true" AutoEventWireup="true" CodeBehind="ParallelPage2.aspx.vb" Inherits="AdoWebAsync.ParallelPage" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" >
<head runat="server">
	<title>Use multiple asynchronous tasks for processing requests in parrallel</title>
</head>
<body>
	<form id="form1" runat="server">
	<div>
		Command:<br />
		<asp:TextBox ID="txtCmd" runat="server" Width="365px">Delete from Shippers Where ShipperID &gt; 3</asp:TextBox><br />
		<br />
		Query:<br />
		<asp:TextBox ID="txtQuery" runat="server" Width="475px">Select * from Orders Where OrderID Between 10250 and 10252</asp:TextBox>
		<asp:GridView ID="gvQuery" runat="server" Height="202px" Width="745px">
		</asp:GridView>
		<br />
		<asp:Label ID="lblInfo" runat="server" Width="739px"></asp:Label><br />
		<br />
		<asp:Button ID="idDoMultiTask" runat="server" OnClick="idDoMultiTask_Click" Text="Execute Multiple Tasks in Parallel"
			Width="364px" />
		<br />
		<br />
		To see the execution of batched queries asynchrously,
		<asp:HyperLink ID="HyperLink1" runat="server" NavigateUrl="~/Default.aspx">click here</asp:HyperLink>.
		<br />
		To test parallel processing with multiple socket pools with waiting,
		<asp:HyperLink ID="hlParallel" runat="server" NavigateUrl="~/Parallel1.aspx" Width="72px">click here.</asp:HyperLink></div>
	</form>
</body>
</html>