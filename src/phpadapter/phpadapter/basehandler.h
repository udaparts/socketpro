
#ifndef PHP_SPA_CLIENT_BASE_HANDLER_H
#define PHP_SPA_CLIENT_BASE_HANDLER_H

namespace PA {
	
	template <typename T>
	class CPhpBaseHandler
	{
	protected:
		CPhpBaseHandler(bool locked, SPA::ClientSide::CAsyncServiceHandler *h) : m_locked(locked), m_h(h) {
			assert(m_h);
		}
		CPhpBaseHandler(const CPhpBaseHandler& h) = delete;
		virtual ~CPhpBaseHandler() {}
	
	protected:
		static void RegInto(Php::Class<T> &h, Php::Namespace &cs) {
			h.method(PHP_CONSTRUCT, &T::__construct, Php::Private);
			h.method(PHP_SENDREQUEST, &T::SendRequest, {
				Php::ByVal(PHP_SENDREQUEST_REQID, Php::Type::Numeric),
				Php::ByVal(PHP_SENDREQUEST_BUFF, PHP_BUFFER, true, false),
				Php::ByVal(PHP_SENDREQUEST_RH, Php::Type::Null, false), //sync boolean or callback for returning result
				Php::ByVal(PHP_SENDREQUEST_CH, Php::Type::Callable, false), //callback for cancel or disconnection
				Php::ByVal(PHP_SENDREQUEST_EX, Php::Type::Callable, false)
			});
			h.method("WaitAll", &T::WaitAll, {
				Php::ByVal("timeout", Php::Type::Numeric, false)
			});
			h.method("StartBatching", &T::StartBatching);
			h.method("AbortBatching", &T::AbortBatching);
			h.method("CommitBatching", &T::CommitBatching);
		}

	public:
		CPhpBaseHandler& operator=(const CPhpBaseHandler& h) = delete;
		inline bool IsLocked() { 
			return m_locked;
		}
		virtual Php::Value SendRequest(Php::Parameters &params) {
			if (m_h) {
				return m_h->SendRequest(params);
			}
			return false;
		}
		virtual void __construct(Php::Parameters &params) {
		}

		virtual int __compare(const T &h) const = 0;

		virtual Php::Value WaitAll(Php::Parameters &params) {
			unsigned int timeout = (~0);
			if (params.size()) {
				timeout = (unsigned int)params[0].numericValue();
			}
			return m_h->WaitAll();
		}

		virtual Php::Value StartBatching() {
			return m_h->StartBatching();
		}

		virtual Php::Value CommitBatching() {
			return m_h->CommitBatching();
		}

		virtual Php::Value AbortBatching() {
			return m_h->AbortBatching();
		}

		Php::Value BaseGet(const Php::Value &name) {
			if (name == "Socket" || name == "ClientSocket" || name == "AttachedClientSocket") {
				return Php::Object((SPA_CS_NS + PHP_SOCKET).c_str(), new CPhpSocket(m_h->GetAttachedClientSocket()));
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
			return nullptr;
		}

	private:
		bool m_locked;
		SPA::ClientSide::CAsyncServiceHandler *m_h;
	};
	
}

#endif
