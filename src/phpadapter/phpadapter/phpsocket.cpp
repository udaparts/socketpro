#include "stdafx.h"
#include "phpsocket.h"
#include "phpcert.h"

namespace PA {
	CPhpSocket::CPhpSocket(CClientSocket *cs) : m_cs(cs) {
	}

	CPhpSocket::~CPhpSocket() {
	}

	void CPhpSocket::__construct(Php::Parameters &params) {
	}

	Php::Value CPhpSocket::__get(const Php::Value &name) {
		if (name == "Cert" || name == "UCert") {
			SPA::IUcert *cert = m_cs->GetUCert();
			if (!cert) {
				return nullptr;
			}
			return Php::Object((SPA_NS + PHP_CERT).c_str(), new CPhpCert(cert));
		}
		return Php::Base::__get(name);
	}

	void CPhpSocket::__set(const Php::Value &name, const Php::Value &value) {

	}

	void CPhpSocket::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpSocket> socket(PHP_SOCKET);
		socket.method(PHP_CONSTRUCT, &CPhpSocket::__construct, Php::Private);

		cs.add(socket);
	}

} //namespace PA