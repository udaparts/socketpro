
#ifndef _UDAPARTS_ASYNC_QUEUE_DEFINES_H_
#define _UDAPARTS_ASYNC_QUEUE_DEFINES_H_

#include "../ucomm.h"

namespace SPA {
    namespace Queue {
        //use built-in chat service id
        static const unsigned int sidQueue = (unsigned int)tagServiceID::sidChat;

        //queue-related request ids
        static const unsigned short idEnqueue = (unsigned short)tagBaseRequestID::idReservedTwo + 1;
        static const unsigned short idDequeue = idEnqueue + 1;
        static const unsigned short idStartTrans = idEnqueue + 2;
        static const unsigned short idEndTrans = idEnqueue + 3;
        static const unsigned short idFlush = idEnqueue + 4;
        static const unsigned short idClose = idEnqueue + 5;
        static const unsigned short idGetKeys = idEnqueue + 6;
        static const unsigned short idEnqueueBatch = idEnqueue + 7;

        //this id is designed for notifying dequeue batch size from server to client
        static const unsigned short idBatchSizeNotified = idEnqueue + 19;

        //error code
        static const int QUEUE_OK = 0;
        static const int QUEUE_TRANS_ALREADY_STARTED = 1;
        static const int QUEUE_TRANS_STARTING_FAILED = 2;
        static const int QUEUE_TRANS_NOT_STARTED_YET = 3;
        static const int QUEUE_TRANS_COMMITTING_FAILED = 4;
        static const int QUEUE_DEQUEUING = 5;
        static const int QUEUE_OTHER_WORKING_WITH_SAME_QUEUE = 6;
        static const int QUEUE_CLOSE_FAILED = 7;
        static const int QUEUE_ENQUEUING_FAILED = 8;

    } //namespace Queue
} //namespace SPA

#endif //_UDAPARTS_ASYNC_QUEUE_DEFINES_H_