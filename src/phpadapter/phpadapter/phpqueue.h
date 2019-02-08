#ifndef SPA_PHP_ASYNC_QUEUE_HANDER_H
#define SPA_PHP_ASYNC_QUEUE_HANDER_H

#include "basehandler.h"

namespace PA {

	typedef SPA::ClientSide::CAsyncQueue CAsyncQueue;
	typedef SPA::ClientSide::CSocketPool<CAsyncQueue> CPhpQueuePool;

	class CPhpQueue : public CPhpBaseHandler
	{
	public:
		CPhpQueue(unsigned int poolId, CAsyncQueue *aq, bool locked);
		CPhpQueue(const CPhpQueue &q) = delete;

	public:
		CPhpQueue& operator=(const CPhpQueue &q) = delete;
		static void RegisterInto(Php::Namespace &cs);
		Php::Value __get(const Php::Value &name);

	private:
		Php::Value CloseQueue(Php::Parameters &params);
		Php::Value GetKeys(Php::Parameters &params);
		Php::Value StartQueueTrans(Php::Parameters &params);
		Php::Value EndQueueTrans(Php::Parameters &params);
		Php::Value FlushQueue(Php::Parameters &params);
		Php::Value Dequeue(Php::Parameters &params);
		Php::Value Enqueue(Php::Parameters &params);
		void BatchMessage(Php::Parameters &params);
		Php::Value EnqueueBatch(Php::Parameters &params);

		CAsyncQueue::DEnqueue SetEnqueueResCallback(const Php::Value& phpDl, std::shared_ptr<Php::Value> &pV, unsigned int &timeout);
		std::string GetKey(const Php::Value &v);
		CAsyncQueue::DQueueTrans SetQueueTransCallback(const Php::Value& phpTrans, std::shared_ptr<Php::Value> &pV, unsigned int &timeout);

	private:
		CAsyncQueue *m_aq;
		std::shared_ptr<CPhpBuffer> m_pBuff;
		static const char *PHP_QUEUE_KEY;
		static const char *PHP_QUEUE_MESSAGES;
		static const char *PHP_QUEUE_FILESIZE;
		static const char *PHP_QUEUE_MESSAGES_DEQUEUED;
		static const char *PHP_QUEUE_BYTES_DEQUEUED;
	};

} //namespace PA

#endif