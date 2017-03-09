# Async persistent queue applications on SocketPro communication framework

1. A public repository for implementation of async persistent queue applications by use of SocketPro communication framework, which contains six projects, uasyncqueue, test_squeue, test_cqueue, test_java, test_sharp and test_python at this time.

2. uasyncqueue: a dynamic link library project of async queue server implementation for both windows and linux platforms. Its source code is actually located at the directory ../../../include/queue/server_impl. The produced dynamic library can be loaded at server side by a SocketPro adapter as shown at the below demonstration project test_squeue.

3. test_cqueue: a test C++ project for demonstration of using async queue server at client side. For detail, refer to the demo source code at the file test_cqueue/test_cqueue.cpp.

4. test_squeue: a test C++ project for hosting async queue server dynamic link library. For detail, refer to the demonstration source code at the file test_squeue/test_squeue.cpp. Other demo server projects could be found at tutorials/(cplusplus|csharp|vbnet|java)/all_servers and tutorials/(cplusplus|csharp|vbnet|java)/server_queue.

5. test_java: a test Java Netbeans project for demonstration of using async queue server at client side. For detail, refer to the demo source code at the file test_java/src/test_java.java.

6. test_python: a test demonstration of using async queue server at client side for Python. For detail, refer to the demo source code at the file test_python/test_python.py.

7. test_sharp: a test .NET c-sharp project for demonstration of using async queue server at client side. For detail, refer to the demo source code at the file test_sharp/Program.cs.

8. Key features are:
    - Simple for development
    - Support of both windows and linux platforms
    - Rich features with many unique ones because of its unique design
    - Superior performance and scalability
    - Continous message writing supported with inline messaging batching, asynchronous data transferring and parallel computation
    - Cancel supported fully
    - Transactional message writing and reading supported fully
    - Async persistent queue service is totally free to you
