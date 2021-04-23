#ifndef _U_TDS_SQL_BATCH_H_
#define _U_TDS_SQL_BATCH_H_

#include "reqbase.h"

namespace tds {

    class CSqlBatch : public CReqBase {
    private:
        SPA::CScopeUQueue m_sb;
        SPA::CScopeUQueue m_sbOut;

        static constexpr unsigned short INVALID_COL = (~0);
		static constexpr unsigned int UINT_NULL_LEN = (~0);
		static constexpr unsigned short USHORT_NULL_LEN = (~0);
		static constexpr unsigned short VAR_MAX = (~0);

    public:
        CSqlBatch(bool meta);

#pragma pack(push,1)

        struct MetaInfoHeader {
            unsigned int UserType = 0;
            ColFlag Flags;
            tagDataType SqlType = tagDataType::SQL_NULL;
        };
#pragma pack(pop)

    public:
        bool GetClientMessage(unsigned char packet_id, const char16_t *sql, SPA::CUQueue &buffer);
        void OnResponse(const unsigned char *data, unsigned int bytes);

    protected:
        void Reset();

    private:
        bool ParseMeta();
        bool ParseEventChange();
        bool ParseErrorInfo();
        bool ParseRow();
        bool ParseNBCRow();
		bool ParseDone();
		bool ParseData(tagDataType dt, CDBColumnInfo *cinfo);
		bool ParseOrder();

    private:
        SPA::CUQueue &m_buffer;
        SPA::CUQueue &m_out;
        TokenDone m_Done;
        std::vector<TokenEventChange> m_vEventChange;
        std::vector<TokenInfo> m_vInfo;
        tagTokenType m_tt;
        CDBColumnInfoArray m_vCol;
        bool m_meta;
        unsigned short m_cols;
        unsigned short m_posCol;
        std::vector<tagDataType> m_vDT;
        Collation m_collation;
        std::vector<unsigned char> m_vNull;
		UINT64 m_lenLarge;
		unsigned int m_endLarge;
		std::vector<unsigned short> m_vOrder;
    };

}

#endif