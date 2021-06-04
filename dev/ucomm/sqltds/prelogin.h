#ifndef _U_TDS_PRELOGIN_H_
#define _U_TDS_PRELOGIN_H_

#include "reqbase.h"

namespace tds {

    class CPrelogin : public CReqBase {
        static std::atomic<unsigned int> g_sequence;
    public:

        enum class tagEncryptionType : unsigned char {
            etOff = 0x00,
            etOn = 0x01,
            etNotSupported = 0x02,
            etRequired = 0x03
        };

    private:
        enum class tagOptionToken : unsigned char {
            VERSION = 0,
            ENCRYPTION = 1,
            INSTOPT = 2,
            THREADID = 3,
            MARS = 4,
            TRACEID = 5,
            FEDAUTHREQUIRED = 6,
            NONCEOPT = 7,
            TOKEN_TERMINATOR = 0xff
        };

        enum class tagFedAuth : unsigned char {
            faNotRequired = 0x00,
            faRequired = 0x01,
            faIllegal = 0x02
        };

#pragma pack(push,1)

        struct Option {
            tagOptionToken Token;
            unsigned short Offset; //big endian
            unsigned short Len; //Big endian
        };
#pragma pack(pop)
        static_assert(sizeof (Option) == 5, "Wrong Option size");

    public:
        CPrelogin(CTdsChannel& channel, bool mars_enabled = false, tagEncryptionType et = tagEncryptionType::etNotSupported);
        int SendTDSMessage(const char *instanceName = "");

        inline bool MarEnabled() const {
            return m_bMars ? true : false;
        }

        inline tagEncryptionType GetEncryptionType() const {
            return m_bEncryption;
        }

        inline bool IsInstFailed() const {
            return InstFailed ? true : false;
        }

        inline unsigned int GetVersion() const {
            return Version;
        }

	private:
		bool ParseStream();

    private:
        unsigned char m_bMars;
        tagFedAuth m_bFed;
        unsigned int Version;
        unsigned short SubBuild;
        tagEncryptionType m_bEncryption;
        unsigned char InstFailed;
    };


}

#endif
