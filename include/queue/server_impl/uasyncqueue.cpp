
#include "asyncqueueimpl.h"

std::shared_ptr<SPA::ServerSide::CSocketProService<SPA::ServerSide::CAsyncQueueImpl> > g_pAsyncQueue;
static const unsigned int DEFAULT_DEQUEUE_BATCH_SIZE = 16384; //default dequeue batch size
static const unsigned int MIN_DEQUEUE_BATCH_SIZE = 2048;

bool WINAPI InitServerLibrary(int param) {
    unsigned options = (unsigned int) param;
    SPA::ServerSide::CAsyncQueueImpl::m_bNoAuto = (unsigned char) (options >> 24);
    unsigned int batchSize = (options & 0xffffff);
    if (!batchSize) {
        batchSize = DEFAULT_DEQUEUE_BATCH_SIZE;
    } else if (batchSize < MIN_DEQUEUE_BATCH_SIZE) {
        batchSize = MIN_DEQUEUE_BATCH_SIZE;
    }
    SPA::ServerSide::CAsyncQueueImpl::m_nBatchSize = batchSize;
    g_pAsyncQueue.reset(new SPA::ServerSide::CSocketProService<SPA::ServerSide::CAsyncQueueImpl>(SPA::Queue::sidQueue, SPA::taNone));
    return true;
}

void WINAPI UninitServerLibrary() {
    g_pAsyncQueue.reset();
}

unsigned short WINAPI GetNumOfServices() {
    return 1; //The library exposes 1 service only
}

unsigned int WINAPI GetAServiceID(unsigned short index) {
    if (index == 0)
        return SPA::Queue::sidQueue;
    return 0;
}

CSvsContext WINAPI GetOneSvsContext(unsigned int serviceId) {
    CSvsContext sc;
    if (g_pAsyncQueue && serviceId == SPA::Queue::sidQueue)
        sc = g_pAsyncQueue->GetSvsContext();
    else
        memset(&sc, 0, sizeof (sc));
    return sc;
}

unsigned short WINAPI GetNumOfSlowRequests(unsigned int serviceId) {
    return 6; //The service only has six slow requests
}

unsigned short WINAPI GetOneSlowRequestID(unsigned int serviceId, unsigned short index) {
    //The following six requests are slow ones
    switch (index) {
        case 0:
            return SPA::Queue::idDequeue;
            break;
        case 1:
            return SPA::Queue::idEnqueue;
            break;
        case 2:
            return SPA::Queue::idFlush;
            break;
        case 3:
            return SPA::Queue::idClose;
            break;
        case 4:
            return SPA::Queue::idEndTrans;
            break;
        case 5:
            return SPA::Queue::idEnqueueBatch;
            break;
        default:
            break;
    }
    return 0;
}
