<%@ Page Language="vb" AutoEventWireup="true" CodeBehind="Parallel1.aspx.vb" Inherits="AdoWebAsync.Parallel" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" >
<head runat="server">
	<title>Process multiple requests in parallel with barriers</title>
</head>
<body>
	<form id="form1" runat="server">
	<div title="Batch and Parallel Processing with SocketPro">
		<strong><span style="font-size: 16pt">This is a synchronous page, but supports parallel
			processing .....</span></strong><br />
		<br />
		<strong>
		Query one:</strong>
		<asp:TextBox ID="txtSQL1" runat="server" Width="574px">Select OrderID, CustomerID, OrderDate from Orders Where OrderID &lt; 10255</asp:TextBox><br />
		<asp:GridView ID="gvQueryOne" runat="server" Width="657px">
		</asp:GridView>

	</div>
		<br />
		<strong>
		Query two:</strong>
		<asp:TextBox ID="txtSQL2" runat="server" Width="570px">Select ProductID, ProductName, UnitsOnOrder from Products Where ProductID between 2 and 10</asp:TextBox>
		<asp:GridView ID="gvQueryTwo" runat="server" Width="653px">
		</asp:GridView>
		<br />
		<asp:Button ID="btnParallel" runat="server" OnClick="btnParallel_Click" Text="Execute two SQLs in Parallel"
			Width="653px" /><br />
		<br />
		To see the execution of batched queries asynchronously,
		<asp:HyperLink ID="HyperLink1" runat="server" NavigateUrl="~/Default.aspx">click here</asp:HyperLink>.
		<br />
		To see multiple asynchronous processing without waiting,
		<asp:HyperLink ID="HyperLink2" runat="server" NavigateUrl="~/ParallelPage2.aspx">click here</asp:HyperLink>.
	</form>
</body>
</html>