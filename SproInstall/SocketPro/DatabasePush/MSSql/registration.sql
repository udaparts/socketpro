sp_configure 'clr enabled', 1
GO
RECONFIGURE
GO

ALTER DATABASE Northwind
SET TRUSTWORTHY ON
GO

--Load usocketnet.dll into SQL server
CREATE ASSEMBLY [usocketnet]
AUTHORIZATION [dbo]
FROM 'D:\cyetest\SpPush\bin\Release\usocketnet.dll'
WITH PERMISSION_SET = UNSAFE
GO

--Load SpMSSqlPush.dll into SQL server
CREATE ASSEMBLY [SpMSSqlPush]
AUTHORIZATION [dbo]
FROM 'D:\cyetest\SpPush\bin\Release\SpMSSqlPush.dll'
WITH PERMISSION_SET = UNSAFE
GO

CREATE FUNCTION Connect(@ConnectionString nvarchar(2048))
returns int
As
	EXTERNAL NAME SpMSSqlPush.[SpMSSqlPush.Messenger].Connect
GO

Create FUNCTION Disconnect()
Returns int
As
	EXTERNAL NAME SpMSSqlPush.[SpMSSqlPush.Messenger].Disconnect
GO

CREATE FUNCTION Notify(
	@Message nvarchar(2048),
	@Groups nvarchar(4000))
returns int
As
	EXTERNAL NAME SpMSSqlPush.[SpMSSqlPush.Messenger].Notify
GO

CREATE FUNCTION SendUserMessage(
	@Message nvarchar(2048),
	@UserId nvarchar(256))
returns int
As
	EXTERNAL NAME SpMSSqlPush.[SpMSSqlPush.Messenger].SendUserMessage
GO

Create FUNCTION IsConnected()
returns bit
As
	EXTERNAL NAME SpMSSqlPush.[SpMSSqlPush.Messenger].IsConnected
GO

--An example for pushing a message from SQL server onto any clients
CREATE PROCEDURE [dbo].[cspInsertShipper]
	@CompanyName nvarchar(40),
	@Phone nvarchar(24)
As
Begin
	declare @res int
	declare @msg nvarchar(2048)
	insert into Shippers(CompanyName, Phone) values (@CompanyName, @Phone);
	Set @msg = 'Insert:ShipperID=' + Cast(Scope_Identity() as varchar);
	Set @res = [dbo].Notify(@msg, '{1;2;3,5,10}');
End
GO

CREATE PROCEDURE [dbo].[cspNotify]
	@msg nvarchar(2048),
	@groups nvarchar(2048),
	@res int output
As
Begin
	Select @res = [dbo].Notify(@msg, @groups);
End
GO

CREATE TRIGGER NewShipperReminder
ON dbo.Shippers
for INSERT
AS
	declare @res int
	declare @id int
	declare @msg nvarchar(2048)
	set @id = IDENT_CURRENT('Shippers')
	select @msg = 'A new entry of Shipper with id = ' + Cast(@id as varchar); 
	set @res = [dbo].Notify(@msg, '1:2:3:10');

--Build a connection to a remote SocketPro server
declare @res int
set @res = [dbo].Connect('server=cye;port=20901;uid=socketpro;pwd=PassOne');
print @res
set @res = [dbo].IsConnected();
print @res
exec cspNotify 'test message', '{1;2;3,5,10}', @res output
print @res
Set @res = [dbo].SendUserMessage('A message from SQL server', 'IE8');
print @res
GO

cspInsertShipper 'Oracle', '(272) 333-444'
GO