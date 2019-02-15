#ifndef SPA_PHP_CERT_H
#define SPA_PHP_CERT_H

namespace PA {

	class CPhpCert : public Php::Base {
	public:
		CPhpCert(SPA::IUcert *cert);
		CPhpCert(const CPhpCert &cert) = delete;

	public:
		CPhpCert& operator=(const CPhpCert &cert) = delete;
		void __construct(Php::Parameters &params);
		static void RegisterInto(Php::Namespace &cs);
		Php::Value __get(const Php::Value &name);
		int __compare(const CPhpCert &cert) const;
		static std::string ToString(const unsigned char *buffer, unsigned int bytes);

	private:
		Php::Value Verify();

	private:
		SPA::IUcert *m_cert;
	};

} //namespace PA
#endif
