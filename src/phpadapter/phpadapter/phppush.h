#ifndef PHP_SPA_CLIENTSIDE_CLIENTSOCKET_PUSH_H
#define PHP_SPA_CLIENTSIDE_CLIENTSOCKET_PUSH_H

namespace PA {

	typedef SPA::ClientSide::CClientSocket::CPushImpl CPush;

	class CPhpPush : public Php::Base {
	public:
		CPhpPush(CPush &p);
		CPhpPush(const CPhpPush &p) = delete;

	public:
		CPhpPush& operator=(const CPhpPush &p) = delete;
		static void RegisterInto(Php::Namespace &cs);

	private:
		void __construct(Php::Parameters &params);

	private:
		CPush &Push;
	};

} //namespace PA

#endif
