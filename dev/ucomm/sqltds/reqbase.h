#ifndef _U_TDS_REQUEST_BASE_H_
#define _U_TDS_REQUEST_BASE_H_

#include "tdsdef.h"
#include "../include/channelpool.h"
#include <mutex>
#include <condition_variable>

namespace tds {
    //using namespace SPA::UDB;
    class CTdsChannel;

    class CReqBase {
    private:
        SPA::CScopeUQueue m_sb;

    protected:
        typedef unsigned short Packet_Length; //big-endian 512 - 32,767
        typedef unsigned short SPID; //big-endian
        typedef unsigned char Token;

        static const Token TOKEN_TERMINATOR = 0xff;

#pragma pack(push,1)

        enum class tagPacketType : unsigned char {
            ptInitial = 0,
            ptBatch = 1, //SQL batch
            ptPre7Login = 2, //Pre-TDS7 Login
            ptRpc = 3,
            ptResponse = 4, //Tabular result, server-only response type
            ptAttention = 6, //Attention signal
            ptBulk = 7, //Bulk load data
            ptFederated = 8, //Federated Authentication Token
            ptTransaction = 14, //Transaction manager request
            ptLogin7 = 16, //TDS7 Login
            ptSspi = 17,
            ptPrelogin = 18, //Pre-Login
            ptFedAuthInfo = 238
        };

        enum class tagPacketStatus : unsigned char {
            psNormal = 0,
            psEOM = 1,
            psIgnore = 2, //client ==> server + psEOM
            psResetConnection = 8, //client ==> server
            psResetConnectionSkipTran = 16, //client ==> server
        };

        enum class tagTokenType : unsigned char {
            ttZero = 0,
            ttOFFSET = 0x78,
            ttRETURNSTATUS = 0x79,
            ttCOLMETADATA = 0x81,
            ttALTMETADATA = 0x88,
            ttDATACLASSIFICATION = 0xa3,
            ttTABNAME = 0xa4,
            ttCOLINFO = 0xa5,
            ttORDER = 0xa9,
            ttTDS_ERROR = 0xaa,
            ttINFO = 0xab,
            ttRETURNVALUE = 0xac,
            ttLOGINACK = 0xad,
            ttFEATUREEXTACK = 0xae,
            ttROW = 0xd1,
            ttNBCROW = 0xd2,
            ttALTROW = 0xd3,
            ttENVCHANGE = 0xe3,
            ttSESSIONSTATE = 0xe4,
            ttSSPI = 0xed,
            ttFEDAUTHINFO = 0xee,
            ttDONE = 0xfd,
            ttDONEPROC = 0xfe,
            ttDONEINPROC = 0xff
        };

        enum tagDoneStatus : unsigned short {
            dsFinal = 0,
            dsMore = 0x01,
            dsError = 0x02,
            dsInTrans = 0x04,
            dsCount = 0x10,
            dsAttention = 0x20,
            dsSrvError = 0x100,
            dsInitial = 0xffff
        };

        struct PacketHeader {

            PacketHeader(tagPacketType type, unsigned char packetId) : Type(type), PacketID(packetId) {
            }
            tagPacketType Type;
            tagPacketStatus Status = tagPacketStatus::psEOM;
            Packet_Length Length = 0;
            SPID Spid = 0;
            unsigned char PacketID;
            unsigned char Window = 0; //ignored
        };

        struct TokenDone {
            tagDoneStatus Status = tagDoneStatus::dsInitial;
            unsigned short Operation = 0;
            UINT64 RowCount = 0;
        };

        struct CollationFlag {

            CollationFlag() {
                ::memset(this, 0, sizeof (CollationFlag));
            }
            unsigned short Reserved : 4;
            unsigned short fIgnoreCase : 1;
            unsigned short fIgnoreAccent : 1;
            unsigned short fIgnoreWidth : 1;
            unsigned short fIgnoreKana : 1;
            unsigned short fBinary : 1;
            unsigned short fBinary2 : 1;
            unsigned short fUTF8 : 1;
            unsigned short fReserved : 1;
            unsigned short Version : 4;

            unsigned short GetValue() const {
                return *(unsigned short*) this;
            }
        };

        struct Collation {
            unsigned short CodePage = 0; //LCID
            CollationFlag Flags;
            unsigned char SortOrder = 0;

            inline bool operator==(const Collation& c) const noexcept {
                return (!::memcmp(this, &c, sizeof (c)));
            }

            inline bool operator!=(const Collation& c) const noexcept {
                return ::memcmp(this, &c, sizeof (c));
            }

            CDBString GetString() const {
                char str[16];
#ifdef WIN32_64
                sprintf_s(str, "%x|%x|%x", CodePage, Flags.GetValue(), SortOrder);
#else
                sprintf(str, "%x|%x|%x", CodePage, Flags.GetValue(), SortOrder);
#endif
                size_t len = ::strlen(str);
                return CDBString(str, str + len);
            }
        };

        typedef TokenDone DoneInProc;

#pragma pack(pop)

        static_assert(sizeof (PacketHeader) == 8, "Wrong PacketHeader size");
        static_assert(sizeof (TokenDone) == 12, "Wrong TokenDone size");

        struct TokenInfo {
            unsigned int SQLErrorNumber = 0;
            unsigned char State = 0;
            unsigned char Class = 0;
            CDBString ErrorMessage;
            CDBString ServerName;
            unsigned char ProcessNameLength = 0;
            CDBString ProcName;
            unsigned int LineNumber = 0;

            void Reset() {
                SQLErrorNumber = 0;
                State = 0;
                Class = 0;
                ErrorMessage.clear();
                ProcessNameLength = 0;
                ProcName.clear();
                LineNumber = 0;
            }
        };

    public:
        CReqBase(CTdsChannel &channel);
        virtual ~CReqBase();

    public:
        bool IsDone();
        bool HasMore();
        UINT64 GetCount();
        virtual bool Wait(unsigned int ms);
        int Send(const unsigned char* buffer, unsigned int bytes, unsigned int milliseconds, bool sync = true);
        int GetSQLError(SPA::CDBString& errMsg);
        CTdsChannel& GetChannel();

    protected:
        const PacketHeader& GetResponseHeader() const;
        virtual void Reset();
        virtual bool ParseError();
        virtual bool ParseDone();
        static unsigned char GetFixLen(Token token);
        static SPID GetSPID();
        static unsigned int GetThreadId();
        static unsigned short ChangeEndian(unsigned short s);
        static unsigned int ChangeEndian(unsigned int s);
        static int ChangeEndian(int s);

    private:
        virtual bool ParseStream() = 0;
        void OnResponse(const unsigned char* data, unsigned int bytes);
        inline bool HasMoreInternal() const;
        inline UINT64 GetCountInternal() const;
        inline bool IsDoneInternal() const;
        void OnChannelClosed();

    protected:
        CTdsChannel& m_channel;
        SPA::CUQueue &m_buffer;
        tagTokenType m_tt;
        TokenDone m_Done; //protected by m_cs
        Collation m_collation;

    private:
        typedef std::unique_lock<std::mutex> CAutoLock;
        std::mutex m_csSend;
        std::mutex m_cs;
        std::condition_variable m_cv;
        bool m_bWaiting; //protected by m_cs
        PacketHeader ResponseHeader; //protected by m_cs
        TokenInfo m_errCode; //protected by m_cs
        friend class CTdsChannel;
    };

}

#endif