# Async mysql and mariadb middle-tier applications on SocketPro communication framework

1. A public repository for implementation of async mysql (mariadb) client/server application by use of SocketPro communication framework, which contains six projects, smysql, test_smysql, test_cmysql, test_java, test_sharp and test_python at this time.

2. smysql: a dynamic link library project of async mysql (mariadb) server implementation for both windows and linux platforms. Its source code is actually located at the directory socketpro/include/mysql/server_impl. The produced dynamic library can be loaded at server side by a SocketPro adapter as shown at the below demonstration project test_smysql.

3. test_cmysql: a test C++ project for demonstration of using async mysql (mariadb) server at client side. For detail, refer to the demo source code at the file test_cmysql/test_cmysql.cpp.

4. test_smysql: a test C++ project for hosting async mysql (mariadb) server dynamic link library. For detail, refer to the demonstration source code at the file test_smysql/test_smysql.cpp.

5. test_java: a test Java Netbeans project for demonstration of using async mysql (mariadb) server at client side. For detail, refer to the demo source code at the file test_java/src/test_java.java.

6. test_python: a test demonstration of using async mysql (mariadb) server at client side for Python. For detail, refer to the demo source code at the file test_python/test_python.py.

7. test_sharp: a test .NET c-sharp project for demonstration of using async mysql (mariadb) server at client side. For detail, refer to the demo source code at the file test_sharp/Program.cs.

8. Key features are:
    - Simple for development
    - Support of both windows and linux platforms
    - Rich features with many unique ones because of its unique design
    - Superior performance and scalability
    - Continuous SQL stream processing supported with continuous inline request/response batching, asynchronous data transferring and parallel computation
    - Cancel supported fully
    - Async mysql/mariadb service is totally free to you
