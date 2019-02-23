#ifndef SPA_PHP_CERT_H
#define SPA_PHP_CERT_H

namespace PA {

    class CPhpCert : public Php::Base {
    public:
        CPhpCert(SPA::IUcert *cert);
        CPhpCert(const CPhpCert &cert) = delete;

    public:
        CPhpCert& operator=(const CPhpCert &cert) = delete;
        int __compare(const CPhpCert &cert) const;
        void __destruct();
        static void RegisterInto(Php::Namespace &cs);
        Php::Value __get(const Php::Value &name);

        static std::string ToString(const unsigned char *buffer, unsigned int bytes);

    private:
        void __construct(Php::Parameters &params);
        Php::Value Verify();

    private:
        SPA::IUcert *m_cert;
    };

} //namespace PA
#endif
