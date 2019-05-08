
#include "ucertimpl.h"
#ifdef USE_SPIRIT_CLSSICAL_FOR_MULTIPART
#include "../ServerCoreUnix/server.h"
#else
#include "../../clientcore/clientthread.h"
#endif

const char* CUCertImpl::m_empty = "";

CUCertImpl::CUCertImpl(SSL *pSsl)
: m_pSsl(pSsl), m_cert(::SSL_get_peer_certificate(m_pSsl)) {
    Set();
    ReleaseCert();
}

CUCertImpl::CUCertImpl(X509 *cert)
: m_pSsl(nullptr), m_cert(cert) {
    Set();
    m_cert = nullptr;
}

CUCertImpl::~CUCertImpl() {
    ReleaseCert();
}

void CUCertImpl::ReleaseCert() {
    if (m_cert) {
        ::X509_free(m_cert);
        m_cert = nullptr;
    }
}

void CUCertImpl::Set() {
    Validity = GetValidity();

    m_Issuer = GetIssuer();
    Issuer = m_Issuer.c_str();

    m_Subject = GetSubject();
    Subject = m_Subject.c_str();

    m_NotBefore = GetNotBefore();
    NotBefore = m_NotBefore.c_str();

    m_NotAfter = GetNotAfter();
    NotAfter = m_NotAfter.c_str();

    m_CertPem = GetCertPem();
    CertPem = m_CertPem.c_str();

    m_PublicKey = GetPublicKey();
    PublicKey = &m_PublicKey.front();
    PKSize = (unsigned int) m_PublicKey.size();

    m_SerialNumber = GetSerialNumber();
    SerialNumber = &m_SerialNumber.front();
    SNSize = (unsigned int) m_SerialNumber.size();

    m_Algorithm = GetAlgorithm();
    Algorithm = &m_Algorithm.front();
    AlgSize = (unsigned int) m_Algorithm.size();

    SigAlg = GetSigAlg();

    m_SessionInfo = GetSessionInfo();
    SessionInfo = m_SessionInfo.c_str();
}

void CUCertImpl::asn1time_to_string(const ASN1_TIME *tm, char *buf) {
    char *expires = nullptr;
    char *pos = nullptr;
    BIO *bio = nullptr;

    strcpy(buf, "[invalid date]");
    bio = ::BIO_new(::BIO_s_mem());
    if (bio) {
        ::ASN1_TIME_print(bio, tm);
        //BIO_get_mem_data(bio,&expires);
        //#define BIO_get_mem_data(b,pp) BIO_ctrl(b,BIO_CTRL_INFO,0,(char *)pp)
        ::BIO_ctrl(bio, BIO_CTRL_INFO, 0, &expires);
        if (expires) {
            ::strcpy(buf, expires);
            pos = ::strstr(buf, "GMT");
            if (pos) {
                pos += 3;
                *pos = 0;
            }
        } else
            strcpy(buf, "Invalid Time");
        ::BIO_free(bio);
    }
}

std::string CUCertImpl::GetIssuer() const {
    if (m_cert == nullptr)
        return "";
    return ::X509_NAME_oneline(::X509_get_issuer_name(m_cert), 0, 0);
}

std::string CUCertImpl::GetSubject() const {
    if (m_cert == nullptr)
        return "";
    return ::X509_NAME_oneline(::X509_get_subject_name(m_cert), 0, 0);
}

bool CUCertImpl::SetVerifyLocation(const char *caFile) {
    int res = -1;
    if (!caFile)
        return false;
    std::string ca(caFile);
    if (ca.rfind(".pem") == ca.size() - 4) {
#ifdef USE_SPIRIT_CLSSICAL_FOR_MULTIPART
        if (g_pServer && g_pServer->m_pSslContext)
            res = ::SSL_CTX_load_verify_locations(g_pServer->m_pSslContext->native_handle(), caFile, nullptr);
#else
        res = ::SSL_CTX_load_verify_locations(CClientThread::m_sslContext.native_handle(), caFile, nullptr);
#endif
    } else {
#ifdef USE_SPIRIT_CLSSICAL_FOR_MULTIPART
        if (g_pServer && g_pServer->m_pSslContext)
            res = ::SSL_CTX_load_verify_locations(g_pServer->m_pSslContext->native_handle(), nullptr, caFile);
#else
        res = ::SSL_CTX_load_verify_locations(CClientThread::m_sslContext.native_handle(), nullptr, caFile);
#endif
    }
    return (res > 0);
}

const char* CUCertImpl::Verify(int *errCode) const {
    if (m_pSsl == nullptr) {
        if (errCode)
            *errCode = -1;
        return "";
    }
    int res = ::SSL_get_verify_result(m_pSsl);
    if (errCode)
        *errCode = res;
    return ::X509_verify_cert_error_string(res);
}

std::string CUCertImpl::GetCertPem() const {
    if (m_cert == nullptr)
        return "";
    SPA::CScopeUQueue su;
    BIO *pMem = ::BIO_new(::BIO_s_mem());
    if (pMem != nullptr) {
        BUF_MEM *bptr = nullptr;
        int nRtn = ::PEM_write_bio_X509(pMem, m_cert);
        nRtn = ::BIO_get_mem_ptr(pMem, &bptr);
        if (nRtn > 0 && bptr != nullptr && bptr->data && bptr->length > 0)
            su->Push(bptr->data, (unsigned int) bptr->length);
        ::BIO_free_all(pMem);
    }
    su->SetNull();
    return (const char*) su->GetBuffer();
}

