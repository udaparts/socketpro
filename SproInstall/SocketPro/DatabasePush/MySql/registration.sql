CREATE FUNCTION Connect
RETURNS INTEGER SONAME 'SpMySqlPush.dll'

CREATE FUNCTION IsConnected
RETURNS INTEGER SONAME 'SpMySqlPush.dll'

CREATE FUNCTION Disconnect
RETURNS INTEGER SONAME 'SpMySqlPush.dll'

CREATE FUNCTION Notify
RETURNS INTEGER SONAME 'SpMySqlPush.dll'

CREATE FUNCTION SendUserMessage
RETURNS INTEGER SONAME 'SpMySqlPush.dll'

Select Connect('server=cye;port=20901;uid=socketpro;pwd=PassOne')

Select IsConnected()

Select SendUserMessage('A message from mySQL database', 'ReceiverUserId')

Select Notify('Hello from mySQL', '1:2:3:4')