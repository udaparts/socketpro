# SQL streaming for any databases having ODBC driver on SocketPro communication framework

1. A public repository for implementation of async ODBC database application by use of SocketPro communication framework, which contains nine projects, sodbc, test_sodbc, test_codbc, test_java, test_sharp, test_python, netperf as well as onperf at this time.

2. test_codbc: a test C++ project for demonstration of using async ODBC server at client side. For detail, refer to the demo source code at the file test_codbc/test_codbc.cpp.

3. test_sodbc: a test C++ project for hosting async ODBC server dynamic link library. For detail, refer to the demonstration source code at the file test_sodbc/test_sodbc.cpp.

4. test_java: a test Java Netbeans project for demonstration of using async ODBC server at client side. For detail, refer to the demo source code at the file test_java/src/test_java.java.

5. test_python: a test demonstration of using async ODBC server at client side for Python. For detail, refer to the demo source code at the file test_python/test_python.py.

6. test_sharp: a test .NET c-sharp project for demonstration of using async ODBC server at client side. For detail, refer to the demo source code at the file test_sharp/Program.cs.

7. netperf: a performance study project written from SocketPro async ODBC service.

8. onperf: a performance study project written from MySQL/MariaDB ODBC ADO.NET

9. Key features are:
    - Simple for development
    - Support of both windows and linux platforms
    - Rich features with many unique ones because of its unique design
    - Superior performance and scalability
    - Continuous SQL stream processing supported with inline request/response batching, asynchronous data transferring and parallel computation
    - Cancel supported fully
    - Real-time cache for table update, insert and delete (MS SQL Server 2008 or later only at this writing time)
