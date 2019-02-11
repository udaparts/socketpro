#include "stdafx.h"
#include "phpqueue.h"
#include "phpbuffer.h"

namespace PA {

	const char *CPhpQueue::PHP_QUEUE_KEY = "key";
	const char *CPhpQueue::PHP_QUEUE_MESSAGES = "messages";
	const char *CPhpQueue::PHP_QUEUE_FILESIZE = "fileSize";
	const char *CPhpQueue::PHP_QUEUE_MESSAGES_DEQUEUED = "messagesDequeued";
	const char *CPhpQueue::PHP_QUEUE_BYTES_DEQUEUED = "bytesDequeued";

	CPhpQueue::CPhpQueue(unsigned int poolId, CAsyncQueue *aq, bool locked)
		: CPhpBaseHandler(locked, aq, poolId), m_aq(aq), m_pBuff(new CPhpBuffer) {
	}

	Php::Value CPhpQueue::__get(const Php::Value &name) {
		if (name == "AutoNotified") {
			return m_aq->GetEnqueueNotified();
		}
		else if (name == "DequeueBatchSize") {
			return (int64_t)m_aq->GetDequeueBatchSize();
		}
		return CPhpBaseHandler::__get(name);
	}

	Php::Value CPhpQueue::CloseQueue(Php::Parameters &params) {
		unsigned int timeout;
		std::string key = GetKey(params[0]);
		std::shared_ptr<Php::Value> pErrCode;
		auto c = SetQueueTransCallback(params[1], pErrCode, timeout);
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 2) {
			phpCanceled = params[2];
		}
		auto discarded = SetAbortCallback(phpCanceled, SPA::Queue::idClose, pErrCode ? true : false);
		bool permanent = false;
		if (args > 3) {
			permanent = params[3].boolValue();
		}
		if (pErrCode) {
			std::unique_lock<std::mutex> lk(m_mPhp);
			ReqSyncEnd(m_aq->CloseQueue(key.c_str(), c, permanent, discarded), lk, timeout);
			return *pErrCode;
		}
		return m_aq->CloseQueue(key.c_str(), c, permanent, discarded);
	}

	Php::Value CPhpQueue::EnqueueBatch(Php::Parameters &params) {
		if (!m_pBuff->GetBuffer()->GetSize()) {
			throw Php::Exception("No message batched yet");
		}
		std::string key = GetKey(params[0]);
		unsigned int timeout;
		std::shared_ptr<Php::Value> pIndex;
		auto c = SetEnqueueResCallback(params[1], pIndex, timeout);
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 2) {
			phpCanceled = params[2];
		}
		auto discarded = SetAbortCallback(phpCanceled, SPA::Queue::idEnqueueBatch, pIndex ? true : false);
		if (pIndex) {
			std::unique_lock<std::mutex> lk(m_mPhp);
			ReqSyncEnd(m_aq->EnqueueBatch(key.c_str(), m_pBuff->GetBuffer()->GetBuffer(), m_pBuff->GetBuffer()->GetSize(), c, discarded), lk, timeout);
			return *pIndex;
		}
		return m_aq->EnqueueBatch(key.c_str(), m_pBuff->GetBuffer()->GetBuffer(), m_pBuff->GetBuffer()->GetSize(), c, discarded);
	}

	Php::Value CPhpQueue::Dequeue(Php::Parameters &params) {
		std::string key = GetKey(params[0]);
		unsigned int timeout = m_aq->GetAttachedClientSocket()->GetRecvTimeout();
		bool sync = false;
		Php::Value phpF = params[1];
		if (phpF.isNumeric()) {
			int64_t t = phpF.numericValue();
			if (t >= 0 && t < timeout) {
				timeout = (unsigned int)t;
			}
			sync = true;
		}
		else if (phpF.isBool()) {
			sync = phpF.boolValue();
		}
		else if (phpF.isNull()) {
		}
		else if (!phpF.isCallable()) {
			throw Php::Exception("A callback required for Dequeue final result");
		}
		std::shared_ptr<Php::Value> pF;
		if (sync) {
			pF.reset(new Php::Value);
		}
		CAsyncQueue::DDequeue f = [phpF, sync, pF, this](CAsyncQueue *aq, SPA::UINT64 messages, SPA::UINT64 fileSize, unsigned int messagesDequeued, unsigned int bytesDequeued) {
			if (sync) {
				pF->set(PHP_QUEUE_MESSAGES, (int64_t)messages);
				pF->set(PHP_QUEUE_FILESIZE, (int64_t)fileSize);
				pF->set(PHP_QUEUE_MESSAGES_DEQUEUED, (int64_t)messagesDequeued);
				pF->set(PHP_QUEUE_BYTES_DEQUEUED, (int64_t)bytesDequeued);
				std::unique_lock<std::mutex> lk(this->m_mPhp);
				this->m_cvPhp.notify_all();
			}
			else if (phpF.isCallable()) {
				Php::Value v;
				v.set(PHP_QUEUE_MESSAGES, (int64_t)messages);
				v.set(PHP_QUEUE_FILESIZE, (int64_t)fileSize);
				v.set(PHP_QUEUE_MESSAGES_DEQUEUED, (int64_t)messagesDequeued);
				v.set(PHP_QUEUE_BYTES_DEQUEUED, (int64_t)bytesDequeued);
				phpF(v);
			}
		};
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 2) {
			phpCanceled = params[2];
		}
		auto discarded = SetAbortCallback(phpCanceled, SPA::Queue::idDequeue, sync);
		unsigned int to = 0;
		if (args > 3) {
			if (params[3].isNumeric()) {
				int64_t o = params[3].numericValue();
				if (o < 0) {
					throw Php::Exception("Bad value for Dequeue timeout");
				}
				to = (unsigned int)o;
			}
			else {
				throw Php::Exception("An integer value required for Dequeue timeout");
			}
		}
		if (sync) {
			std::unique_lock<std::mutex> lk(m_mPhp);
			ReqSyncEnd(m_aq->Dequeue(key.c_str(), f, to, discarded), lk, timeout);
			return *pF;
		}
		return m_aq->Dequeue(key.c_str(), f, to, discarded);
	}

	Php::Value CPhpQueue::FlushQueue(Php::Parameters &params) {
		std::string key = GetKey(params[0]);
		unsigned int timeout = m_aq->GetAttachedClientSocket()->GetRecvTimeout();
		bool sync = false;
		Php::Value phpF = params[1];
		if (phpF.isNumeric()) {
			int64_t t = phpF.numericValue();
			if (t >= 0 && t < timeout) {
				timeout = (unsigned int)t;
			}
			sync = true;
		}
		else if (phpF.isBool()) {
			sync = phpF.boolValue();
		}
		else if (phpF.isNull()) {
		}
		else if (!phpF.isCallable()) {
			throw Php::Exception("A callback required for Flush final result");
		}
		std::shared_ptr<Php::Value> pF;
		if (sync) {
			pF.reset(new Php::Value);
		}
		CAsyncQueue::DFlush f = [phpF, pF, this](CAsyncQueue *aq, SPA::UINT64 messages, SPA::UINT64 fileSize) {
			if (pF) {
				pF->set(PHP_QUEUE_MESSAGES, (int64_t)messages);
				pF->set(PHP_QUEUE_FILESIZE, (int64_t)fileSize);
				std::unique_lock<std::mutex> lk(this->m_mPhp);
				this->m_cvPhp.notify_all();
			}
			else if (phpF.isCallable()) {
				Php::Value v;
				v.set(PHP_QUEUE_MESSAGES, (int64_t)messages);
				v.set(PHP_QUEUE_FILESIZE, (int64_t)fileSize);
				phpF(v);
			}
		};
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 2) {
			phpCanceled = params[2];
		}
		auto discarded = SetAbortCallback(phpCanceled, SPA::Queue::idFlush, sync);
		SPA::tagOptimistic option = SPA::oMemoryCached;
		if (args > 3) {
			if (params[3].isNumeric()) {
				int64_t o = params[3].numericValue();
				if (o < 0 || o > SPA::oDiskCommitted) {
					throw Php::Exception("Bad value for memory queue flush status");
				}
				option = (SPA::tagOptimistic)o;
			}
			else {
				throw Php::Exception("An integer value required for memory queue flush status");
			}
		}
		if (sync) {
			std::unique_lock<std::mutex> lk(m_mPhp);
			ReqSyncEnd(m_aq->FlushQueue(key.c_str(), f, option, discarded), lk, timeout);
			return *pF;
		}
		return m_aq->FlushQueue(key.c_str(), f, option, discarded);
	}

	Php::Value CPhpQueue::StartQueueTrans(Php::Parameters &params) {
		unsigned int timeout;
		std::string key = GetKey(params[0]);
		std::shared_ptr<Php::Value> pErrCode;
		auto qt = SetQueueTransCallback(params[1], pErrCode, timeout);
		Php::Value phpCanceled;
		if (params.size() > 2) {
			phpCanceled = params[2];
		}
		auto discarded = SetAbortCallback(phpCanceled, SPA::Queue::idStartTrans, pErrCode ? true : false);
		if (pErrCode) {
			std::unique_lock<std::mutex> lk(m_mPhp);
			ReqSyncEnd(m_aq->StartQueueTrans(key.c_str(), qt, discarded), lk, timeout);
			return *pErrCode;
		}
		return m_aq->StartQueueTrans(key.c_str(), qt, discarded);
	}

	CAsyncQueue::DQueueTrans CPhpQueue::SetQueueTransCallback(const Php::Value& phpTrans, std::shared_ptr<Php::Value> &pErrCode, unsigned int &timeout) {
		timeout = (~0);
		bool sync = false;
		timeout = m_aq->GetAttachedClientSocket()->GetRecvTimeout();
		if (phpTrans.isNumeric()) {
			timeout = (unsigned int)phpTrans.numericValue();
			sync = true;
		}
		else if (phpTrans.isBool()) {
			sync = phpTrans.boolValue();
		}
		else if (phpTrans.isNull()) {
		}
		else if (!phpTrans.isCallable()) {
			throw Php::Exception("A callback required for queue transaction final result");
		}
		if (sync) {
			pErrCode.reset(new Php::Value);
		}
		else {
			pErrCode.reset();
		}
		CAsyncQueue::DQueueTrans qt = [phpTrans, pErrCode, this](CAsyncQueue *aq, int errCode) {
			if (pErrCode) {
				*pErrCode = errCode;
				std::unique_lock<std::mutex> lk(this->m_mPhp);
				this->m_cvPhp.notify_all();
			}
			else if (phpTrans.isCallable()) {
				phpTrans(errCode);
			}
		};
		return qt;
	}

	Php::Value CPhpQueue::EndQueueTrans(Php::Parameters &params) {
		unsigned int timeout;
		bool rollback = params[0].boolValue();
		std::shared_ptr<Php::Value> pErrCode;
		auto qt = SetQueueTransCallback(params[1], pErrCode, timeout);
		Php::Value phpCanceled;
		if (params.size() > 2) {
			phpCanceled = params[2];
		}
		auto discarded = SetAbortCallback(phpCanceled, SPA::Queue::idEndTrans, pErrCode ? true : false);
		if (pErrCode) {
			std::unique_lock<std::mutex> lk(m_mPhp);
			ReqSyncEnd(m_aq->EndQueueTrans(rollback, qt, discarded), lk, timeout);
			return *pErrCode;
		}
		return m_aq->EndQueueTrans(rollback, qt, discarded);
	}

	Php::Value CPhpQueue::GetKeys(Php::Parameters &params) {
		unsigned int timeout = m_aq->GetAttachedClientSocket()->GetRecvTimeout();
		bool sync = false;
		Php::Value phpGK = params[0];
		if (phpGK.isNumeric()) {
			int64_t t = phpGK.numericValue();
			if (t >= 0 && t < timeout) {
				timeout = (unsigned int)t;
			}
			sync = true;
		}
		else if (phpGK.isBool()) {
			sync = phpGK.boolValue();
		}
		else if (phpGK.isNull()) {
		}
		else if (!phpGK.isCallable()) {
			throw Php::Exception("A callback required for GetKeys final result");
		}
		std::shared_ptr<Php::Value> pV;
		if (sync) {
			pV.reset(new Php::Value);
		}
		CAsyncQueue::DGetKeys gk = [phpGK, pV, this](CAsyncQueue *aq, const std::vector<std::string> &v) {
			if (pV) {
				*pV = v;
				std::unique_lock<std::mutex> lk(this->m_mPhp);
				this->m_cvPhp.notify_all();
			}
			else if (phpGK.isCallable()) {
				phpGK(v);
			}
		};
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 1) {
			phpCanceled = params[1];
		}
		auto discarded = SetAbortCallback(phpCanceled, SPA::Queue::idGetKeys, sync);
		if (sync) {
			std::unique_lock<std::mutex> lk(m_mPhp);
			ReqSyncEnd(m_aq->GetKeys(gk, discarded), lk, timeout);
			return *pV;
		}
		return m_aq->GetKeys(gk, discarded);
	}

	CAsyncQueue::DEnqueue CPhpQueue::SetEnqueueResCallback(const Php::Value& phpF, std::shared_ptr<Php::Value> &pF, unsigned int &timeout) {
		timeout = (~0);
		bool sync = false;
		if (phpF.isNumeric()) {
			timeout = (unsigned int)phpF.numericValue();
			sync = true;
		}
		else if (phpF.isBool()) {
			sync = phpF.boolValue();
		}
		else if (phpF.isNull()) {
		}
		else if (!phpF.isCallable()) {
			throw Php::Exception("A callback required for Enqueue final result");
		}
		if (sync) {
			pF.reset(new Php::Value);
		}
		else {
			pF.reset();
		}
		CAsyncQueue::DEnqueue f = [phpF, pF, this](CAsyncQueue *aq, SPA::UINT64 index) {
			if (pF) {
				*pF = (int64_t)index;
				std::unique_lock<std::mutex> lk(this->m_mPhp);
				this->m_cvPhp.notify_all();
			}
			else if (phpF.isCallable()) {
				phpF((int64_t)index);
			}
		};
		return f;
	}

	std::string CPhpQueue::GetKey(const Php::Value &v) {
		std::string key = v.stringValue();
		Trim(key);
		if (!key.size()) {
			throw Php::Exception("Message queue key cannot be empty");
		}
		return key;
	}

	Php::Value CPhpQueue::Enqueue(Php::Parameters &params) {
		std::string key = GetKey(params[0]);
		int64_t idMsg = params[1].numericValue();
		if (idMsg <= SPA::idReservedTwo || idMsg > 0xffff) {
			throw Php::Exception("Bad message request Id");
		}
		unsigned int bytes = 0;
		const unsigned char *pBuffer = nullptr;
		Php::Value v;
		Php::Value &q = params[2];
		if (q.instanceOf((SPA_NS + PHP_BUFFER).c_str())) {
			v = q.call("PopBytes");
			pBuffer = (const unsigned char*)v.rawValue();
			bytes = (unsigned int)v.length();
		}
		else if (!q.isNull()) {
			throw Php::Exception("An instance of CUQueue or null required for Enqueue");
		}
		unsigned int timeout;
		std::shared_ptr<Php::Value> pF;
		auto f = SetEnqueueResCallback(params[3], pF, timeout);
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 4) {
			phpCanceled = params[4];
		}
		auto discarded = SetAbortCallback(phpCanceled, SPA::Queue::idEnqueue, pF ? true : false);
		if (pF) {
			std::unique_lock<std::mutex> lk(m_mPhp);
			ReqSyncEnd(m_aq->Enqueue(key.c_str(), (unsigned short)idMsg, pBuffer, bytes, f, discarded), lk, timeout);
			return *pF;
		}
		return m_aq->Enqueue(key.c_str(), (unsigned short)idMsg, pBuffer, bytes, f, discarded);
	}

	void CPhpQueue::BatchMessage(Php::Parameters &params) {
		int64_t idMsg = params[0].numericValue();
		if (idMsg <= SPA::idReservedTwo || idMsg > 0xffff) {
			throw Php::Exception("Bad message request Id");
		}
		m_pBuff->EnsureBuffer();
		unsigned int bytes = 0;
		const unsigned char *pBuffer = nullptr;
		Php::Value v;
		Php::Value &q = params[1];
		if (q.instanceOf((SPA_NS + PHP_BUFFER).c_str())) {
			v = q.call("PopBytes");
			pBuffer = (const unsigned char*)v.rawValue();
			bytes = (unsigned int)v.length();
		}
		else if (!q.isNull()) {
			throw Php::Exception("An instance of CUQueue or null required for Batch or BatchMessage");
		}
		CAsyncQueue::BatchMessage((unsigned short)idMsg, pBuffer, bytes, *m_pBuff->GetBuffer());
	}

	void CPhpQueue::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpQueue> handler(PHP_QUEUE_HANDLER);
		Register(handler);

		handler.property("idEnqueue", SPA::Queue::idEnqueue, Php::Const);
		handler.property("idDequeue", SPA::Queue::idDequeue, Php::Const);
		handler.property("idStartTrans", SPA::Queue::idStartTrans, Php::Const);
		handler.property("idEndTrans", SPA::Queue::idEndTrans, Php::Const);
		handler.property("idFlush", SPA::Queue::idFlush, Php::Const);
		handler.property("idClose", SPA::Queue::idClose, Php::Const);
		handler.property("idGetKeys", SPA::Queue::idGetKeys, Php::Const);
		handler.property("idEnqueueBatch", SPA::Queue::idEnqueueBatch, Php::Const);
		handler.property("idBatchSizeNotified", SPA::Queue::idBatchSizeNotified, Php::Const);

		handler.property("OK", SPA::Queue::QUEUE_OK, Php::Const);
		handler.property("TRANS_ALREADY_STARTED", SPA::Queue::QUEUE_TRANS_ALREADY_STARTED, Php::Const);
		handler.property("TRANS_STARTING_FAILED", SPA::Queue::QUEUE_TRANS_STARTING_FAILED, Php::Const);
		handler.property("TRANS_NOT_STARTED_YET", SPA::Queue::QUEUE_TRANS_NOT_STARTED_YET, Php::Const);
		handler.property("TRANS_COMMITTING_FAILED", SPA::Queue::QUEUE_TRANS_COMMITTING_FAILED, Php::Const);
		handler.property("DEQUEUING", SPA::Queue::QUEUE_DEQUEUING, Php::Const);
		handler.property("OTHER_WORKING_WITH_SAME_QUEUE", SPA::Queue::QUEUE_OTHER_WORKING_WITH_SAME_QUEUE, Php::Const);
		handler.property("CLOSE_FAILED", SPA::Queue::QUEUE_CLOSE_FAILED, Php::Const);
		handler.property("ENQUEUING_FAILED", SPA::Queue::QUEUE_ENQUEUING_FAILED, Php::Const);

		handler.method("Enqueue", &CPhpQueue::Enqueue, {
			Php::ByVal(PHP_QUEUE_KEY, Php::Type::String),
			Php::ByVal("idMessage", Php::Type::Numeric),
			Php::ByVal(PHP_SENDREQUEST_BUFF, Php::Type::Null),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("Close", &CPhpQueue::CloseQueue, {
			Php::ByVal(PHP_QUEUE_KEY, Php::Type::String),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("GetKeys", &CPhpQueue::GetKeys, {
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("StartTrans", &CPhpQueue::StartQueueTrans, {
			Php::ByVal(PHP_QUEUE_KEY, Php::Type::String),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("EndTrans", &CPhpQueue::EndQueueTrans, {
			Php::ByVal("rollback", Php::Type::Bool),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("Flush", &CPhpQueue::FlushQueue, {
			Php::ByVal(PHP_QUEUE_KEY, Php::Type::String),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("Dequeue", &CPhpQueue::Dequeue, {
			Php::ByVal(PHP_QUEUE_KEY, Php::Type::String),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("Batch", &CPhpQueue::BatchMessage, {
			Php::ByVal("idMsg", Php::Type::Numeric),
			Php::ByVal(PHP_SENDREQUEST_BUFF, Php::Type::Null)
		});
		handler.method("BatchMessage", &CPhpQueue::BatchMessage, {
			Php::ByVal("idMsg", Php::Type::Numeric),
			Php::ByVal(PHP_SENDREQUEST_BUFF, Php::Type::Null)
		});
		handler.method("EnqueueBatch", &CPhpQueue::EnqueueBatch, {
			Php::ByVal(PHP_QUEUE_KEY, Php::Type::String),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("BatchEnqueue", &CPhpQueue::EnqueueBatch, {
			Php::ByVal(PHP_QUEUE_KEY, Php::Type::String),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		cs.add(handler);
	}
}
