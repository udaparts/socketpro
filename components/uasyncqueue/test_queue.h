
#ifndef _TEST_ASYNC_QUEUE_DEFINES_H_
#define _TEST_ASYNC_QUEUE_DEFINES_H_

#include "../../include/ucomm.h"

namespace SPA {
    static const unsigned short idMessage0 = (unsigned short)tagBaseRequestID::idReservedTwo + 100;
    static const unsigned short idMessage1 = idMessage0 + 1;
    static const unsigned short idMessage2 = idMessage0 + 2;
    static const unsigned short idMessage3 = idMessage0 + 3;
    static const unsigned short idMessage4 = idMessage0 + 4;
} //namespace SPA

#endif //_TEST_ASYNC_QUEUE_DEFINES_H_