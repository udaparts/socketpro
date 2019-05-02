
#include "startup.h"

CStartup *g_pStartup = nullptr;

CStartup::CStartup(bool client)
: m_pCtx(nullptr),
m_hSSLEay(nullptr),
m_hLibeay(nullptr),
m_pMeth(nullptr),
m_pSSLLockHandle(nullptr),
m_bClient(client) {
    g_pStartup = this;
    LoadOpenSSL();
    StartOpenSSL();
}

CStartup::~CStartup() {
    //SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
    g_pStartup = nullptr;
    StopOpenSSL();
    UnloadOpenSSL();
}

void CStartup::UnloadOpenSSL() {
    if (m_hSSLEay) {
        ::FreeLibrary(m_hSSLEay);
        m_hSSLEay = nullptr;
    }
    if (m_hLibeay) {
        ::FreeLibrary(m_hLibeay);
        m_hLibeay = nullptr;
    }
    SSL_library_init = nullptr;
    SSLv23_method = nullptr;
    SSL_load_error_strings = nullptr;
    SSL_CTX_new = nullptr;
    SSL_CTX_free = nullptr;
    SSL_new = nullptr;
    SSL_free = nullptr;
    SSL_set_fd = nullptr;
    SSL_accept = nullptr;
    SSL_get_error = nullptr;
    SSL_write = nullptr;
    SSL_read = nullptr;
    SSL_shutdown = nullptr;
    SSL_state = nullptr;
    SSL_set_connect_state = nullptr;
    SSL_set_accept_state = nullptr;
    SSL_CTX_use_certificate_file = nullptr;
    SSL_CTX_use_PrivateKey_file = nullptr;
    SSL_CTX_check_private_key = nullptr;
    SSL_state_string_long = nullptr;
    SSL_do_handshake = nullptr;
    SSL_want = nullptr;
    SSL_pending = nullptr;
    SSL_connect = nullptr;
    SSL_peek = nullptr;
    TLSv1_method = nullptr;
    ERR_clear_error = nullptr;
    SSL_clear = nullptr;
    CRYPTO_free = nullptr;
    CRYPTO_malloc = nullptr;
    CRYPTO_set_locking_callback = nullptr;
    CRYPTO_num_locks = nullptr;
    SSL_set_read_ahead = nullptr;
    SSL_set_info_callback = nullptr;
    SSL_get_fd = nullptr;
    ERR_get_error = nullptr;
    ERR_error_string = nullptr;
    SSL_CTX_ctrl = nullptr;
    //	d2i_PKCS12_fp = nullptr;
    PKCS12_parse = nullptr;
    PKCS12_free = nullptr;
    SSL_use_certificate = nullptr;
    SSL_use_PrivateKey = nullptr;
    ERR_load_crypto_strings = nullptr;
    OPENSSL_add_all_algorithms_noconf = nullptr;
    BIO_free = nullptr;
    BIO_new_file = nullptr;
    d2i_PKCS12_bio = nullptr;
    X509_free = nullptr;
    EVP_PKEY_free = nullptr;

    BIO_new = nullptr;
    BIO_s_mem = nullptr;
    SSL_set_bio = nullptr;
    BIO_read = nullptr;
    BIO_write = nullptr;
    BIO_ctrl_pending = nullptr;
    SSL_get_rbio = nullptr;
    SSL_get_wbio = nullptr;
    SSL_CTX_use_certificate_chain_file = nullptr;
    SSL_CTX_set_default_passwd_cb = nullptr;
    CRYPTO_set_dynlock_create_callback = nullptr;
    CRYPTO_set_dynlock_lock_callback = nullptr;
    CRYPTO_set_dynlock_destroy_callback = nullptr;
    TLSv1_server_method = nullptr;
    TLSv1_client_method = nullptr;
    CRYPTO_set_id_callback = nullptr;
    ERR_load_BIO_strings = nullptr;
    SSL_ctrl = nullptr;
}

