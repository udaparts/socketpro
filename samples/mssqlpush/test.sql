/*
CREATE TABLE reptable(myint int primary key, mystr nvarchar(max) NULL);
GO

CREATE ASSEMBLY [SproAdapter35] from 'c:\SproAdapter35.dll' with Permission_Set = UNSAFE;
GO

CREATE ASSEMBLY [dbrep] from 'c:\dbrep.dll' with Permission_Set = UNSAFE;
GO

Create Trigger dmltrigger on reptable FOR INSERT, UPDATE, DELETE as EXTERNAL NAME dbrep.sqlpush.dmltrigger;
GO

Create function sendQuery (@sql nvarchar(max), @name nvarchar(max)) returns bit as EXTERNAL NAME dbrep.sqlpush.sendQuery;
GO

Create function getConnections () returns int as EXTERNAL NAME dbrep.sqlpush.getConnections;
GO

Create function getHosts () returns int as EXTERNAL NAME dbrep.sqlpush.getHosts;
GO

Create function getQueues () returns int as EXTERNAL NAME dbrep.sqlpush.getQueues;
GO
*/

/*
select dbo.getConnections()
go

select dbo.getHosts();
go

select dbo.getQueues();
go

delete from reptable where myint < 10;
go

insert into reptable values(1, 'MyTestAgain');
go

insert into reptable values(2, 'MyTestMEAgain');
go

update reptable set mystr = 'A new test data string' where myint = 2;
go
*/
-- select dbo.sendquery('select * from dbo.DimProduct', 'mytestquery');
