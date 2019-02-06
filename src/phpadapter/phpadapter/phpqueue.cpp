#include "stdafx.h"
#include "phpqueue.h"

namespace PA {

	const char *CPhpQueue::PHP_QUEUE_KEY = "key";

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
		std::string key = params[0].stringValue();
		Trim(key);
		if (!key.size()) {
			throw Php::Exception("Message queue key cannot be empty");
		}
		unsigned int timeout = m_aq->GetAttachedClientSocket()->GetRecvTimeout();
		bool sync = false;
		Php::Value phpC = params[1];
		if (phpC.isNumeric()) {
			int64_t t = phpC.numericValue();
			if (t >= 0 && t < timeout) {
				timeout = (unsigned int)t;
			}
			sync = true;
		}
		else if (phpC.isBool()) {
			sync = phpC.boolValue();
		}
		else if (phpC.isNull()) {
		}
		else if (!phpC.isCallable()) {
			throw Php::Exception("A callback required for CloseQueue final result");
		}
		std::shared_ptr<int> pErrCode;
		if (sync) {
			pErrCode.reset(new int);
		}
		SPA::ClientSide::CAsyncQueue::DClose c = [phpC, sync, pErrCode, this](SPA::ClientSide::CAsyncQueue *aq, int errCode) {
			if (sync) {
				*pErrCode = errCode;
				std::unique_lock<std::mutex> lk(this->m_aq->m_mPhp);
				this->m_aq->m_cvPhp.notify_all();
			}
			else if (phpC.isCallable()) {
				phpC(errCode);
			}
		};
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 2) {
			phpCanceled = params[2];
			if (phpCanceled.isNull()) {
			}
			else if (!phpCanceled.isCallable()) {
				throw Php::Exception("A callback required for CloseQueue aborting event");
			}
		}
		tagRequestReturnStatus rrs = rrsOk;
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = [phpCanceled, sync, &rrs, this](SPA::ClientSide::CAsyncServiceHandler *ash, bool canceled) {
			if (phpCanceled.isCallable()) {
				phpCanceled(canceled);
			}
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_aq->m_mPhp);
				rrs = canceled ? rrsCanceled : rrsClosed;
				this->m_aq->m_cvPhp.notify_all();
			}
		};
		bool permanent = false;
		if (args > 3) {
			permanent = params[3].boolValue();
		}
		if (sync) {
			std::unique_lock<std::mutex> lk(m_aq->m_mPhp);
			if (!m_aq->CloseQueue(key.c_str(), c, permanent, discarded)) {
				Unlock();
				throw Php::Exception(PHP_SOCKET_CLOSED);
			}
			Unlock();
			auto status = m_aq->m_cvPhp.wait_for(lk, std::chrono::milliseconds(timeout));
			if (status == std::cv_status::timeout) {
				rrs = rrsTimeout;
			}
			switch (rrs) {
			case rrsServerException:
				throw Php::Exception(PHP_SERVER_EXCEPTION);
			case rrsCanceled:
				throw Php::Exception(PHP_REQUEST_CANCELED);
			case rrsClosed:
				throw Php::Exception(PHP_SOCKET_CLOSED);
			case rrsTimeout:
				throw Php::Exception(PHP_REQUEST_TIMEOUT);
			default:
				break;
			}
			return *pErrCode;
		}
		return m_aq->CloseQueue(key.c_str(), c, permanent, discarded) ? rrsOk : rrsClosed;
	}

	Php::Value CPhpQueue::EnqueueBatch(Php::Parameters &params) {
		if (!m_pBuff->GetBuffer()->GetSize()) {
			throw Php::Exception("No message batched yet");
		}
		std::string key = params[0].stringValue();
		Trim(key);
		if (!key.size()) {
			throw Php::Exception("Message queue key cannot be empty");
		}
		unsigned int timeout = m_aq->GetAttachedClientSocket()->GetRecvTimeout();
		bool sync = false;
		Php::Value phpC = params[1];
		if (phpC.isNumeric()) {
			int64_t t = phpC.numericValue();
			if (t >= 0 && t < timeout) {
				timeout = (unsigned int)t;
			}
			sync = true;
		}
		else if (phpC.isBool()) {
			sync = phpC.boolValue();
		}
		else if (phpC.isNull()) {
		}
		else if (!phpC.isCallable()) {
			throw Php::Exception("A callback required for BatchEnqueue or EnqueueBatch final result");
		}
		std::shared_ptr<Php::Value> pIndex;
		if (sync) {
			pIndex.reset(new Php::Value);
		}
		SPA::ClientSide::CAsyncQueue::DEnqueue c = [phpC, sync, pIndex, this](SPA::ClientSide::CAsyncQueue *aq, SPA::UINT64 index) {
			if (sync) {
				*pIndex = (int64_t)index;
				std::unique_lock<std::mutex> lk(this->m_aq->m_mPhp);
				this->m_aq->m_cvPhp.notify_all();
			}
			else if (phpC.isCallable()) {
				phpC((int64_t)index);
			}
		};
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 2) {
			phpCanceled = params[2];
			if (phpCanceled.isNull()) {
			}
			else if (!phpCanceled.isCallable()) {
				throw Php::Exception("A callback required for BatchEnqueue or EnqueueBatch aborting event");
			}
		}
		tagRequestReturnStatus rrs = rrsOk;
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = [phpCanceled, sync, &rrs, this](SPA::ClientSide::CAsyncServiceHandler *ash, bool canceled) {
			if (phpCanceled.isCallable()) {
				phpCanceled(canceled);
			}
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_aq->m_mPhp);
				rrs = canceled ? rrsCanceled : rrsClosed;
				this->m_aq->m_cvPhp.notify_all();
			}
		};
		if (sync) {
			std::unique_lock<std::mutex> lk(m_aq->m_mPhp);
			if (!m_aq->EnqueueBatch(key.c_str(), m_pBuff->GetBuffer()->GetBuffer(), m_pBuff->GetBuffer()->GetSize(), c, discarded)) {
				Unlock();
				m_pBuff->GetBuffer()->SetSize(0);
				throw Php::Exception(PHP_SOCKET_CLOSED);
			}
			m_pBuff->GetBuffer()->SetSize(0);
			Unlock();
			auto status = m_aq->m_cvPhp.wait_for(lk, std::chrono::milliseconds(timeout));
			if (status == std::cv_status::timeout) {
				rrs = rrsTimeout;
			}
			switch (rrs) {
			case rrsServerException:
				throw Php::Exception(PHP_SERVER_EXCEPTION);
			case rrsCanceled:
				throw Php::Exception(PHP_REQUEST_CANCELED);
			case rrsClosed:
				throw Php::Exception(PHP_SOCKET_CLOSED);
			case rrsTimeout:
				throw Php::Exception(PHP_REQUEST_TIMEOUT);
			default:
				break;
			}
			return *pIndex;
		}
		return m_aq->EnqueueBatch(key.c_str(), m_pBuff->GetBuffer()->GetBuffer(), m_pBuff->GetBuffer()->GetSize(), c, discarded) ? rrsOk : rrsClosed;
	}

	Php::Value CPhpQueue::Dequeue(Php::Parameters &params) {
		std::string key = params[0].stringValue();
		Trim(key);
		if (!key.size()) {
			throw Php::Exception("Message queue key cannot be empty");
		}
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
		SPA::ClientSide::CAsyncQueue::DDequeue f = [phpF, sync, pF, this](SPA::ClientSide::CAsyncQueue *aq, SPA::UINT64 messages, SPA::UINT64 fileSize, unsigned int messagesDequeued, unsigned int bytesDequeued) {
			if (sync) {
				pF->set("messages", (int64_t)messages);
				pF->set("fileSize", (int64_t)fileSize);
				pF->set("messagesDequeued", (int64_t)messagesDequeued);
				pF->set("bytesDequeued", (int64_t)bytesDequeued);
				std::unique_lock<std::mutex> lk(this->m_aq->m_mPhp);
				this->m_aq->m_cvPhp.notify_all();
			}
			else if (phpF.isCallable()) {
				Php::Value v;
				v.set("messages", (int64_t)messages);
				v.set("fileSize", (int64_t)fileSize);
				v.set("messagesDequeued", (int64_t)messagesDequeued);
				v.set("bytesDequeued", (int64_t)bytesDequeued);
				phpF(v);
			}
		};
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 2) {
			phpCanceled = params[2];
			if (phpCanceled.isNull()) {
			}
			else if (!phpCanceled.isCallable()) {
				throw Php::Exception("A callback required for Dequeue aborting event");
			}
		}
		tagRequestReturnStatus rrs = rrsOk;
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = [phpCanceled, sync, &rrs, this](SPA::ClientSide::CAsyncServiceHandler *ash, bool canceled) {
			if (phpCanceled.isCallable()) {
				phpCanceled(canceled);
			}
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_aq->m_mPhp);
				rrs = canceled ? rrsCanceled : rrsClosed;
				this->m_aq->m_cvPhp.notify_all();
			}
		};
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
			std::unique_lock<std::mutex> lk(m_aq->m_mPhp);
			if (!m_aq->Dequeue(key.c_str(), f, to, discarded)) {
				Unlock();
				throw Php::Exception(PHP_SOCKET_CLOSED);
			}
			Unlock();
			auto status = m_aq->m_cvPhp.wait_for(lk, std::chrono::milliseconds(timeout));
			if (status == std::cv_status::timeout) {
				rrs = rrsTimeout;
			}
			switch (rrs) {
			case rrsServerException:
				throw Php::Exception(PHP_SERVER_EXCEPTION);
			case rrsCanceled:
				throw Php::Exception(PHP_REQUEST_CANCELED);
			case rrsClosed:
				throw Php::Exception(PHP_SOCKET_CLOSED);
			case rrsTimeout:
				throw Php::Exception(PHP_REQUEST_TIMEOUT);
			default:
				break;
			}
			return *pF;
		}
		return m_aq->Dequeue(key.c_str(), f, to, discarded) ? rrsOk : rrsClosed;
	}

	Php::Value CPhpQueue::FlushQueue(Php::Parameters &params) {
		std::string key = params[0].stringValue();
		Trim(key);
		if (!key.size()) {
			throw Php::Exception("Message queue key cannot be empty");
		}
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
			throw Php::Exception("A callback required for FlushQueue final result");
		}
		std::shared_ptr<Php::Value> pF;
		if (sync) {
			pF.reset(new Php::Value);
		}
		SPA::ClientSide::CAsyncQueue::DFlush f = [phpF, sync, pF, this](SPA::ClientSide::CAsyncQueue *aq, SPA::UINT64 messages, SPA::UINT64 fileSize) {
			if (sync) {
				pF->set("messages", (int64_t)messages);
				pF->set("fileSize", (int64_t)fileSize);
				std::unique_lock<std::mutex> lk(this->m_aq->m_mPhp);
				this->m_aq->m_cvPhp.notify_all();
			}
			else if (phpF.isCallable()) {
				Php::Value v;
				v.set("messages", (int64_t)messages);
				v.set("fileSize", (int64_t)fileSize);
				phpF(v);
			}
		};
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 2) {
			phpCanceled = params[2];
			if (phpCanceled.isNull()) {
			}
			else if (!phpCanceled.isCallable()) {
				throw Php::Exception("A callback required for FlushQueue aborting event");
			}
		}
		tagRequestReturnStatus rrs = rrsOk;
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = [phpCanceled, sync, &rrs, this](SPA::ClientSide::CAsyncServiceHandler *ash, bool canceled) {
			if (phpCanceled.isCallable()) {
				phpCanceled(canceled);
			}
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_aq->m_mPhp);
				rrs = canceled ? rrsCanceled : rrsClosed;
				this->m_aq->m_cvPhp.notify_all();
			}
		};
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
			std::unique_lock<std::mutex> lk(m_aq->m_mPhp);
			if (!m_aq->FlushQueue(key.c_str(), f, option, discarded)) {
				Unlock();
				throw Php::Exception(PHP_SOCKET_CLOSED);
			}
			Unlock();
			auto status = m_aq->m_cvPhp.wait_for(lk, std::chrono::milliseconds(timeout));
			if (status == std::cv_status::timeout) {
				rrs = rrsTimeout;
			}
			switch (rrs) {
			case rrsServerException:
				throw Php::Exception(PHP_SERVER_EXCEPTION);
			case rrsCanceled:
				throw Php::Exception(PHP_REQUEST_CANCELED);
			case rrsClosed:
				throw Php::Exception(PHP_SOCKET_CLOSED);
			case rrsTimeout:
				throw Php::Exception(PHP_REQUEST_TIMEOUT);
			default:
				break;
			}
			return *pF;
		}
		return m_aq->FlushQueue(key.c_str(), f, option, discarded) ? rrsOk : rrsClosed;
	}

	Php::Value CPhpQueue::StartQueueTrans(Php::Parameters &params) {
		std::string key = params[0].stringValue();
		Trim(key);
		if (!key.size()) {
			throw Php::Exception("Message queue key cannot be empty");
		}
		unsigned int timeout = m_aq->GetAttachedClientSocket()->GetRecvTimeout();
		bool sync = false;
		Php::Value phpTrans = params[1];
		if (phpTrans.isNumeric()) {
			int64_t t = phpTrans.numericValue();
			if (t >= 0 && t < timeout) {
				timeout = (unsigned int)t;
			}
			sync = true;
		}
		else if (phpTrans.isBool()) {
			sync = phpTrans.boolValue();
		}
		else if (phpTrans.isNull()) {
		}
		else if (!phpTrans.isCallable()) {
			throw Php::Exception("A callback required for StartTrans final result");
		}
		std::shared_ptr<int> pErrCode;
		if (sync) {
			pErrCode.reset(new int);
		}
		SPA::ClientSide::CAsyncQueue::DQueueTrans qt = [phpTrans, sync, pErrCode, this](SPA::ClientSide::CAsyncQueue *aq, int errCode) {
			if (sync) {
				*pErrCode = errCode;
				std::unique_lock<std::mutex> lk(this->m_aq->m_mPhp);
				this->m_aq->m_cvPhp.notify_all();
			}
			else if (phpTrans.isCallable()) {
				phpTrans(errCode);
			}
		};
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 2) {
			phpCanceled = params[2];
			if (phpCanceled.isNull()) {
			}
			else if (!phpCanceled.isCallable()) {
				throw Php::Exception("A callback required for StartTrans aborting event");
			}
		}
		tagRequestReturnStatus rrs = rrsOk;
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = [phpCanceled, sync, &rrs, this](SPA::ClientSide::CAsyncServiceHandler *ash, bool canceled) {
			if (phpCanceled.isCallable()) {
				phpCanceled(canceled);
			}
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_aq->m_mPhp);
				rrs = canceled ? rrsCanceled : rrsClosed;
				this->m_aq->m_cvPhp.notify_all();
			}
		};
		if (sync) {
			std::unique_lock<std::mutex> lk(m_aq->m_mPhp);
			if (!m_aq->StartQueueTrans(key.c_str(), qt, discarded)) {
				Unlock();
				throw Php::Exception(PHP_SOCKET_CLOSED);
			}
			Unlock();
			auto status = m_aq->m_cvPhp.wait_for(lk, std::chrono::milliseconds(timeout));
			if (status == std::cv_status::timeout) {
				rrs = rrsTimeout;
			}
			switch (rrs) {
			case rrsServerException:
				throw Php::Exception(PHP_SERVER_EXCEPTION);
			case rrsCanceled:
				throw Php::Exception(PHP_REQUEST_CANCELED);
			case rrsClosed:
				throw Php::Exception(PHP_SOCKET_CLOSED);
			case rrsTimeout:
				throw Php::Exception(PHP_REQUEST_TIMEOUT);
			default:
				break;
			}
			return *pErrCode;
		}
		return m_aq->StartQueueTrans(key.c_str(), qt, discarded) ? rrsOk : rrsClosed;
	}

	Php::Value CPhpQueue::EndQueueTrans(Php::Parameters &params) {
		bool rollback = params[0].boolValue();
		unsigned int timeout = m_aq->GetAttachedClientSocket()->GetRecvTimeout();
		bool sync = false;
		Php::Value phpTrans = params[1];
		if (phpTrans.isNumeric()) {
			int64_t t = phpTrans.numericValue();
			if (t >= 0 && t < timeout) {
				timeout = (unsigned int)t;
			}
			sync = true;
		}
		else if (phpTrans.isBool()) {
			sync = phpTrans.boolValue();
		}
		else if (phpTrans.isNull()) {
		}
		else if (!phpTrans.isCallable()) {
			throw Php::Exception("A callback required for EndTrans final result");
		}
		std::shared_ptr<int> pErrCode;
		if (sync) {
			pErrCode.reset(new int);
		}
		SPA::ClientSide::CAsyncQueue::DQueueTrans qt = [phpTrans, sync, pErrCode, this](SPA::ClientSide::CAsyncQueue *aq, int errCode) {
			if (sync) {
				*pErrCode = errCode;
				std::unique_lock<std::mutex> lk(this->m_aq->m_mPhp);
				this->m_aq->m_cvPhp.notify_all();
			}
			else if (phpTrans.isCallable()) {
				phpTrans(errCode);
			}
		};
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 2) {
			phpCanceled = params[2];
			if (phpCanceled.isNull()) {
			}
			else if (!phpCanceled.isCallable()) {
				throw Php::Exception("A callback required for EndTrans aborting event");
			}
		}
		tagRequestReturnStatus rrs = rrsOk;
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = [phpCanceled, sync, &rrs, this](SPA::ClientSide::CAsyncServiceHandler *ash, bool canceled) {
			if (phpCanceled.isCallable()) {
				phpCanceled(canceled);
			}
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_aq->m_mPhp);
				rrs = canceled ? rrsCanceled : rrsClosed;
				this->m_aq->m_cvPhp.notify_all();
			}
		};
		if (sync) {
			std::unique_lock<std::mutex> lk(m_aq->m_mPhp);
			if (!m_aq->EndQueueTrans(rollback, qt, discarded)) {
				Unlock();
				throw Php::Exception(PHP_SOCKET_CLOSED);
			}
			Unlock();
			auto status = m_aq->m_cvPhp.wait_for(lk, std::chrono::milliseconds(timeout));
			if (status == std::cv_status::timeout) {
				rrs = rrsTimeout;
			}
			switch (rrs) {
			case rrsServerException:
				throw Php::Exception(PHP_SERVER_EXCEPTION);
			case rrsCanceled:
				throw Php::Exception(PHP_REQUEST_CANCELED);
			case rrsClosed:
				throw Php::Exception(PHP_SOCKET_CLOSED);
			case rrsTimeout:
				throw Php::Exception(PHP_REQUEST_TIMEOUT);
			default:
				break;
			}
			return *pErrCode;
		}
		return m_aq->EndQueueTrans(rollback, qt, discarded) ? rrsOk : rrsClosed;
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
		SPA::ClientSide::CAsyncQueue::DGetKeys gk = [phpGK, sync, pV, this](SPA::ClientSide::CAsyncQueue *aq, const std::vector<std::string> &v) {
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_aq->m_mPhp);
				*pV = v;
				this->m_aq->m_cvPhp.notify_all();
			}
			else if (phpGK.isCallable()) {
				phpGK(v);
			}
		};
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 1) {
			phpCanceled = params[1];
			if (phpCanceled.isNull()) {
			}
			else if (!phpCanceled.isCallable()) {
				throw Php::Exception("A callback required for GetKeys aborting event");
			}
		}
		tagRequestReturnStatus rrs = rrsOk;
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = [phpCanceled, sync, &rrs, this](SPA::ClientSide::CAsyncServiceHandler *ash, bool canceled) {
			if (phpCanceled.isCallable()) {
				phpCanceled(canceled);
			}
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_aq->m_mPhp);
				rrs = canceled ? rrsCanceled : rrsClosed;
				this->m_aq->m_cvPhp.notify_all();
			}
		};
		if (sync) {
			std::unique_lock<std::mutex> lk(m_aq->m_mPhp);
			if (!m_aq->GetKeys(gk, discarded)) {
				Unlock();
				throw Php::Exception(PHP_SOCKET_CLOSED);
			}
			Unlock();
			auto status = m_aq->m_cvPhp.wait_for(lk, std::chrono::milliseconds(timeout));
			if (status == std::cv_status::timeout) {
				rrs = rrsTimeout;
			}
			switch (rrs) {
			case rrsServerException:
				throw Php::Exception(PHP_SERVER_EXCEPTION);
			case rrsCanceled:
				throw Php::Exception(PHP_REQUEST_CANCELED);
			case rrsClosed:
				throw Php::Exception(PHP_SOCKET_CLOSED);
			case rrsTimeout:
				throw Php::Exception(PHP_REQUEST_TIMEOUT);
			default:
				break;
			}
			return *pV;
		}
		return m_aq->GetKeys(gk, discarded) ? rrsOk : rrsClosed;
	}

	Php::Value CPhpQueue::Enqueue(Php::Parameters &params) {
		std::string key = params[0].stringValue();
		Trim(key);
		if (!key.size()) {
			throw Php::Exception("Message queue key cannot be empty");
		}
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
		unsigned int timeout = m_aq->GetAttachedClientSocket()->GetRecvTimeout();
		bool sync = false;
		Php::Value phpF = params[3];
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
			throw Php::Exception("A callback required for Enqueue final result");
		}
		std::shared_ptr<Php::Value> pF;
		if (sync) {
			pF.reset(new Php::Value);
		}
		SPA::ClientSide::CAsyncQueue::DEnqueue f = [phpF, sync, pF, this](SPA::ClientSide::CAsyncQueue *aq, SPA::UINT64 index) {
			if (sync) {
				*pF = (int64_t)index;
				std::unique_lock<std::mutex> lk(this->m_aq->m_mPhp);
				this->m_aq->m_cvPhp.notify_all();
			}
			else if (phpF.isCallable()) {
				phpF((int64_t)index);
			}
		};
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 4) {
			phpCanceled = params[4];
			if (phpCanceled.isNull()) {
			}
			else if (!phpCanceled.isCallable()) {
				throw Php::Exception("A callback required for Enqueue aborting event");
			}
		}
		tagRequestReturnStatus rrs = rrsOk;
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = [phpCanceled, sync, &rrs, this](SPA::ClientSide::CAsyncServiceHandler *ash, bool canceled) {
			if (phpCanceled.isCallable()) {
				phpCanceled(canceled);
			}
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_aq->m_mPhp);
				rrs = canceled ? rrsCanceled : rrsClosed;
				this->m_aq->m_cvPhp.notify_all();
			}
		};
		if (sync) {
			std::unique_lock<std::mutex> lk(m_aq->m_mPhp);
			if (!m_aq->Enqueue(key.c_str(), (unsigned short)idMsg, pBuffer, bytes, discarded)) {
				Unlock();
				throw Php::Exception(PHP_SOCKET_CLOSED);
			}
			Unlock();
			auto status = m_aq->m_cvPhp.wait_for(lk, std::chrono::milliseconds(timeout));
			if (status == std::cv_status::timeout) {
				rrs = rrsTimeout;
			}
			switch (rrs) {
			case rrsServerException:
				throw Php::Exception(PHP_SERVER_EXCEPTION);
			case rrsCanceled:
				throw Php::Exception(PHP_REQUEST_CANCELED);
			case rrsClosed:
				throw Php::Exception(PHP_SOCKET_CLOSED);
			case rrsTimeout:
				throw Php::Exception(PHP_REQUEST_TIMEOUT);
			default:
				break;
			}
			return *pF;
		}
		return m_aq->Enqueue(key.c_str(), (unsigned short)idMsg, pBuffer, bytes, discarded) ? rrsOk : rrsClosed;
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
		SPA::ClientSide::CAsyncQueue::BatchMessage((unsigned short)idMsg, pBuffer, bytes, *m_pBuff->GetBuffer());
	}

	void CPhpQueue::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpQueue> handler(PHP_QUEUE_HANDLER);
		Register(handler);
		handler.method("Enqueue", &CPhpQueue::Enqueue, {
			Php::ByVal(PHP_QUEUE_KEY, Php::Type::String),
			Php::ByVal("idMessage", Php::Type::Numeric),
			Php::ByVal(PHP_SENDREQUEST_BUFF, Php::Type::Null),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("CloseQueue", &CPhpQueue::CloseQueue, {
			Php::ByVal(PHP_QUEUE_KEY, Php::Type::String),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("GetKeys", &CPhpQueue::GetKeys, {
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("StartQueueTrans", &CPhpQueue::StartQueueTrans, {
			Php::ByVal(PHP_QUEUE_KEY, Php::Type::String),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("StartTrans", &CPhpQueue::StartQueueTrans, {
			Php::ByVal(PHP_QUEUE_KEY, Php::Type::String),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("EndQueueTrans", &CPhpQueue::EndQueueTrans, {
			Php::ByVal("rollback", Php::Type::Bool),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("EndTrans", &CPhpQueue::EndQueueTrans, {
			Php::ByVal("rollback", Php::Type::Bool),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("FlushQueue", &CPhpQueue::FlushQueue, {
			Php::ByVal(PHP_QUEUE_KEY, Php::Type::String),
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