bool CStartup::LoadOpenSSL() {
#ifdef WIN32_64
    m_hSSLEay = ::LoadLibraryW(L"ssleay32.dll");
    m_hLibeay = ::LoadLibraryW(L"libeay32.dll");
#else
    m_hSSLEay = ::dlopen("libssl.so", RTLD_LAZY);
    m_hLibeay = ::dlopen("libcrypto.so", RTLD_LAZY);
#endif
    if (!m_hSSLEay || !m_hLibeay) {
        UnloadOpenSSL();
        return false;
    }

    SSL_get_rbio = (PSSL_get_rbio)::GetProcAddress(m_hSSLEay, "SSL_get_rbio");
    SSL_get_wbio = (PSSL_get_wbio)::GetProcAddress(m_hSSLEay, "SSL_get_wbio");
    SSL_set_bio = (PSSL_set_bio)::GetProcAddress(m_hSSLEay, "SSL_set_bio");
    BIO_new = (PBIO_new)::GetProcAddress(m_hLibeay, "BIO_new");
    BIO_s_mem = (PBIO_s_mem)::GetProcAddress(m_hLibeay, "BIO_s_mem");
    BIO_read = (PBIO_read)::GetProcAddress(m_hLibeay, "BIO_read");
    BIO_write = (PBIO_write)::GetProcAddress(m_hLibeay, "BIO_write");
    BIO_ctrl_pending = (PBIO_ctrl_pending)::GetProcAddress(m_hLibeay, "BIO_ctrl_pending");
    ERR_load_BIO_strings = (PERR_load_BIO_strings)::GetProcAddress(m_hLibeay, "ERR_load_BIO_strings");

    X509_free = (PX509_free)::GetProcAddress(m_hLibeay, "X509_free");
    EVP_PKEY_free = (PEVP_PKEY_free)::GetProcAddress(m_hLibeay, "EVP_PKEY_free");

    BIO_free = (PBIO_free)::GetProcAddress(m_hLibeay, "BIO_free");
    d2i_PKCS12_bio = (Pd2i_PKCS12_bio)::GetProcAddress(m_hLibeay, "d2i_PKCS12_bio");
    BIO_new_file = (PBIO_new_file)::GetProcAddress(m_hLibeay, "BIO_new_file");

    ERR_load_crypto_strings = (PERR_load_crypto_strings)::GetProcAddress(m_hLibeay, "ERR_load_crypto_strings");
    OPENSSL_add_all_algorithms_noconf = (POPENSSL_add_all_algorithms_noconf)::GetProcAddress(m_hLibeay, "OPENSSL_add_all_algorithms_noconf");

    SSL_use_certificate = (PSSL_use_certificate)::GetProcAddress(m_hSSLEay, "SSL_use_certificate");
    SSL_use_PrivateKey = (PSSL_use_PrivateKey)::GetProcAddress(m_hSSLEay, "SSL_use_PrivateKey");

    PKCS12_free = (PPKCS12_free)::GetProcAddress(m_hLibeay, "PKCS12_free");
    PKCS12_parse = (PPKCS12_parse)::GetProcAddress(m_hLibeay, "PKCS12_parse");
    //debug
    ERR_error_string = (PERR_error_string)::GetProcAddress(m_hLibeay, "ERR_error_string");
    ERR_get_error = (PERR_get_error)::GetProcAddress(m_hLibeay, "ERR_get_error");
    ERR_clear_error = (PERR_clear_error)::GetProcAddress(m_hLibeay, "ERR_clear_error");
    TLSv1_method = (PTLSv1_method)::GetProcAddress(m_hSSLEay, "TLSv1_method");
    TLSv1_server_method = (PTLSv1_server_method)::GetProcAddress(m_hSSLEay, "TLSv1_server_method");
    TLSv1_client_method = (PTLSv1_client_method)::GetProcAddress(m_hSSLEay, "TLSv1_client_method");
    SSL_set_info_callback = (PSSL_set_info_callback)::GetProcAddress(m_hSSLEay, "SSL_set_info_callback");
    SSL_get_fd = (PSSL_get_fd)::GetProcAddress(m_hSSLEay, "SSL_get_fd");
    SSL_clear = (PSSL_clear)::GetProcAddress(m_hSSLEay, "SSL_clear");
    SSL_peek = (PSSL_peek)::GetProcAddress(m_hSSLEay, "SSL_peek");
    SSL_connect = (PSSL_connect)::GetProcAddress(m_hSSLEay, "SSL_connect");
    SSL_pending = (PSSL_pending)::GetProcAddress(m_hSSLEay, "SSL_pending");
    SSL_accept = (PSSL_accept)::GetProcAddress(m_hSSLEay, "SSL_accept");
    SSL_want = (PSSL_want)::GetProcAddress(m_hSSLEay, "SSL_want");
    SSL_CTX_set_default_passwd_cb = (PSSL_CTX_set_default_passwd_cb)::GetProcAddress(m_hSSLEay, "SSL_CTX_set_default_passwd_cb");
    SSL_CTX_use_certificate_chain_file = (PSSL_CTX_use_certificate_chain_file)::GetProcAddress(m_hSSLEay, "SSL_CTX_use_certificate_chain_file");
    SSL_CTX_ctrl = (PSSL_CTX_ctrl)::GetProcAddress(m_hSSLEay, "SSL_CTX_ctrl");
    SSL_ctrl = (PSSL_ctrl)::GetProcAddress(m_hSSLEay, "SSL_ctrl");
    SSL_CTX_check_private_key = (PSSL_CTX_check_private_key)::GetProcAddress(m_hSSLEay, "SSL_CTX_check_private_key");
    SSL_CTX_free = (PSSL_CTX_free)::GetProcAddress(m_hSSLEay, "SSL_CTX_free");
    SSL_CTX_new = (PSSL_CTX_new)::GetProcAddress(m_hSSLEay, "SSL_CTX_new");
    SSL_CTX_use_certificate_file = (PSSL_CTX_use_certificate_file)::GetProcAddress(m_hSSLEay, "SSL_CTX_use_certificate_file");
    SSL_CTX_use_PrivateKey_file = (PSSL_CTX_use_PrivateKey_file)::GetProcAddress(m_hSSLEay, "SSL_CTX_use_PrivateKey_file");
    SSL_free = (PSSL_free)::GetProcAddress(m_hSSLEay, "SSL_free");
    SSL_do_handshake = (PSSL_do_handshake)::GetProcAddress(m_hSSLEay, "SSL_do_handshake");
    SSL_get_error = (PSSL_get_error)::GetProcAddress(m_hSSLEay, "SSL_get_error");
    SSL_library_init = (PSSL_library_init)::GetProcAddress(m_hSSLEay, "SSL_library_init");
    SSL_load_error_strings = (PSSL_load_error_strings)::GetProcAddress(m_hSSLEay, "SSL_load_error_strings");
    SSL_new = (PSSL_new)::GetProcAddress(m_hSSLEay, "SSL_new");
    SSL_read = (PSSL_read)::GetProcAddress(m_hSSLEay, "SSL_read");
    SSL_set_accept_state = (PSSL_set_accept_state)::GetProcAddress(m_hSSLEay, "SSL_set_accept_state");
    SSL_set_connect_state = (PSSL_set_connect_state)::GetProcAddress(m_hSSLEay, "SSL_set_connect_state");
    SSL_set_fd = (PSSL_set_fd)::GetProcAddress(m_hSSLEay, "SSL_set_fd");
    SSL_shutdown = (PSSL_shutdown)::GetProcAddress(m_hSSLEay, "SSL_shutdown");
    SSL_state = (PSSL_state)::GetProcAddress(m_hSSLEay, "SSL_state");
    SSL_state_string_long = (PSSL_state_string_long)::GetProcAddress(m_hSSLEay, "SSL_state_string_long");
    SSL_write = (PSSL_write)::GetProcAddress(m_hSSLEay, "SSL_write");
    SSLv23_method = (PSSLv23_method)::GetProcAddress(m_hSSLEay, "SSLv23_method");
    SSL_set_read_ahead = (PSSL_set_read_ahead)::GetProcAddress(m_hSSLEay, "SSL_set_read_ahead");
    CRYPTO_free = (PCRYPTO_free)::GetProcAddress(m_hLibeay, "CRYPTO_free");
    CRYPTO_malloc = (PCRYPTO_malloc)::GetProcAddress(m_hLibeay, "CRYPTO_malloc");
    CRYPTO_set_locking_callback = (PCRYPTO_set_locking_callback)::GetProcAddress(m_hLibeay, "CRYPTO_set_locking_callback");

    CRYPTO_set_dynlock_create_callback = (PCRYPTO_set_dynlock_create_callback)::GetProcAddress(m_hLibeay, "CRYPTO_set_dynlock_create_callback");
    CRYPTO_set_dynlock_lock_callback = (PCRYPTO_set_dynlock_lock_callback)::GetProcAddress(m_hLibeay, "CRYPTO_set_dynlock_lock_callback");
    CRYPTO_set_dynlock_destroy_callback = (PCRYPTO_set_dynlock_destroy_callback)::GetProcAddress(m_hLibeay, "CRYPTO_set_dynlock_destroy_callback");
    CRYPTO_set_id_callback = (PCRYPTO_set_id_callback)::GetProcAddress(m_hLibeay, "CRYPTO_set_id_callback");

    CRYPTO_num_locks = (PCRYPTO_num_locks)::GetProcAddress(m_hLibeay, "CRYPTO_num_locks");

    if (SSL_library_init && SSL_load_error_strings && SSL_CTX_new && d2i_PKCS12_bio && ERR_load_crypto_strings
            && SSL_CTX_free && SSL_new && SSL_free && SSL_set_fd && PKCS12_parse && BIO_free && X509_free
            && SSL_write && SSL_read && SSL_shutdown && SSL_state && SSL_set_connect_state && CRYPTO_malloc && SSL_CTX_ctrl
            && SSL_set_accept_state && SSL_CTX_use_certificate_file && SSL_CTX_use_PrivateKey_file && ERR_get_error
            && SSL_CTX_check_private_key && SSL_set_info_callback && PKCS12_free && SSL_use_PrivateKey
            && SSL_set_read_ahead && ERR_clear_error && CRYPTO_num_locks && ERR_error_string
            && CRYPTO_free && SSL_get_fd && SSL_accept && SSL_get_error && CRYPTO_set_locking_callback
            && SSL_state_string_long && SSL_do_handshake && SSL_want && SSL_use_certificate && EVP_PKEY_free
            && SSL_pending && SSL_connect && SSL_peek && BIO_new_file && OPENSSL_add_all_algorithms_noconf
            && BIO_read && BIO_write && BIO_ctrl_pending && SSL_set_bio && SSL_get_rbio && SSL_get_wbio && BIO_new && BIO_s_mem
            && SSL_CTX_use_certificate_chain_file && SSL_CTX_set_default_passwd_cb && CRYPTO_set_dynlock_destroy_callback
            && CRYPTO_set_dynlock_create_callback && CRYPTO_set_dynlock_lock_callback && TLSv1_server_method
            && TLSv1_client_method && CRYPTO_set_id_callback && ERR_load_BIO_strings
            ) {
        if (m_bClient && TLSv1_client_method && TLSv1_method)
            return true;
        else if (!m_bClient && TLSv1_server_method && TLSv1_method)
            return true;
    }
    UnloadOpenSSL();
    return false;
}

