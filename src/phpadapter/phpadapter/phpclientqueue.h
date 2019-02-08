
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

	private:
		void __construct(Php::Parameters &params);

	private:
		CClientQueue &m_cq;
	};

}

#endif
