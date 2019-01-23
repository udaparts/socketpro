
#ifndef SPA_PHP_SOCKET_H
#define SPA_PHP_SOCKET_H

namespace PA {

	class CPhpSocket : public Php::Base {
	public:
		CPhpSocket(CClientSocket *cs);
		~CPhpSocket();

	public:
		static void RegisterInto(Php::Namespace &cs);
		void __construct(Php::Parameters &params);

	private:
		CClientSocket *m_cs;
	};

} //namespace PA

#endif