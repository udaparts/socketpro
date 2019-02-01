
#include "phpbuffer.h"
#include "phpsocket.h"

#ifndef PHP_SPA_CLIENT_BASE_HANDLER_H
#define PHP_SPA_CLIENT_BASE_HANDLER_H

namespace PA {
	
	template <typename T>
	class CPhpBaseHandler : public Php::Base
	{
	protected:
		CPhpBaseHandler(bool locked, SPA::ClientSide::CAsyncServiceHandler *h, unsigned int poolId) : m_locked(locked), m_h(h), m_PoolId(poolId){
			assert(m_h);
		}
		CPhpBaseHandler(const CPhpBaseHandler& h) = delete;
		virtual ~CPhpBaseHandler() {
			if (m_locked) {
				SPA::ClientSide::ClientCoreLoader.UnlockASocket(m_PoolId, m_h->GetAttachedClientSocket()->GetHandle());
			}
		}
	
	protected:
		static void RegInto(Php::Class<T> &h, Php::Namespace &cs) {
			h.method(PHP_CONSTRUCT, &T::__construct, Php::Private);
			h.method(PHP_SENDREQUEST, &T::SendRequest, {
				Php::ByVal(PHP_SENDREQUEST_REQID, Php::Type::Numeric),
				Php::ByVal(PHP_SENDREQUEST_BUFF, (SPA_NS + PHP_BUFFER).c_str(), true, false),
				Php::ByVal(PHP_SENDREQUEST_RH, Php::Type::Null, false), //sync boolean, timeout value or callback for returning result
				Php::ByVal(PHP_SENDREQUEST_CH, Php::Type::Callable, false),
				Php::ByVal(PHP_SENDREQUEST_EX, Php::Type::Callable, false)
			});
			h.method("WaitAll", &T::WaitAll, {
				Php::ByVal("timeout", Php::Type::Numeric, false)
			});
			h.method("StartBatching", &T::StartBatching);
			h.method("AbortBatching", &T::AbortBatching);
			h.method("CommitBatching", &T::CommitBatching);
			h.method("Unlock", &T::Unlock);
		}

	public:
		CPhpBaseHandler& operator=(const CPhpBaseHandler& h) = delete;
		inline bool IsLocked() { 
			return m_locked;
		}

		Php::Value Unlock() {
			if (m_locked) {
				SPA::ClientSide::ClientCoreLoader.UnlockASocket(m_PoolId, m_h->GetAttachedClientSocket()->GetHandle());
				m_locked = false;
			}
			return true;
		}

		Php::Value SendRequest(Php::Parameters &params) {
			int64_t id = params[0].numericValue();
			if (id <= SPA::sidReserved || id > 0xffff) {
				throw Php::Exception("Bad request id");
			}
			unsigned short reqId = (unsigned short)id;
			SPA::ClientSide::ResultHandler rh;
			SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded;
			SPA::ClientSide::CAsyncServiceHandler::DServerException se;
			Php::Value phpRh, phpCanceled, phpEx;

			std::shared_ptr<CPhpBuffer> buffer;
			unsigned int timeout = m_h->GetAttachedClientSocket()->GetRecvTimeout();
			bool sync = false;
			size_t args = params.size();
			if (args > 2) {
				phpRh = params[2];
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
				else if (!phpRh.isCallable()) {
					throw Php::Exception("A callback required for returning result");
				}
				if (sync) {
					buffer.reset(new CPhpBuffer);
				}
			}
			rh = [phpRh, sync, buffer, this](SPA::ClientSide::CAsyncResult & ar) {
				SPA::ClientSide::PAsyncServiceHandler ash = ar.AsyncServiceHandler;
				if (sync) {
					std::unique_lock<std::mutex> lk(this->m_h->m_mPhp);
					buffer->Swap(&ar.UQueue);
					this->m_h->m_cvPhp.notify_all();
				}
				else if (phpRh.isCallable()) {
					CPhpBuffer *p = new CPhpBuffer;
					p->Swap(&ar.UQueue);
					Php::Object q(PHP_BUFFER, p);
					phpRh(q, ar.RequestId);
				}
				else {
					ar.UQueue.SetSize(0);
				}
			};
			if (args > 4) {
				phpEx = params[4];
				if (!phpEx.isCallable()) {
					throw Php::Exception("A callback required for server exception");
				}
			}
			tagRequestReturnStatus rrs = rrsOk;
			se = [phpEx, sync, &rrs, this](SPA::ClientSide::CAsyncServiceHandler *ash, unsigned short reqId, const wchar_t *errMsg, const char *errWhere, unsigned int errCode) {
				if (phpEx.isCallable()) {
					phpEx(SPA::Utilities::ToUTF8(errMsg).c_str(), (int64_t)errCode, errWhere, reqId);
				}
				if (sync) {
					std::unique_lock<std::mutex> lk(this->m_h->m_mPhp);
					rrs = rrsServerException;
					this->m_h->m_cvPhp.notify_all();
				}
			};
			if (args > 3) {
				phpCanceled = params[3];
				if (!phpCanceled.isCallable()) {
					throw Php::Exception("A callback required for request aborting event");
				}
			}
			discarded = [phpCanceled, reqId, sync, this, &rrs](SPA::ClientSide::CAsyncServiceHandler *ash, bool canceled) {
				if (phpCanceled.isCallable()) {
					phpCanceled(canceled, reqId);
				}
				if (sync) {
					std::unique_lock<std::mutex> lk(this->m_h->m_mPhp);
					rrs = canceled ? rrsCanceled : rrsClosed;
					this->m_h->m_cvPhp.notify_all();
				}
			};
			unsigned int bytes = 0;
			const unsigned char *pBuffer = nullptr;
			Php::Value v;
			if (args > 1) {
				Php::Value &q = params[1];
				if (q.instanceOf(PHP_BUFFER)) {
					v = q.call("PopBytes");
					pBuffer = (const unsigned char*)v.rawValue();
					bytes = (unsigned int)v.length();
				}
				else if (!q.isNull()) {
					throw Php::Exception("An instance of CUQueue or null required for request sending data");
				}
			}
			if (sync) {
				if (!m_h->SendRequest(reqId, pBuffer, bytes, rh, discarded, se)) {
					throw Php::Exception(PHP_SOCKET_CLOSED);
				}
				if (m_locked) {
					//auto unlock
					SPA::ClientSide::ClientCoreLoader.UnlockASocket(m_PoolId, m_h->GetAttachedClientSocket()->GetHandle());
					m_locked = false;
				}
				std::unique_lock<std::mutex> lk(m_h->m_mPhp);
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
		void __construct(Php::Parameters &params) {
		}

		Php::Value WaitAll(Php::Parameters &params) {
			unsigned int timeout = (~0);
			if (params.size()) {
				timeout = (unsigned int)params[0].numericValue();
			}
			return m_h->WaitAll();
		}

		Php::Value StartBatching() {
			return m_h->StartBatching();
		}

		Php::Value CommitBatching() {
			return m_h->CommitBatching();
		}

		Php::Value AbortBatching() {
			return m_h->AbortBatching();
		}

		Php::Value __get(const Php::Value &name) {
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

	private:
		bool m_locked;
		SPA::ClientSide::CAsyncServiceHandler *m_h;
		unsigned int m_PoolId;
	};
	
}

#endif
