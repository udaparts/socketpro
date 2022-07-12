# SocketPro libraries and others

Here is the list of libraries according to sub directory name:

- dotnet ==>> SocketPro adapter for .NET development on all window and linux platforms with either Visual studio, monodevelop or other IDE tools
- free_services ==>> A directory for an array of SocketPro pre-compiled components with opened source codes for their implementations
- gspa ==>> SocketPro adapter for Golang
- java ==>> A pre-compiled SocketPro adapter for Java cross-platform development
- js ==> SocketPro adapters for browser JavaScript and Node.js
- linux ==> Native libraries for four linux variants, CentOS/Redhat/Fedora, OpenSUSE, Debian and Ubuntu/Mint
- spa ==> SocketPro adapter for Python version 2.7 or later
- win ==> Native libraries for windows xp or later platforms
- wince ==> SocketPro .NET adapter for window ce platforms and its client core library usocket.dll (armv4i and x86). The library can be available for other architectures.

Certificates and keys are used for TLSv1.x tests:

- ca.cert.pem A root certificate that is used to sign the below certificate intermediate.cert.pem or intermediate.pfx
- intermediate.cert.pem A certificate signed by the above root certificate ca.cert.pem
- intermediate.key.pem A private key protected with password (mypassword), which is associated with certificate intermediate.cert.pem
- intermediate.pfx certificate and private key protected with password (mypassword) in pfx format for intermediate.cert.pem and intermediate.key.pem

Sample test database

- sakila.db a sqlite database for MySQL famous sample test database sakila

We may gradually add more libraries here for other development environments in the future.
