sp_configure 'clr enabled', 1
GO
RECONFIGURE
GO

EXEC sp_changedbowner 'sa' ALTER DATABASE northwind SET TRUSTWORTHY ON
GO

--Load spmssqlpush.dll into SQL server
CREATE ASSEMBLY [spmssqlpush]
AUTHORIZATION [dbo]
FROM 'C:\my_directory\spmssqlpush.dll'
WITH PERMISSION_SET = UNSAFE
GO

CREATE FUNCTION  SetConnectionString(@connectionString nvarchar(2048))
returns int
As
	EXTERNAL NAME spmssqlpush.[spmssqlpush].SetConnectionString
GO

Create FUNCTION GetConnections()
Returns nvarchar(64)
As
	EXTERNAL NAME spmssqlpush.[spmssqlpush].GetConnections
GO

CREATE FUNCTION NotifyDatabaseEvent(
	@eventType int,
	@filter nvarchar(2048),
	@groups nvarchar(2048))
Returns nvarchar(64)
As
	EXTERNAL NAME spmssqlpush.[spmssqlpush].NotifyDatabaseEvent
GO

/*
demo statements
*/

/*
a list of remote SocketPro server connection strings separated by the char '*'
268435457 -- SocketPro tutorial hello_world service id

See the pages at the site http://www.udaparts.com/document/Tutorial/httppush.htm and http://www.udaparts.com/document/Tutorial/helloworld.htm

To see the action of real-time database change notification, start all_servers tutorial application at https://github.com/udaparts/socketpro/tree/master/tutorials
*/

/*
Calling the function returns the number of remote socketpro servers after trying to connect these servers
*/
select dbo.SetConnectionString('host=localhost;port=20901;userid=udatabase_update;pwd=dbupdate_pass_1;serviceid=268435457*server=yyetouch;port=20901;uid=udatabase_update;password=dbupdate_pass_0;zip=1;service-id=268435457') as Res

/*
Calling the function sends a string message (1/rental_id=12@localhost.mysql.sakila.rental) to all connected servers,
and returns a sending status string to all remote SocketPro servers (1 -- success and 0 -- fail).

The last input ('1,2') is a list of chat (notification or topic) group identification numbers.
*/
select dbo.NotifyDatabaseEvent(1, 'rental_id=12@localhost.mysql.sakila.rental', '1,2') as Res

/*
Calling the function returns a connnection status string to all remote SocketPro servers (1 -- connected and 0 -- disconnected)
*/
select dbo.GetConnections() as Res