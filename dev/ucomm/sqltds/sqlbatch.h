#ifndef _U_TDS_SQL_BATCH_H_
#define _U_TDS_SQL_BATCH_H_

#include "transmanager.h"

namespace tds {

    class CSqlBatch : public CTransManager {
    private:
        SPA::CScopeUQueue m_sbOut;

        static constexpr unsigned short INVALID_COL = (~0);
        static constexpr unsigned int UINT_NULL_LEN = (~0);
        static constexpr unsigned short USHORT_NULL_LEN = (~0);
        static constexpr unsigned short VAR_MAX = (~0);
        static constexpr UINT64 UNKNOWN_XML_LEN = 0xfffffffffffffffe;
        static constexpr unsigned int MAX_IMAGE_TEXT_LEN = 0x7fffffff;
        static constexpr unsigned int MAX_NTEXT_LEN = 0x7ffffffe;

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
        bool GetClientMessage(unsigned char packet_id, const char16_t *sql, SPA::CUQueue &buffer, SPA::UINT64 trans_decriptor = 0);

    protected:
        void Reset();
		bool ParseStream();

    private:
        bool ParseMeta();
        bool ParseEventChange();
        bool ParseRow();
        bool ParseNBCRow();
        bool ParseDone();
        bool ParseData(tagDataType dt, CDBColumnInfo *cinfo);
        bool ParseOrder();
        bool ParseData(tagDataType dt, unsigned char bytes, unsigned char scale);
        bool ParseVariant(CDBColumnInfo *cinfo);
		bool ParseDoneInProc();
		bool ParseReturnStatus();

    private:
        SPA::CUQueue &m_out;
		std::vector<StringEventChange> m_vEventChange;
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
		DoneInProc m_dip;
		unsigned int m_rs; //ReturnStatus;
    };

}

#endif