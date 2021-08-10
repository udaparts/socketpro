
#ifndef _UMSSQL_SOCKETPRO_H_
#define _UMSSQL_SOCKETPRO_H_

#include "../ucomm.h"

namespace SPA {
    namespace SqlServer {
        static const unsigned int sidMsSql = (unsigned int) tagServiceID::sidReserved + 0x6FFFFFF2; //asynchronous MS SQL stream service id

        static const unsigned int READ_ONLY = 0x20000000;
        static const unsigned int USE_ENCRYPTION = 0x40000000;
    } //namespace SqlServer
} //namespace SPA
#endif
