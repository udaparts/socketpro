# SQL streaming for SQLite on SocketPro communication framework

1. A public repository for implementation of async sqlite client/server application by use of SocketPro communication framework, which contains seven projects, ssqlite, test_ssqlite, test_csqlite, test_java, test_sharp, test_cache and test_python as well as DBPerf at this time.

2. test_csqlite: a test C++ project for demonstration of using async sqlite server at client side. For detail, refer to the demo source code at the file test_csqlite/test_csqlite.cpp.

3. test_ssqlite: a test C++ project for hosting async sqlite server dynamic link library. For detail, refer to the demonstration source code at the file test_ssqlite/test_ssqlite.cpp. Other demo server projects could be found at tutorials/(cplusplus|csharp|vbnet|java)/all_servers.

4. test_java: a test Java Netbeans project for demonstration of using async sqlite server at client side. For detail, refer to the demo source code at the file test_java/src/test_java.java.

5. test_python: a test demonstration of using async sqlite server at client side for Python. For detail, refer to the demo source code at the file test_python/test_python.py.

6. test_sharp: a test .NET c-sharp project for demonstration of using async sqlite server at client side. For detail, refer to the demo source code at the file test_sharp/Program.cs.

7. test_cache: a test .NET c-sharp project for demonstration of async SQLite real-time update cache. For detail, refer to the demo source code at the file test_cache/frmCache.cs.

8. DBPerf: a directory containing two projects for performance comparision between SocketPro approach and mysql/mariadb .NET provider.

9. sqlite_streaming.js a test demonstration for node.js

10. Key features are:
    - Simple for development
    - Support of both windows and linux platforms
    - Rich features with many unique ones because of its unique design
    - Superior performance and scalability
    - Continuous SQL stream processing supported with inline request/response batching, asynchronous data transferring and parallel computation
    - Cancel supported fully
    - Auto server notification upon record updating (add, update and delete)
    - Async sqlite service is totally free to you