bool CStartup::StartOpenSSL() {
    if (!SSL_library_init())
        return false;
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    OPENSSL_add_all_algorithms_noconf();
    ERR_load_crypto_strings();
    if (m_bClient)
        m_pMeth = TLSv1_client_method();
    else
        m_pMeth = TLSv1_server_method();
    if (!m_pMeth) {
        StopOpenSSL();
        return false;
    }

    m_pCtx = SSL_CTX_new(m_pMeth);
    if (!m_pCtx) {
        StopOpenSSL();
        return false;
    }
    SSL_CTX_ctrl(m_pCtx, SSL_CTRL_MODE, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER, nullptr);
    m_pSSLLockHandle = new SPA::CUCriticalSection[CRYPTO_num_locks()];
    CRYPTO_set_locking_callback(LockingCallback);
    CRYPTO_set_dynlock_create_callback(dyn_create_function);
    CRYPTO_set_dynlock_lock_callback(dyn_lock_function);
    CRYPTO_set_dynlock_destroy_callback(dyn_destroy_function);
    CRYPTO_set_id_callback(openssl_id_func);
    return true;
}

void CStartup::StopOpenSSL() {
    CRYPTO_set_id_callback(nullptr);
    CRYPTO_set_dynlock_create_callback(nullptr);
    CRYPTO_set_dynlock_lock_callback(nullptr);
    CRYPTO_set_dynlock_destroy_callback(nullptr);

    if (CRYPTO_set_locking_callback)
        CRYPTO_set_locking_callback(nullptr);
    if (m_pSSLLockHandle) {
        delete []m_pSSLLockHandle;
        m_pSSLLockHandle = nullptr;
    }
    if (m_pCtx) {
        SSL_CTX_free(m_pCtx);
        m_pCtx = nullptr;
    }
    if (m_pMeth)
        m_pMeth = nullptr;
}

