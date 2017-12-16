# SQL streaming for MS SQL server on SocketPro communication framework

1. A public repository for implementation of MS SQL server SQL streaming by use of SocketPro communication framework, which contains four projects, usqlserver, plugindev, test_cache, and test_sharp as well as DBPerf at this time.

2. usqlserver: .NET library for MS SQL server SQLCLR implementation.

3. test_cache: a test C# project for demonstration of using real-time cache for database table events update, delete and insert at client side. For details, refer to the demo source code at the file test_cache/frmCache.cs.

4. plugindev: a .NET project for developing streaming plugin for MS SQL server. For details, refer to the files streamsql.cs and myserver.cs inside the directory usqlserver. The sample project can be changed for other database systems such as Oracle, DB2 and so on by use of their ADO.NET providers.

5. test_sharp: a test .NET c-sharp project for demonstration of using MS SQL server streaming at client side. For detail, refer to the demo source code at the file test_sharp/Program.cs.

8. DBPerf: a directory containing two .NET projects for performance comparision between SocketPro approach and native MS SQL server .NET provider.

9. Key features are:
    - Simple for development
    - Support of both windows and linux platforms
    - Rich features with many unique ones because of its unique design
    - Superior performance and scalability
    - Continuous SQL stream processing supported with inline request/response batching, asynchronous data transferring and parallel computation
    - Real-time cache for table update, insert and delete
    - Cancel supported fully
