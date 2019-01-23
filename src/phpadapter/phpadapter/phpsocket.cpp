#include "stdafx.h"
#include "phpsocket.h"

namespace PA {


	CPhpSocket::CPhpSocket(CClientSocket *cs) : m_cs(cs) {
	}


	CPhpSocket::~CPhpSocket() {
	}

	void CPhpSocket::__construct(Php::Parameters &params) {

	}

	void CPhpSocket::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpSocket> socket(PHP_SOCKET);
		socket.method(PHP_CONSTRUCT, &CPhpSocket::__construct, Php::Private);

		cs.add(socket);
	}

} //namespace PA