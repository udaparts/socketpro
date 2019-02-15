#include "stdafx.h"
#include "phpcert.h"

namespace PA {

	CPhpCert::CPhpCert(SPA::IUcert *cert) : m_cert(cert) {
		assert(cert);
	}

	void CPhpCert::__construct(Php::Parameters &params) {
	}

	void CPhpCert::__destruct() {
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
		v.set(PHP_ERR_CODE, errCode);
		v.set(PHP_ERR_MSG, s);
 		return v;
	}

	std::string CPhpCert::ToString(const unsigned char *buffer, unsigned int bytes) {
		std::string s;
		char str[8] = { 0 };
		if (!buffer) bytes = 0;
		for (unsigned int n = 0; n < bytes; ++n) {
#ifdef WIN32_64
			sprintf_s(str, "%02x", buffer[n]);
#else
			sprintf(str, "%02x", buffer[n]);
#endif
			s += str;
		}
		return s;
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
			return ToString(m_cert->PublicKey, m_cert->PKSize);
		}
		else if (name == "Alg" || name == "Algorithm") {
			return ToString(m_cert->Algorithm, m_cert->AlgSize);
		}
		else if (name == "SN" || name == "SerialNumber") {
			return ToString(m_cert->SerialNumber, m_cert->SNSize);
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