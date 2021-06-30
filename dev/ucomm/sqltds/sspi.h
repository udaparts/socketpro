#ifndef _U_TDS_WIN_SSPI_H_
#define _U_TDS_WIN_SSPI_H_

#include "../include/membuffer.h"
#include "../include/channelpool.h"

#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif // !SECURITY_WIN32

#include <windows.h>
#include <sspi.h>

namespace tds {

    class CSspi {
    public:
        CSspi();
        CSspi(const CSspi& sspi) = delete;
        virtual ~CSspi();

    public:
        unsigned long GetContextAttributes() const;
        SECURITY_STATUS QuerySecurityContext(const char16_t* target_name, unsigned char* pIn, unsigned int bytes, unsigned char* pOut, unsigned int& cbOut);
        bool IsDone() const;
        CSspi& operator=(const CSspi& sspi) = delete;

    private:
        CredHandle m_hCred;
        CredHandle m_hText;
        unsigned long m_ca;
        bool m_done;
    };
} //namespace tds

#endif