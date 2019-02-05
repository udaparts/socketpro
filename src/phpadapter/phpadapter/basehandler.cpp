#include "stdafx.h"
#include "basehandler.h"

namespace PA {
	CPhpBaseHandler::CPhpBaseHandler(bool locked, SPA::ClientSide::CAsyncServiceHandler *h, unsigned int poolId) : m_locked(locked), m_h(h), m_PoolId(poolId) {
		assert(m_h);
	}

	CPhpBaseHandler::~CPhpBaseHandler() {
		Unlock();
	}

	Php::Value CPhpBaseHandler::Unlock() {
		if (m_locked) {
			SPA::ClientSide::ClientCoreLoader.UnlockASocket(m_PoolId, m_h->GetAttachedClientSocket()->GetHandle());
			m_locked = false;
		}
		return true;
	}

	Php::Value CPhpBaseHandler::SendRequest(Php::Parameters &params) {
		int64_t id = params[0].numericValue();
		if (id <= SPA::idReservedTwo || id > 0xffff) {
			throw Php::Exception("Bad request id");
		}
		unsigned short reqId = (unsigned short)id;
		
		unsigned int timeout = m_h->GetAttachedClientSocket()->GetRecvTimeout();
		bool sync = false;
		Php::Value phpRh = params[2];
		if (phpRh.isNumeric()) {
			sync = true;
			unsigned int t = (unsigned int)phpRh.numericValue();
			if (t < timeout) {
				timeout = t;
			}
		}
		else if (phpRh.isBool()) {
			sync = phpRh.boolValue();
		}
		else if (phpRh.isNull()) {
		}
		else if (!phpRh.isCallable()) {
			throw Php::Exception("A callback required for returning result");
		}
		std::shared_ptr<CPhpBuffer> buffer;
		if (sync) {
			buffer.reset(new CPhpBuffer);
		}
		SPA::ClientSide::ResultHandler rh = [phpRh, sync, buffer, this](SPA::ClientSide::CAsyncResult & ar) {
			SPA::ClientSide::PAsyncServiceHandler ash = ar.AsyncServiceHandler;
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_h->m_mPhp);
				buffer->Swap(&ar.UQueue);
				this->m_h->m_cvPhp.notify_all();
			}
			else if (phpRh.isCallable()) {
				CPhpBuffer *p = new CPhpBuffer;
				p->Swap(&ar.UQueue);
				Php::Object q((SPA_NS + PHP_BUFFER).c_str(), p);
				phpRh(q, ar.RequestId);
			}
			else {
				ar.UQueue.SetSize(0);
			}
		};
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 3) {
			phpCanceled = params[3];
			if (phpCanceled.isNull()) {
			}
			else if (!phpCanceled.isCallable()) {
				throw Php::Exception("A callback required for request aborting event");
			}
		}
		tagRequestReturnStatus rrs = rrsOk;
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = [phpCanceled, reqId, sync, this, &rrs](SPA::ClientSide::CAsyncServiceHandler *ash, bool canceled) {
			if (phpCanceled.isCallable()) {
				phpCanceled(canceled, reqId);
			}
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_h->m_mPhp);
				rrs = canceled ? rrsCanceled : rrsClosed;
				this->m_h->m_cvPhp.notify_all();
			}
		};
		Php::Value phpEx;
		if (args > 4) {
			phpEx = params[4];
			if (phpEx.isNull()) {
			}
			else if (!phpEx.isCallable()) {
				throw Php::Exception("A callback required for server exception");
			}
		}
		SPA::ClientSide::CAsyncServiceHandler::DServerException se = [phpEx, sync, &rrs, this](SPA::ClientSide::CAsyncServiceHandler *ash, unsigned short reqId, const wchar_t *errMsg, const char *errWhere, unsigned int errCode) {
			if (phpEx.isCallable()) {
				phpEx(SPA::Utilities::ToUTF8(errMsg).c_str(), (int64_t)errCode, errWhere, reqId);
			}
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_h->m_mPhp);
				rrs = rrsServerException;
				this->m_h->m_cvPhp.notify_all();
			}
		};
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
			throw Php::Exception("An instance of CUQueue or null required for request sending data");
		}
		if (sync) {
			std::unique_lock<std::mutex> lk(m_h->m_mPhp);
			if (!m_h->SendRequest(reqId, pBuffer, bytes, rh, discarded, se)) {
				Unlock();
				throw Php::Exception(PHP_SOCKET_CLOSED);
			}
			Unlock();
			auto status = m_h->m_cvPhp.wait_for(lk, std::chrono::milliseconds(timeout));
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
			CPhpBuffer *p = new CPhpBuffer;
			p->Swap(buffer.get());
			return Php::Object((SPA_NS + PHP_BUFFER).c_str(), p);
		}
		return m_h->SendRequest(reqId, pBuffer, bytes, rh, discarded, se) ? rrsOk : rrsClosed;
	}
	void CPhpBaseHandler::__construct(Php::Parameters &params) {
	}

	Php::Value CPhpBaseHandler::WaitAll(Php::Parameters &params) {
		unsigned int timeout = (~0);
		if (params.size()) {
			timeout = (unsigned int)params[0].numericValue();
		}
		return m_h->WaitAll();
	}

	Php::Value CPhpBaseHandler::StartBatching() {
		return m_h->StartBatching();
	}

	Php::Value CPhpBaseHandler::CommitBatching() {
		return m_h->CommitBatching();
	}

	Php::Value CPhpBaseHandler::AbortBatching() {
		return m_h->AbortBatching();
	}

	int CPhpBaseHandler::__compare(const CPhpBaseHandler &pbh) const {
		if (!m_h || !pbh.m_h) {
			return 1;
		}
		return (m_h == pbh.m_h) ? 0 : 1;
	}

	Php::Value CPhpBaseHandler::__get(const Php::Value &name) {
		if (name == "Socket" || name == "ClientSocket" || name == "AttachedClientSocket") {
			return Php::Object((SPA_CS_NS + PHP_SOCKET).c_str(), new CPhpSocket(m_h->GetAttachedClientSocket()));
		}
		else if (name == "Locked") {
			return m_locked;
		}
		else if (name == "SvsId" || name == "SvsID") {
			return (int64_t)m_h->GetSvsID();
		}
		else if (name == "Batching") {
			return m_h->IsBatching();
		}
		else if (name == "RouteeRequest") {
			return m_h->IsRouteeRequest();
		}
		else if (name == "DequeuedResult") {
			return m_h->IsDequeuedResult();
		}
		else if (name == "DequeuedMessageAborted") {
			return m_h->IsDequeuedMessageAborted();
		}
		return Php::Base::__get(name);
	}

} //namespace PA