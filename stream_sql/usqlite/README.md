# SQL streaming for SQLite on SocketPro communication framework

1. A public repository for implementation of async sqlite client/server application by use of SocketPro communication framework, which contains seven projects, ssqlite, test_ssqlite, test_csqlite, test_java, test_sharp, test_cache and test_python as well as DBPerf at this time.

2. ssqlite: a dynamic link library project of async sqlite server implementation for both windows and linux platforms. Its source code is actually located at the directory ../../../include/sqlite/server_impl. The produced dynamic library can be loaded at server side by a SocketPro adapter as shown at the below demonstration project test_ssqlite.

3. test_csqlite: a test C++ project for demonstration of using async sqlite server at client side. For detail, refer to the demo source code at the file test_csqlite/test_csqlite.cpp.

4. test_ssqlite: a test C++ project for hosting async sqlite server dynamic link library. For detail, refer to the demonstration source code at the file test_ssqlite/test_ssqlite.cpp. Other demo server projects could be found at tutorials/(cplusplus|csharp|vbnet|java)/all_servers.

5. test_java: a test Java Netbeans project for demonstration of using async sqlite server at client side. For detail, refer to the demo source code at the file test_java/src/test_java.java.

6. test_python: a test demonstration of using async sqlite server at client side for Python. For detail, refer to the demo source code at the file test_python/test_python.py.

7. test_sharp: a test .NET c-sharp project for demonstration of using async sqlite server at client side. For detail, refer to the demo source code at the file test_sharp/Program.cs.

8. test_cache: a test .NET c-sharp project for demonstration of async SQLite real-time update cache. For detail, refer to the demo source code at the file test_cache/frmCache.cs.

9. DBPerf: a directory containing two projects for performance comparision between SocketPro approach and mysql/mariadb .NET provider.

10. Key features are:
    - Simple for development
    - Support of both windows and linux platforms
    - Rich features with many unique ones because of its unique design
    - Superior performance and scalability
    - Continuous SQL stream processing supported with inline request/response batching, asynchronous data transferring and parallel computation
    - Cancel supported fully
    - Auto server notification upon record updating (add, update and delete)
    - Async sqlite service is totally free to you
