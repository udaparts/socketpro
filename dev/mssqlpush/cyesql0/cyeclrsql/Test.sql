-- Examples for queries that exercise different SQL objects implemented by this assembly

-----------------------------------------------------------------------------------------
-- Stored procedure
-----------------------------------------------------------------------------------------
-- exec StoredProcedureName


-----------------------------------------------------------------------------------------
-- User defined function
-----------------------------------------------------------------------------------------
-- select dbo.FunctionName()


-----------------------------------------------------------------------------------------
-- User defined type
-----------------------------------------------------------------------------------------
-- CREATE TABLE test_table (col1 UserType)
-- go
--
-- INSERT INTO test_table VALUES (convert(uri, 'Instantiation String 1'))
-- INSERT INTO test_table VALUES (convert(uri, 'Instantiation String 2'))
-- INSERT INTO test_table VALUES (convert(uri, 'Instantiation String 3'))
--
-- select col1::method1() from test_table



-----------------------------------------------------------------------------------------
-- User defined type
-----------------------------------------------------------------------------------------
-- select dbo.AggregateName(Column1) from Table1


-- CREATE TABLE cyetable0(myint int primary key, mystr nvarchar(2048) NULL);

-- Create Trigger cyetrigger0 on cyetable0 FOR INSERT, UPDATE, DELETE as EXTERNAL NAME cyesql0.sqlpush.cyetrigger0;
-- Create function sendquery (@sql nvarchar(max), @tableName nvarchar(max)) returns bit as EXTERNAL NAME cyesql0.sqlpush.sendquery;
-- CREATE TRIGGER dblogon ON ALL SERVER for LOGON as EXTERNAL NAME cyesql0.sqlpush.logontrigger;


delete from cyetable0 where myint < 10

insert into cyetable0 values(1, 'MyTestAgain');

insert into cyetable0 values(2, 'MyTestMEAgain');

update cyetable0 set mystr = 'A new test data string' where myint = 2

select dbo.sendquery('select * from dbo.DimProduct', 'mytestquery')