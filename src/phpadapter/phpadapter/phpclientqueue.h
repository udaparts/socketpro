
#ifndef PHP_SPA_CLIENTSIDE_CLIENTSOCKET_CLIENTQUEUE_H
#define PHP_SPA_CLIENTSIDE_CLIENTSOCKET_CLIENTQUEUE_H

namespace PA {

	typedef SPA::ClientSide::IClientQueue CClientQueue;

	class CPhpClientQueue : public Php::Base {
	public:
		CPhpClientQueue(CClientQueue &cq);
		CPhpClientQueue(const CPhpClientQueue &cq) = delete;

	public:
		CPhpClientQueue& operator=(const CPhpClientQueue &cq) = delete;
		static void RegisterInto(Php::Namespace &cs);
		Php::Value __get(const Php::Value &name);
		void __set(const Php::Value &name, const Php::Value &value);

	private:
		void __construct(Php::Parameters &params);
		Php::Value AbortJob();
		Php::Value EndJob();
		Php::Value StartJob();
		Php::Value RemoveByTTL();
		void Reset();
		Php::Value CancelQueuedMessages(Php::Parameters &params);

	private:
		CClientQueue &m_cq;
	};

}

#endif
