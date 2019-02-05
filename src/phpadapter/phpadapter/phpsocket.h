
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
		Php::Value __get(const Php::Value &name);
		void __set(const Php::Value &name, const Php::Value &value);
		int __compare(const CPhpSocket &socket) const;

	private:
		CClientSocket *m_cs;
	};

} //namespace PA

#endif