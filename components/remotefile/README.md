# High performance file streaming on SocketPro communication framework

1. A public repository for implementation of async file streaming by use of SocketPro communication framework, which contains six projects, ustreamfile, test_client, test_java, test_sharp, test_streamfile and test_python at this time.

2. ustreamfile: a dynamic link library project of async file streaming plugin implementation for both windows and linux platforms. Its source code is actually located at the directory ../../../include/file/server_impl. The produced dynamic library can be loaded at server side by a SocketPro adapter as shown at the below demonstration project test_streamfile.

3. test_client: a test C++ project for demonstration of using async file streaming server at client side. For detail, refer to the demo source code at the file test_client/test_client.cpp.

4. test_streamfile: a test C++ project for hosting async file streaming dynamic link library. For detail, refer to the demonstration source code at the file test_streamfile/test_streamfile.cpp.

5. test_java: a test Java Netbeans project for demonstration of using async file streaming server at client side. For detail, refer to the demo source code at the file test_java/src/test_java.java.

6. test_python: a test demonstration of using async file streaming server at client side for Python. For detail, refer to the demo source code at the file test_python/test_python.py.

7. test_sharp: a test .NET c-sharp project for demonstration of using async file streaming server at client side. For detail, refer to the demo source code at the file test_sharp/Program.cs.


8. Key features are:
    - Simple for development
    - Support of both windows and linux platforms
    - Rich features with many unique ones because of its unique design
    - Superior performance and scalability
    - Continuous file streaming downloading and uploading
    - Cancel supported fully
    - Async file streaming service is totally free to you
