# SocketPro server plug-in development with C/C++

1.	SocketPro supports server side plug-in development. A SocketPro server plugin is actually a dynamic library, which must expose the following required seven functions (See the file socketpro/include/spa_module.h).

	- bool WINAPI InitServerLibrary (int param); //The method will be called from SocketPro server core right after the library is loaded
	- void WINAPI UninitServerLibrary(); //The method will be called from SocketPro server core right before the library is going to be unloaded
	- unsigned short WINAPI GetNumOfServices(); //SocketPro will use the method to query how many services the library has defined
	- unsigned int WINAPI GetAServiceID(unsigned short index); //The method will be called from SocketPro server core to query each service id on zero-based index
	- CSvsContext WINAPI GetOneSvsContext(unsigned int serviceId); //The method will be called from SocketPro server core to get service context for a given service id
	- unsigned short WINAPI GetNumOfSlowRequests(unsigned int serviceId); //The method will be called from SocketPro server core to query the number of slow requests for one service id
	- unsigned short WINAPI GetOneSlowRequestID(unsigned int serviceId, unsigned short index); //The method will be called from SocketPro server core to get a slow request id from given service id and zero-based index
	
2.	After creating such a library, you can load it at runtime by calling the method SocketProAdapter.ServerSide.CSocketProServer.DllManager.AddALibrary in C#. You can do so for all other development environments similarly.

3.	There are two reusable SocketPro server plugins available for you to study at this time.
	- remotefile: async client/server file exchange service. Its pre-compiled plugin is free to the public. The precompiled library is located at the directory ../socketpro/bin/free_services/file
	- uasyncqueue: SocketPro persistent message queue service. Its pre-compiled plugin is free to the public. The precompiled library is located at the directory ../socketpro/bin/free_services/queue
