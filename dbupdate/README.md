# Real-time Database Change Notification

A public repository for implementation of middle-tier application real-time data cache sourced from database change by use of database triggers and UDAParts (http://www.udaparts.com) SocketPro communication framework (https://github.com/udaparts/socketpro). Further, you could use its base library udbubase to notify any events from any applications other than database systems.
To use the libraries, you need to complete the following three setps:

  - Register the database library for each of database systems after consulting database system documentation for how to create user defined functions specifically,
  - Create three user defined functions, SetConnectionString, NotifyDatabaseEvent, and GetConnections. For detail, refer to registration.sql for each of database systems,
  - Create database triggers, and refer to these functions inside triggers.

The repository currently contains the following projects:

1. spdb2push: a project for creating DB2 server plugin
2. sporaclepush: a project for creating oracle server plugin
3. test_udbubase: a project for demonstration of usages of base library udbubase
4. udbubase: a project for creating basic real-time notification library which can be used by any applications other than database server applications
