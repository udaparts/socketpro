#include "stdafx.h"
#include "phpcert.h"

namespace PA {

	CPhpCert::CPhpCert(SPA::IUcert *cert) : m_cert(cert) {
		assert(cert);
	}

	void CPhpCert::__construct(Php::Parameters &params) {
	}

	int CPhpCert::__compare(const CPhpCert &cert) const {
		if (!m_cert || !cert.m_cert) {
			return 1;
		}
		return (m_cert == cert.m_cert) ? 0 : 1;
	}

	Php::Value CPhpCert::Verify() {
		int errCode = 0;
		std::string s = m_cert->Verify(&errCode);
		Php::Array v;
		v[0] = errCode;
		v[1] = s;
 		return v;
	}

	Php::Value CPhpCert::__get(const Php::Value &name) {
		if (name == "Issuer") {
			return m_cert->Issuer;
		}
		else if (name == "Subject") {
			return m_cert->Subject;
		}
		else if (name == "NotBefore") {
			return m_cert->NotBefore;
		}
		else if (name == "NotAfter") {
			return m_cert->NotAfter;
		}
		else if (name == "Validity") {
			return m_cert->Validity;
		}
		else if (name == "SigAlg") {
			return m_cert->SigAlg;
		}
		else if (name == "CertPem") {
			return m_cert->CertPem;
		}
		else if (name == "SessionInfo") {
			return m_cert->SessionInfo;
		}
		else if (name == "PK" || name == "PublicKey") {
			std::string str(m_cert->PublicKey, m_cert->PublicKey + m_cert->PKSize);
			return str;
		}
		else if (name == "Alg" || name == "Algorithm") {
			std::string str(m_cert->Algorithm, m_cert->Algorithm + m_cert->AlgSize);
			return str;
		}
		else if (name == "SN" || name == "SerialNumber") {
			std::string str(m_cert->SerialNumber, m_cert->SerialNumber + m_cert->SNSize);
			return str;
		}
		return Php::Base::__get(name);
	}
	
	void CPhpCert::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpCert> cert(PHP_CERT);
		cert.method(PHP_CONSTRUCT, &CPhpCert::__construct, Php::Private);
		cert.method("Verify", &CPhpCert::Verify);
		cs.add(cert);
	}
}