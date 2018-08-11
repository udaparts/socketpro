# SQL streaming for MySQL 8.0.11 or later on SocketPro communication framework

1. A public repository for implementation of async MySQL client/server application by use of SocketPro communication framework, which contains seven projects, smysql, test_cache, test_cmysql, test_java, test_sharp and test_python as well as DBPerf at this time.

2. smysql: a dynamic link library project of async MySQL server implementation for both windows and linux platforms.

3. test_cache: a test C# project for demonstration of using real-time cache for database table events update, delete and insert at client side. For details, refer to the demo source code at the file test_cache/frmCache.cs.

4. test_cmysql: a test C++ project for demonstration of using async MySQL server at client side. For detail, refer to the demo source code at the file test_cmysql/test_cmysql.cpp.

5. test_java: a test Java Netbeans project for demonstration of using async MySQL server at client side. For detail, refer to the demo source code at the file test_java/src/test_java.java.

6. test_python: a test demonstration of using async MySQL server at client side for Python. For detail, refer to the demo source code at the file test_python/test_python.py.

7. test_sharp: a test .NET c-sharp project for demonstration of using async MySQL server at client side. For detail, refer to the demo source code at the file test_sharp/Program.cs.

8. DBPerf: a directory containing three projects for performance comparision between SocketPro approach and MySQL .NET provider.

9. Key features are:
    - Simple for development
    - Support of both windows and linux platforms
    - Rich features with many unique ones because of its unique design
    - Superior performance and scalability
    - Continuous SQL stream processing supported with inline request/response batching, asynchronous data transferring and parallel computation
    - Real-time cache for table update, insert and delete
    - Cancel supported fully
    - Async MySQL service is totally free to you

10. This Async MySQL SQL-streaming service supports MySQL 8.0.11 or later.
