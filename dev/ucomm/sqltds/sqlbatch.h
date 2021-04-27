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

		struct SmallDateTime {
			//days since January 1, 1900
			unsigned short Date;
			unsigned short Minute;
		};

		struct DateTime {
			//days January 1, 1900
			int Day;
			unsigned int SecCount; //300 counts per second
		};

		//days since January 1, year 1
		struct Date {
			unsigned short Low;
			unsigned char High;
		};

		//10-n second increments since 12 AM within a day
		typedef unsigned int Time;	// 3 bytes if 0 <= n < = 2.
									// 4 bytes if 3 <= n < = 4.
									// 5 bytes if 5 <= n < = 7.

		struct DateTime2 {
			Time Time;
			Date Date;
		};

		struct DateTimeOffset : public DateTime2 {
			//time zone offset MUST be between -840 and 840
			short Zone;
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
		bool ParseData(tagDataType dt, unsigned char bytes, unsigned char scale);
		bool ParseVariant();

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