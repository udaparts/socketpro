#include "sspi.h"
#include <assert.h>

namespace tds
{

    CSspi::CSspi() : m_ca(0), m_done(false) {
        ::memset(&m_hText, 0, sizeof (m_hText));
        SECURITY_STATUS ss = ::AcquireCredentialsHandle(
                nullptr,
                (LPWSTR) L"Negotiate",
                SECPKG_CRED_OUTBOUND,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                &m_hCred,
                nullptr);
        if (SEC_E_OK != ss) {
            ::memset(&m_hCred, 0, sizeof (m_hCred));
        }
    }

    CSspi::~CSspi() {
        SECURITY_STATUS ss = SEC_E_OK;
        if (m_hText.dwLower || m_hText.dwUpper) {
            ss = DeleteSecurityContext(&m_hText);
            assert(SEC_E_OK == ss);
        }
        ss = ::FreeCredentialsHandle(&m_hCred);
        assert(SEC_E_OK == ss);
    }

    unsigned long CSspi::GetContextAttributes() const {
        return m_ca;
    }

    bool CSspi::IsDone() const {
        return m_done;
    }

    SECURITY_STATUS CSspi::QuerySecurityContext(const char16_t* target_name, unsigned char *pIn, unsigned int bytes, unsigned char *pOut, unsigned int& cbOut) {
        SECURITY_STATUS ss;
        m_done = false;
        SecBuffer OutSecBuff;
        SecBufferDesc OutBuffDesc;
        OutBuffDesc.ulVersion = 0;
        OutBuffDesc.cBuffers = 1;
        OutBuffDesc.pBuffers = &OutSecBuff;

        OutSecBuff.cbBuffer = cbOut;
        OutSecBuff.BufferType = SECBUFFER_TOKEN;
        OutSecBuff.pvBuffer = pOut;
        if (pIn) {
            SecBufferDesc InBuffDesc;
            SecBuffer InSecBuff;
            InBuffDesc.ulVersion = 0;
            InBuffDesc.cBuffers = 1;
            InBuffDesc.pBuffers = &InSecBuff;

            InSecBuff.cbBuffer = bytes;
            InSecBuff.BufferType = SECBUFFER_TOKEN;
            InSecBuff.pvBuffer = pIn;
            ss = ::InitializeSecurityContext(&m_hCred,
                    &m_hText,
                    (SEC_WCHAR*) target_name,
                    ISC_REQ_CONFIDENTIALITY,
                    0,
                    SECURITY_NATIVE_DREP,
                    &InBuffDesc,
                    0,
                    &m_hText,
                    &OutBuffDesc,
                    &m_ca,
                    nullptr);
        } else {
            ss = ::InitializeSecurityContext(&m_hCred,
                    nullptr,
                    (SEC_WCHAR*) target_name,
                    ISC_REQ_CONFIDENTIALITY,
                    0,
                    SECURITY_NATIVE_DREP,
                    nullptr,
                    0,
                    &m_hText,
                    &OutBuffDesc,
                    &m_ca,
                    nullptr);
        }
        //ISC_RET_USE_SESSION_KEY
        if (ss < 0) {
            return ss;
        }
        if (SEC_I_COMPLETE_NEEDED == ss || SEC_I_COMPLETE_AND_CONTINUE == ss) {
            ss = CompleteAuthToken(&m_hText, &OutBuffDesc);
            if (ss < 0) {
                return ss;
            }
        }
        cbOut = OutSecBuff.cbBuffer;
        m_done = !(SEC_I_CONTINUE_NEEDED == ss || SEC_I_COMPLETE_AND_CONTINUE == ss);
        return ss;
    }
} //namespace tds
