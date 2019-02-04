#include "stdafx.h"
#include "phpqueue.h"

namespace PA {

	CPhpQueue::CPhpQueue(unsigned int poolId, CAsyncQueue *aq, bool locked)
		: CPhpBaseHandler(locked, aq, poolId), m_aq(aq) {
	}

	Php::Value CPhpQueue::__get(const Php::Value &name) {
		if (name == "AutoNotified") {
			return m_aq->GetEnqueueNotified();
		}
		else if (name == "DequeueBatchSize") {
			return (int64_t)m_aq->GetDequeueBatchSize();
		}
		else {
			CPhpBaseHandler::__get(name);
		}
	}

	Php::Value CPhpQueue::CloseQueue(Php::Parameters &params) {
		std::string key = params[0].stringValue();
		Trim(key);
		if (key.size()) {
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
			*pErrCode = 0;
		}
		SPA::ClientSide::CAsyncQueue::DClose c = [phpC, sync, pErrCode, this](SPA::ClientSide::CAsyncQueue *aq, int errCode) {
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_aq->m_mPhp);
				*pErrCode = errCode;
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
				throw Php::Exception("A callback required for CloseKey aborting event");
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
			if (!m_aq->CloseQueue(key.c_str(), c, permanent, discarded)) {
				throw Php::Exception(PHP_SOCKET_CLOSED);
			}
			Unlock();
			std::unique_lock<std::mutex> lk(m_aq->m_mPhp);
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

	void CPhpQueue::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpQueue> handler(PHP_QUEUE_HANDLER);
		Register(handler);
		handler.method("CloseQueue", &CPhpQueue::CloseQueue, {
			Php::ByVal("key", Php::Type::String),
			Php::ByVal("sync", Php::Type::Null)
		});
		cs.add(handler);
	}
}