void CStartup::LockingCallback(int nMode, int nType, const char *strFile, int nLine) {
    assert(nType < g_pStartup->CRYPTO_num_locks());
    if (nMode & CRYPTO_LOCK) {
        g_pStartup->m_pSSLLockHandle[nType].lock();
    } else {
        g_pStartup->m_pSSLLockHandle[nType].unlock();
    }
}

struct CRYPTO_dynlock_value* CStartup::dyn_create_function(const char *file, int line) {
    SPA::CUCriticalSection *p = new SPA::CUCriticalSection;
    return (struct CRYPTO_dynlock_value*) p;
}

void CStartup::dyn_lock_function(int mode, struct CRYPTO_dynlock_value *lock, const char *file, int line) {
    SPA::CUCriticalSection *p = (SPA::CUCriticalSection *)lock;
    if (mode & CRYPTO_LOCK) {
        p->lock();
    } else {
        p->unlock();
    }
}

void CStartup::dyn_destroy_function(struct CRYPTO_dynlock_value *lock, const char *file, int line) {
    SPA::CUCriticalSection *p = (SPA::CUCriticalSection *)lock;
    delete p;
}

unsigned long CStartup::openssl_id_func() {
#ifdef WIN32_64
    return ::GetCurrentThreadId();
#else
    return (unsigned long) 1;
#endif
}