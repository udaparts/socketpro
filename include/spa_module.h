
#ifndef _SOCKETPRO_MODULE_IMPLEMENTATION_BASIC_HEADER_H_
#define _SOCKETPRO_MODULE_IMPLEMENTATION_BASIC_HEADER_H_

#include "userver.h"

/*
        Required methods for developing a highly reusable SocketPro server plug-in library containing one or more services from C/C++.
 */

#ifdef __cplusplus
extern "C" {
#endif

    bool U_MODULE_OPENED WINAPI InitServerLibrary(int param); //The method will be called from SocketPro server core right after the library is loaded
    void U_MODULE_OPENED WINAPI UninitServerLibrary(); //The method will be called from SocketPro server core right after the library is loaded
    unsigned short U_MODULE_OPENED WINAPI GetNumOfServices(); //SocketPro will use the method to query how many services the library has defined
    unsigned int U_MODULE_OPENED WINAPI GetAServiceID(unsigned short index); //The method will be called from SocketPro server core to query each service id on zero-based index
    CSvsContext U_MODULE_OPENED WINAPI GetOneSvsContext(unsigned int serviceId); //The method will be called from SocketPro server core to get service context for a given service id
    unsigned short U_MODULE_OPENED WINAPI GetNumOfSlowRequests(unsigned int serviceId); //The method will be called from SocketPro server core to query the number of slow requests for one service id
    unsigned short U_MODULE_OPENED WINAPI GetOneSlowRequestID(unsigned int serviceId, unsigned short index); //The method will be called from SocketPro server core to get a slow request id from given service id and zero-based index

#ifdef __cplusplus
}
#endif

#endif