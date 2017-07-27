
CREATE OR REPLACE LIBRARY sporaclepush IS 'C:\Windows\System32\sporaclepush.dll';

CREATE OR REPLACE FUNCTION SetConnectionString(connectionString in varchar2)
RETURN BINARY_INTEGER
AS LANGUAGE C
LIBRARY sporaclepush
NAME "SetConnectionString";
/

CREATE OR REPLACE FUNCTION NotifyDatabaseEvent(eventType IN BINARY_INTEGER, sqlFilter IN VARCHAR2, groups IN VARCHAR2)
RETURN VARCHAR2
AS LANGUAGE C
NAME "NotifyDatabaseEvent"
LIBRARY sporaclepush
WITH CONTEXT 
PARAMETERS (
	CONTEXT,
	eventType,
	eventType INDICATOR short,
	sqlFilter STRING,  
	sqlFilter INDICATOR short,
	groups STRING,
	groups INDICATOR short,
	RETURN INDICATOR short,
	RETURN LENGTH short, 
	RETURN STRING);
/

CREATE OR REPLACE FUNCTION GetConnections
RETURN VARCHAR2
AS LANGUAGE C
NAME "GetConnections"
LIBRARY sporaclepush
WITH CONTEXT 
PARAMETERS (
	CONTEXT,
	RETURN INDICATOR short,
	RETURN LENGTH short, 
	RETURN STRING); 
/

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
select SetConnectionString('host=localhost;port=20901;userid=udatabase_update;pwd=dbupdate_pass_1;serviceid=268435457*server=yyetouch;port=20901;uid=udatabase_update;password=dbupdate_pass_0;zip=1;service-id=268435457') as Res from dual
/

/*
Calling the function sends a string message (1/rental_id=12@localhost.mysql.sakila.rental) to all connected servers,
and returns a sending status string to all remote SocketPro servers (1 -- success and 0 -- fail).

The last input ('1,2') is a list of chat (notification or topic) group identification numbers.
*/
select NotifyDatabaseEvent(1, 'rental_id=12@localhost.mysql.sakila.rental', '1,2') as Res from dual
/

/*
Calling the function returns a connnection status string to all remote SocketPro servers (1 -- connected and 0 -- disconnected)
*/
select GetConnections() as Res from dual
/