std::vector<unsigned char> CUCertImpl::GetPublicKey() const {
    std::vector<unsigned char> vPublic;
    if (m_cert == nullptr)
        return vPublic;
    EVP_PKEY *public_key = ::X509_get0_pubkey(m_cert);
    if (public_key) {
        SPA::CScopeUQueue sb;
        unsigned char *buffer = (unsigned char *) sb->GetBuffer();
        int len = ::i2d_PublicKey(public_key, &buffer);
        if (len > 0) {
            assert((unsigned int) len <= sb->GetMaxSize());
            vPublic.assign(buffer, buffer + len);
        }
    }
    return vPublic;
}

std::string CUCertImpl::GetNotAfter() const {
    char strNotAfter[256] = {0};
    if (m_cert == nullptr)
        return strNotAfter;
    const ASN1_TIME *pNotAfter = X509_get0_notAfter(m_cert);
    asn1time_to_string(pNotAfter, strNotAfter);
    return strNotAfter;
}

std::string CUCertImpl::GetNotBefore() const {
    char strNotBefore[256] = {0};
    if (m_cert == nullptr)
        return strNotBefore;
    const ASN1_TIME *pNotBefore = X509_get0_notBefore(m_cert);
    asn1time_to_string(pNotBefore, strNotBefore);
    return strNotBefore;
}

bool CUCertImpl::GetValidity() const {
    bool bOK = false;
    if (m_cert == nullptr) {
        return bOK;
    }
    const ASN1_TIME *pNotAfter = X509_get0_notAfter(m_cert);
    const ASN1_TIME *pNotBefore = X509_get0_notBefore(m_cert);
    bOK = (::X509_cmp_current_time(pNotAfter) >= 0);
    bOK = (bOK && (::X509_cmp_current_time(pNotBefore) <= 0));
    return bOK;
}

const char* CUCertImpl::GetSigAlg() const {
    if (m_cert == nullptr) {
        return m_empty;
    }
    int nTypeTwo = X509_get_signature_nid(m_cert);
    const char *strSN = ::OBJ_nid2sn(nTypeTwo);
    return strSN;
}

std::vector<unsigned char> CUCertImpl::GetSerialNumber() const {
    std::vector<unsigned char> sn;
    if (m_cert == nullptr)
        return sn;
    const ASN1_INTEGER *ai = X509_get0_serialNumber(m_cert);
    if (!ai)
        return sn;
    BIGNUM *pbn = ASN1_INTEGER_to_BN(ai, nullptr);
    SPA::CScopeUQueue sb;
    unsigned char *buffer = (unsigned char*) sb->GetBuffer();
    int bytes = BN_bn2bin((const BIGNUM *) pbn, buffer);
    if (bytes > 0) {
        sn.assign(buffer, buffer + bytes);
    }
    return sn;
}

std::vector<unsigned char> CUCertImpl::GetAlgorithm() const {
    std::vector<unsigned char> alg;
    if (m_cert == nullptr)
        return alg;
    X509_PUBKEY *public_key = X509_get_X509_PUBKEY(m_cert);
    if (!public_key)
        return alg;
    X509_ALGOR *algor = nullptr;
    X509_PUBKEY_get0_param(nullptr, nullptr, nullptr, &algor, public_key);
    if (!algor || !algor->algorithm)
        return alg;
    unsigned char buff[1024] = {0};
    int len = OBJ_obj2txt((char*) buff, sizeof (buff), algor->algorithm, 0);
    if (len > 0)
        alg.assign(buff, buff + len);
    return alg;
}

std::string CUCertImpl::GetSessionInfo() const {
    char enc[4096] = {0};
    do {
        if (m_cert == nullptr)
            break;
        EVP_PKEY *pkey = ::X509_get0_pubkey(m_cert);
        if (!pkey)
            break;
        RSA *rsa = EVP_PKEY_get0_RSA(pkey);
        DSA *dsa = EVP_PKEY_get0_DSA(pkey);
        DH *dh = EVP_PKEY_get0_DH(pkey);
        EC_KEY *ec = EVP_PKEY_get0_EC_KEY(pkey);
        if (rsa) {
            int bits = EVP_PKEY_bits(pkey);
            if (ec) {
                sprintf(enc, "%d bit ECDHE RSA", bits);
            } else if (dh) {
                sprintf(enc, "%d bit DHE RSA", bits);
            } else {
                sprintf(enc, "%d bit RSA", bits);
            }
        } else if (dsa) {
            int bits = EVP_PKEY_bits(pkey);
            if (ec) {
                sprintf(enc, "%d bit ECDHE DSA", bits);
            } else if (dh) {
                sprintf(enc, "%d bit DHE DSA", bits);
            } else {
                sprintf(enc, "%d bit DSA", bits);
            }
        }
    } while (false);
    /* The SSL API does not allow us to look at temporary RSA/DH keys,
     * otherwise we should print their lengths too */
    SPA::CScopeUQueue su;
    if (m_pSsl) {
        const SSL_CIPHER *ciph = ::SSL_get_current_cipher(m_pSsl);
        su->Push("SSL version = ");
        su->Push(::SSL_get_version(m_pSsl));
        su->Push(", Cipher = ");
        su->Push((const char*) ::SSL_CIPHER_get_version(ciph));
        su->Push(", ");
        su->Push(::SSL_CIPHER_get_name(ciph));
        su->Push(", ");
    }
    su->Push(enc);
    su->SetNull();
    return (const char*) su->GetBuffer();
}
