CREATE OR REPLACE LIBRARY SpOraclePush
As
'C:\oraexp\app\oracle\product\10.2.0\server\BIN\SpOraclePush.dll';

CREATE OR REPLACE FUNCTION IsConnected
RETURN BINARY_INTEGER 
AS LANGUAGE C
LIBRARY SpOraclePush
NAME "IsConnected";

CREATE OR REPLACE FUNCTION Disconnect
RETURN BINARY_INTEGER 
AS LANGUAGE C
LIBRARY SpOraclePush
NAME "Disconnect";

CREATE OR REPLACE FUNCTION ToServer(
Conn VARCHAR2)
RETURN BINARY_INTEGER 
AS LANGUAGE C
LIBRARY SpOraclePush
NAME "Connect";

CREATE OR REPLACE FUNCTION Notify(Message VARCHAR2, Groups VARCHAR2)
RETURN BINARY_INTEGER 
AS LANGUAGE C
LIBRARY SpOraclePush
NAME "Notify";

CREATE OR REPLACE FUNCTION SendUserMessage(Message VARCHAR2, UserId VARCHAR2)
RETURN BINARY_INTEGER 
AS LANGUAGE C
LIBRARY SpOraclePush
NAME "SendUserMessage";

Select ToServer('host=idtye;port=20901;uid=socketpro;pwd=PassOne'), Dummy from Dual;
Select IsConnected(), Dummy from Dual;
Select Disconnect(), Dummy from Dual;
Select Notify('A public message from Oracle database', '{1:2}'), Dummy from Dual;
Select SendUserMessage('A private message from Oracle database', 'SocketPro'), Dummy from Dual;
Select Disconnect(), Dummy from Dual;
Select IsConnected(), Dummy from Dual;