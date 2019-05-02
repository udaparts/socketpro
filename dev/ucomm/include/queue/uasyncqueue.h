
#ifndef _UDAPARTS_ASYNC_QUEUE_DEFINES_H_
#define _UDAPARTS_ASYNC_QUEUE_DEFINES_H_

#include "../ucomm.h"

namespace SPA {
    namespace Queue {
        //use built-in chat service id
        static const unsigned int sidQueue = sidChat;

        //queue-related request ids
        static const unsigned short idEnqueue = idReservedTwo + 1;
        static const unsigned short idDequeue = idReservedTwo + 2;
        static const unsigned short idStartTrans = idReservedTwo + 3;
        static const unsigned short idEndTrans = idReservedTwo + 4;
        static const unsigned short idFlush = idReservedTwo + 5;
        static const unsigned short idClose = idReservedTwo + 6;
        static const unsigned short idGetKeys = idReservedTwo + 7;
        static const unsigned short idEnqueueBatch = idReservedTwo + 8;

        //this id is designed for notifying dequeue batch size from server to client
        static const unsigned short idBatchSizeNotified = idReservedTwo + 20;

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