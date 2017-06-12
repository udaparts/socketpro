# Async mysql and mariadb client/server applications on SocketPro communication framework

1. A public repository for implementation of async mysql (mariadb) client/server application by use of SocketPro communication framework, which contains six projects, smysql, test_cmysql, test_java, test_sharp and test_python as well as DBPerf at this time.

2. smysql: a dynamic link library project of async mysql (mariadb) server implementation for both windows and linux platforms.

3. test_cmysql: a test C++ project for demonstration of using async mysql (mariadb) server at client side. For detail, refer to the demo source code at the file test_cmysql/test_cmysql.cpp.

4. test_java: a test Java Netbeans project for demonstration of using async mysql (mariadb) server at client side. For detail, refer to the demo source code at the file test_java/src/test_java.java.

5. test_python: a test demonstration of using async mysql (mariadb) server at client side for Python. For detail, refer to the demo source code at the file test_python/test_python.py.

6. test_sharp: a test .NET c-sharp project for demonstration of using async mysql (mariadb) server at client side. For detail, refer to the demo source code at the file test_sharp/Program.cs.

7. DBPerf: a directory containing three projects for performance comparision between SocketPro approach and mysql/mariadb .NET provider.

8. Key features are:
    - Simple for development
    - Support of both windows and linux platforms
    - Rich features with many unique ones because of its unique design
    - Superior performance and scalability
    - Continuous SQL stream processing supported with inline request/response batching, asynchronous data transferring and parallel computation
    - Cancel supported fully
    - Async mysql/mariadb service is totally free to you
