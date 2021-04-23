#include "sqlbatch.h"

namespace tds
{

	CSqlBatch::CSqlBatch(bool meta) : m_buffer(*m_sb), m_out(*m_sbOut), m_tt(tagTokenType::ttZero), m_meta(meta), m_cols(0), m_posCol(INVALID_COL), m_lenLarge(0), m_endLarge(0) {
	}

	void CSqlBatch::Reset() {
		memset(&m_Done, 0, sizeof(m_Done));
		m_vEventChange.clear();
		m_vInfo.clear();
		m_vEventChange.clear();
		m_vInfo.clear();
		m_tt = tagTokenType::ttZero;
		m_vCol.clear();
		m_cols = 0;
		m_vDT.clear();
		m_posCol = INVALID_COL;
		CReqBase::Reset();
	}

	bool CSqlBatch::GetClientMessage(unsigned char packet_id, const char16_t *sql, SPA::CUQueue & buffer) {
		Reset();
		SPA::CScopeUQueue sb;
		//Query packet
		unsigned int len = 22;
		unsigned int header_length = 18;
		unsigned short trans_decriptor_type = 2;
		SPA::UINT64 trans_decriptor = 0;
		unsigned int outstanding_request_count = 1;
		sb << len << header_length << trans_decriptor_type << trans_decriptor << outstanding_request_count;
		sb->Push((const unsigned char*)sql, (unsigned int)(SPA::GetLen(sql) << 1));
		PacketHeader ph(tagPacketType::ptBatch, packet_id);
		ph.Length = (unsigned short)(sb->GetSize() + sizeof(ph));
		ph.Length = ChangeEndian(ph.Length);
		ph.Spid = 0;
		buffer << ph;
		buffer.Push(sb->GetBuffer(), sb->GetSize());
		return true;
	}

	bool CSqlBatch::ParseMeta() {
		do {
			if (!m_cols) {
				if (m_buffer.GetSize() < 2) {
					return false;
				}
				m_vCol.clear();
				m_buffer >> m_cols;
			}
			if (m_buffer.GetSize() < 10) {
				return false;
			}
			unsigned char col_len;
			unsigned short col_name_len;

			MetaInfoHeader cmh;
			CDBColumnInfo ci;
			MetaInfoHeader *mih = (MetaInfoHeader*)m_buffer.GetBuffer();
			switch (mih->SqlType) {
			case tagDataType::SMALLINT:
			case tagDataType::TINYINT:
			case tagDataType::DATEN:
			case tagDataType::DATETIME:
			case tagDataType::INT:
			case tagDataType::BIGINT:
			case tagDataType::BIT:
			case tagDataType::REAL:
			case tagDataType::FLOAT:
			case tagDataType::SMALLMONEY:
			case tagDataType::MONEY:
			{
				col_name_len = *m_buffer.GetBuffer(sizeof(MetaInfoHeader));
				col_name_len <<= 1;
				if (col_name_len + sizeof(MetaInfoHeader) + sizeof(unsigned char) > m_buffer.GetSize()) {
					return false;
				}
				m_buffer >> cmh >> col_len;
			}
			break;
			case tagDataType::IMAGE:
			{
				unsigned char tbl_name_parts;
				unsigned int max_len;
				unsigned short tbl_name_len, Offset = sizeof(MetaInfoHeader) + sizeof(max_len) + sizeof(tbl_name_parts);
				if (Offset >= m_buffer.GetSize()) {
					return false;
				}
				tbl_name_len = *(unsigned short*)m_buffer.GetBuffer(Offset);
				tbl_name_len <<= 1;
				Offset += (sizeof(tbl_name_len) + tbl_name_len);
				if (Offset >= m_buffer.GetSize()) {
					return false;
				}
				col_name_len = *m_buffer.GetBuffer(Offset);
				col_name_len <<= 1;
				if (col_name_len + Offset + sizeof(col_len) > m_buffer.GetSize()) {
					return false;
				}
				m_buffer >> cmh >> max_len >> tbl_name_parts >> tbl_name_len;
				ci.ColumnSize = max_len;
				if (m_meta) {
					ci.TablePath.assign((const char16_t*)m_buffer.GetBuffer(), tbl_name_len);
				}
				tbl_name_len <<= 1;
				m_buffer.Pop(tbl_name_len);
				m_buffer >> col_len;

			}
			break;
			case tagDataType::NTEXT:
			case tagDataType::TEXT:
			{
				unsigned char tbl_name_parts;
				unsigned int max_len;
				unsigned short tbl_name_len, Offset = sizeof(MetaInfoHeader) + sizeof(max_len) + sizeof(Collation) + sizeof(tbl_name_parts);
				if (Offset >= m_buffer.GetSize()) {
					return false;
				}
				tbl_name_len = *(unsigned short*)m_buffer.GetBuffer(Offset);
				tbl_name_len <<= 1;
				Offset += (sizeof(tbl_name_len) + tbl_name_len);
				if (Offset >= m_buffer.GetSize()) {
					return false;
				}
				col_name_len = *m_buffer.GetBuffer(Offset);
				col_name_len <<= 1;
				if (col_name_len + Offset + sizeof(col_len) > m_buffer.GetSize()) {
					return false;
				}
				Collation collation;
				m_buffer >> cmh >> max_len >> collation >> tbl_name_parts >> tbl_name_len;
				ci.ColumnSize = max_len;
				if (m_meta) {
					if (collation != m_collation) {
						ci.Collation = collation.GetString();
						m_collation = collation;
					}
					ci.TablePath.assign((const char16_t*)m_buffer.GetBuffer(), tbl_name_len);
				}
				tbl_name_len <<= 1;
				m_buffer.Pop(tbl_name_len);
				m_buffer >> col_len;

			}
			break;
			case tagDataType::CHAR:
			case tagDataType::VARCHAR:
			case tagDataType::NCHAR:
			case tagDataType::NVARCHAR:
			{
				unsigned short col_chars;
				constexpr unsigned short Offset = sizeof(MetaInfoHeader) + sizeof(col_chars) + sizeof(Collation);
				col_name_len = *m_buffer.GetBuffer(Offset);
				col_name_len <<= 1;
				if (col_name_len + Offset + sizeof(col_len) > m_buffer.GetSize()) {
					return false;
				}
				Collation collation;
				m_buffer >> cmh >> col_chars >> collation >> col_len;
				if ((cmh.SqlType == tagDataType::NCHAR || cmh.SqlType == tagDataType::NVARCHAR) && col_chars != 0xffff) {
					col_chars >>= 1;
				}
				ci.ColumnSize = col_chars;
				if (m_meta) {
					if (collation != m_collation) {
						ci.Collation = collation.GetString();
						m_collation = collation;
					}
				}
			}
			break;
			case tagDataType::BINARY:
			case tagDataType::VARBINARY:
			{
				unsigned short col_chars;
				constexpr unsigned short Offset = sizeof(MetaInfoHeader) + sizeof(col_chars);
				col_name_len = *m_buffer.GetBuffer(Offset);
				col_name_len <<= 1;
				if (col_name_len + Offset + sizeof(col_len) > m_buffer.GetSize()) {
					return false;
				}
				m_buffer >> cmh >> col_chars >> col_len;
				ci.ColumnSize = col_chars;
			}
			break;
			case tagDataType::INTN:
			case tagDataType::DATETIME2N: //datetime2
			case tagDataType::DATETIMEOFFSETN:
			case tagDataType::DATETIMN: //smalldatetime
			case tagDataType::TIMEN:
			case tagDataType::MONEYN:
			case tagDataType::UNIQUEIDENTIFIER:
			{
				unsigned char bytes;
				constexpr unsigned short Offset = sizeof(MetaInfoHeader) + sizeof(bytes);
				col_name_len = *m_buffer.GetBuffer(Offset);
				col_name_len <<= 1;
				if (col_name_len + Offset + sizeof(col_len) > m_buffer.GetSize()) {
					return false;
				}
				m_buffer >> cmh >> bytes >> col_len;
				if (cmh.SqlType == tagDataType::DATETIMEOFFSETN || cmh.SqlType == tagDataType::DATETIME2N) {
					ci.Precision = bytes;
				}
				else if (cmh.SqlType == tagDataType::TIMEN) {
					ci.Scale = bytes;
				}
				else {
					ci.ColumnSize = bytes;
				}
			}
			break;
			case tagDataType::UDT:
			{
				unsigned short max_bytes;
				unsigned short Offset = sizeof(MetaInfoHeader) + sizeof(max_bytes);
				if (Offset + sizeof(col_len) > m_buffer.GetSize()) {
					return false;
				}
				col_name_len = col_len = *m_buffer.GetBuffer(Offset);
				col_name_len <<= 1;
				if (col_name_len + Offset + sizeof(col_len) > m_buffer.GetSize()) {
					return false;
				}
				Offset += (col_name_len + sizeof(col_len));
				unsigned short pos = *m_buffer.GetBuffer(Offset); //schema
				Offset += (sizeof(col_len) + (pos << 1));
				if (m_buffer.GetSize() <= Offset) {
					return false;
				}
				pos = *m_buffer.GetBuffer(Offset); //type name
				Offset += (sizeof(col_len) + (pos << 1));
				if (m_buffer.GetSize() <= Offset) {
					return false;
				}
				unsigned short type_len = *(unsigned short*)m_buffer.GetBuffer(Offset); //assembly
				Offset += (sizeof(type_len) + (type_len << 1));
				if (m_buffer.GetSize() <= Offset) {
					return false;
				}
				pos = *m_buffer.GetBuffer(Offset); //column name
				Offset += (sizeof(col_len) + (pos << 1));
				if (m_buffer.GetSize() < Offset) {
					return false;
				}

				m_buffer >> cmh >> max_bytes >> col_len;
				ci.ColumnSize = max_bytes;
				if (m_meta) {
					ci.DBPath.assign((const char16_t*)m_buffer.GetBuffer(), col_len);
				}
				m_buffer.Pop(col_name_len);
				CDBString schema, assembly;
				m_buffer >> col_len;
				col_name_len = col_len;
				col_name_len <<= 1;
				if (m_meta) {
					schema.assign((const char16_t*)m_buffer.GetBuffer(), col_len);
				}
				m_buffer.Pop(col_name_len);

				m_buffer >> col_len;
				col_name_len = col_len;
				col_name_len <<= 1;
				ci.DeclaredType.assign((const char16_t*)m_buffer.GetBuffer(), col_len);
				m_buffer.Pop(col_name_len);

				m_buffer >> type_len; //2 bytes
				if (m_meta) {
					assembly.assign((const char16_t*)m_buffer.GetBuffer(), type_len);
				}
				type_len <<= 1;
				m_buffer.Pop(type_len);
				if (m_meta) {
					ci.Collation = schema + u'+' + assembly;
				}
				m_buffer >> col_len;
				col_name_len = col_len;
				col_name_len <<= 1;
			}
			break;
			case tagDataType::XML:
			{
				unsigned char schema = *m_buffer.GetBuffer(sizeof(MetaInfoHeader));
				unsigned short Offset = sizeof(MetaInfoHeader) + sizeof(schema);
				if (Offset + sizeof(col_len) > m_buffer.GetSize()) {
					return false;
				}
				col_name_len = col_len = *m_buffer.GetBuffer(Offset);
				col_name_len <<= 1;
				if (col_name_len + Offset + sizeof(col_len) > m_buffer.GetSize()) {
					return false;
				}
				if (schema) {
					Offset += (col_name_len + sizeof(col_len));
					unsigned short pos = *m_buffer.GetBuffer(Offset); //schema
					Offset += (sizeof(col_len) + (pos << 1));
					if (m_buffer.GetSize() <= Offset) {
						return false;
					}
					unsigned short type_len = *(unsigned short*)m_buffer.GetBuffer(Offset); //collection
					Offset += (sizeof(type_len) + (type_len << 1));
					if (m_buffer.GetSize() <= Offset) {
						return false;
					}
					pos = *m_buffer.GetBuffer(Offset); //column name
					Offset += (sizeof(col_len) + (pos << 1));
					if (m_buffer.GetSize() < Offset) {
						return false;
					}
					m_buffer >> cmh >> schema >> col_len;
					if (m_meta) {
						ci.DBPath.assign((const char16_t*)m_buffer.GetBuffer(), col_len);
					}
					m_buffer.Pop(col_name_len);
					CDBString ownerName, collection;
					m_buffer >> col_len;
					col_name_len = col_len;
					col_name_len <<= 1;
					if (m_meta) {
						ownerName.assign((const char16_t*)m_buffer.GetBuffer(), col_len);
					}
					m_buffer.Pop(col_name_len);

					m_buffer >> type_len; //2 bytes
					if (m_meta) {
						collection.assign((const char16_t*)m_buffer.GetBuffer(), type_len);
					}
					type_len <<= 1;
					m_buffer.Pop(type_len);

					m_buffer >> col_len;
					col_name_len = col_len;
					col_name_len <<= 1;
					if (m_meta) {
						ci.Collation = ownerName + u'.' + collection;
					}
				}
				else {
					m_buffer >> cmh >> schema >> col_len;
				}
			}
			break;
			case tagDataType::SQL_VARIANT:
			{
				unsigned int bytes;
				col_name_len = *m_buffer.GetBuffer(sizeof(MetaInfoHeader) + sizeof(bytes));
				col_name_len <<= 1;
				if (col_name_len + sizeof(MetaInfoHeader) + sizeof(bytes) + sizeof(col_len) > m_buffer.GetSize()) {
					return false;
				}
				m_buffer >> cmh >> bytes >> col_len;
				ci.ColumnSize = bytes;
			}
			break;
			case tagDataType::NUMERIC:
			case tagDataType::DECIMAL:
			{
				unsigned char type, precision, scale;
				constexpr unsigned short Offset = sizeof(MetaInfoHeader) + sizeof(type) + sizeof(precision) + sizeof(scale);
				col_name_len = *m_buffer.GetBuffer(Offset);
				col_name_len <<= 1;
				if (col_name_len + Offset + sizeof(unsigned char) > m_buffer.GetSize()) {
					return false;
				}
				m_buffer >> cmh >> type >> precision >> scale >> col_len;
				ci.ColumnSize = type;
				ci.Precision = precision;
				ci.Scale = scale;
			}
			break;
			default:
				assert(false);
				break;
			}
			if (m_meta) {
				ci.DisplayName.assign((const char16_t*)m_buffer.GetBuffer(), col_len);
			}
			m_buffer.Pop(col_name_len);
			ci.DataType = GetVarType(cmh.SqlType, ci.ColumnSize);
			ci.Flags = ToUDBFlags(cmh.Flags);
			if (cmh.SqlType == tagDataType::XML) {
				ci.Flags |= CDBColumnInfo::FLAG_XML;
			}
			if (!m_meta) {
				m_vCol.push_back(ci);
			}
			else {
				if (!ci.DeclaredType.size()) {
					ci.DeclaredType = GetSqlDeclaredType(cmh.SqlType, ci.ColumnSize);
				}
				m_vCol.push_back(std::move(ci));
			}
			m_vDT.push_back(mih->SqlType);
			--m_cols;
		} while (m_cols);
		m_tt = tagTokenType::ttZero;
		size_t count = (m_vCol.size() >> 3) + ((m_vCol.size() % 8) ? 1 : 0);
		m_vNull.assign(count, 0);
		m_posCol = INVALID_COL;
		return true;
	}

	bool CSqlBatch::ParseRow() {
		bool done = IsDone();
		unsigned short cols = (unsigned short)m_vCol.size();
		CDBColumnInfo *cinfo = m_vCol.data();
		tagDataType *dt = m_vDT.data();
		if (m_posCol == INVALID_COL) {
			m_posCol = 0;
		}
		for (unsigned short n = m_posCol; n < cols && m_buffer.GetSize(); ++n, ++cinfo, ++m_posCol) {
			unsigned char nullable = (!(cinfo->Flags & CDBColumnInfo::FLAG_NOT_NULL));
			if (!done) {
				if (m_buffer.GetSize() <= sizeof(PacketHeader) + sizeof(m_Done) + sizeof(tagTokenType)) {
					return false;
				}
				switch (dt[m_posCol])
				{
				case tagDataType::SQL_VARIANT:
					if (m_buffer.GetSize() < cinfo->ColumnSize + sizeof(unsigned int)) {
						return false;
					}
					break;
				case tagDataType::NVARCHAR:
				case tagDataType::NCHAR:
				case tagDataType::BINARY:
				case tagDataType::CHAR:
				case tagDataType::VARCHAR:
				case tagDataType::VARBINARY:
					if (cinfo->ColumnSize != VAR_MAX) {
						unsigned short len = *((unsigned short*)m_buffer.GetBuffer());
						if (len == USHORT_NULL_LEN) {
							m_out << (VARTYPE)VT_NULL;
							m_buffer.Pop(2);
							continue;
						}
						assert(len < USHORT_NULL_LEN);
						assert(len <= 8000);
						if (len + sizeof(unsigned short) > m_buffer.GetSize()) {
							return false;
						}
					}
					else {
						unsigned int len = *((unsigned int*)m_buffer.GetBuffer());
						if (len == UINT_NULL_LEN) {
							m_out << (VARTYPE)VT_NULL;
							m_buffer.Pop(4);
							continue;
						}
						assert(len <= 0x7fffffff);
					}
					break;
				default:
					break;
				}
			}
			else if (nullable) {
				switch (dt[m_posCol])
				{
				case tagDataType::NVARCHAR:
				case tagDataType::NCHAR:
				case tagDataType::BINARY:
				case tagDataType::CHAR:
				case tagDataType::VARBINARY:
				case tagDataType::VARCHAR:
					if (cinfo->ColumnSize == VAR_MAX) {
						unsigned int len = *(unsigned int*)m_buffer.GetBuffer();
						if (len == UINT_NULL_LEN) {
							m_out << (VARTYPE)VT_NULL;
							m_buffer.Pop(4);
							continue;
						}
					}
					else {
						unsigned short len = *(unsigned short*)m_buffer.GetBuffer();
						if (len == 0xffff) {
							m_out << (VARTYPE)VT_NULL;
							m_buffer.Pop(2);
							continue;
						}
					}
					break;
				default:
				{
					unsigned char bytes;
					m_buffer >> bytes;
					if (!bytes) {
						m_out << (VARTYPE)VT_NULL;
						continue;
					}
				}
				break;
				}
			}
			if (!ParseData(dt[m_posCol], cinfo)) {
				return false;
			}
		}
		m_posCol = INVALID_COL;
		m_tt = tagTokenType::ttZero;
		return true;
	}

	bool CSqlBatch::ParseData(tagDataType dt, CDBColumnInfo *cinfo) {
		switch (dt)
		{
		case tagDataType::TINYINT:
			m_out << (VARTYPE)VT_UI1;
			m_out.Push(m_buffer.GetBuffer(), 1);
			m_buffer.Pop(1);
			break;
		case tagDataType::SMALLINT:
			m_out << (VARTYPE)VT_I2;
			m_out.Push(m_buffer.GetBuffer(), 2);
			m_buffer.Pop(2);
			break;
		case tagDataType::INT:
			m_out << (VARTYPE)VT_I4;
			m_out.Push(m_buffer.GetBuffer(), 4);
			m_buffer.Pop(4);
			break;
		case tagDataType::BIGINT:
			m_out << (VARTYPE)VT_I8;
			m_out.Push(m_buffer.GetBuffer(), 8);
			m_buffer.Pop(8);
			break;
		case tagDataType::CHAR:
		case tagDataType::VARCHAR:
		{
			unsigned short len;
			m_buffer >> len;
			m_out << (VARTYPE)(VT_I1 | VT_ARRAY) << (unsigned int)len;
			m_out.Push(m_buffer.GetBuffer(), len);
			m_buffer.Pop(len);
		}
		break;
		case tagDataType::BINARY:
		case tagDataType::VARBINARY:
			if (cinfo->ColumnSize == 0xffff) {
				if (m_out.GetSize()) {
					std::cout << "Blob size: " << m_out.GetSize() - 6 << "\n";
					m_out.SetSize(0);
				}
				m_buffer >> m_lenLarge;
				unsigned int remain;
				m_buffer >> remain;
				m_out << (VARTYPE)(VT_UI1 | VT_ARRAY) << (unsigned int)m_lenLarge;
				m_out.Push(m_buffer.GetBuffer(), remain);
				m_buffer.Pop(remain);
				m_lenLarge -= remain;
				m_endLarge = UINT_NULL_LEN;
				if (m_lenLarge) {
					return false;
				}
				if (m_buffer.GetSize() >= sizeof(unsigned int)) {
					m_buffer >> m_endLarge;
					assert(!m_endLarge);
				}
				else {
					return false;
				}
			}
			else
			{
				unsigned short len;
				m_buffer >> len;
				m_out << (VARTYPE)(VT_UI1 | VT_ARRAY) << (unsigned int)len;
				m_out.Push(m_buffer.GetBuffer(), len);
				m_buffer.Pop(len);
			}
			break;
		case tagDataType::DATETIME:
		{
			UINT64 data;
			m_buffer >> data;
			m_out << (VARTYPE)VT_DATE << data;
		}
		break;
		case tagDataType::DATETIMN:
			if (cinfo->ColumnSize > 4) {
				UINT64 data;
				m_buffer >> data;
				m_out << (VARTYPE)VT_DATE << data;
			}
			else {
				unsigned int data;
				m_buffer >> data;
				m_out << (VARTYPE)VT_DATE << data;
			}
			break;
		case tagDataType::FLOAT:
			m_out << (VARTYPE)VT_R8;
			m_out.Push(m_buffer.GetBuffer(), 8);
			m_buffer.Pop(8);
			break;
		case tagDataType::DECIMAL:
		{
			DECIMAL dec;
			memset(&dec, 0, sizeof(dec));
			unsigned char length, sign;
			m_buffer >> length >> sign;
			if (!sign) {
				dec.sign = 0x80;
			}
			dec.scale = cinfo->Scale;
			switch (length) {
			case 5:
				m_buffer >> dec.Lo32;
				break;
			case 9:
				m_buffer >> dec.Lo64;
				break;
			case 13:
				m_buffer >> dec.Lo64 >> dec.Hi32;
				break;
			case 17:
				m_buffer >> dec.Lo64 >> dec.Hi32;
				m_buffer.Pop(4); //data truncated
				break;
			default:
				assert(false);
				break;
			}
		}
		break;
		case tds::tagDataType::SQL_NULL:
			break;
		case tds::tagDataType::IMAGE:
			break;
		case tds::tagDataType::TEXT:
			break;
		case tds::tagDataType::UNIQUEIDENTIFIER:
			break;
		case tds::tagDataType::INTN:
			break;
		case tds::tagDataType::DATEN:
			break;
		case tds::tagDataType::TIMEN:
			break;
		case tds::tagDataType::DATETIME2N:
			break;
		case tds::tagDataType::DATETIMEOFFSETN:
			break;
		case tds::tagDataType::BIT:
			break;
		case tds::tagDataType::DATETIM4:
			break;
		case tds::tagDataType::REAL:
			break;
		case tds::tagDataType::MONEY:
			break;
		case tds::tagDataType::SQL_VARIANT:
			break;
		case tds::tagDataType::NTEXT:
			break;
		case tds::tagDataType::BITN:
			break;
		case tds::tagDataType::NUMERIC:
			break;
		case tds::tagDataType::FLTN:
			break;
		case tds::tagDataType::MONEYN:
			break;
		case tds::tagDataType::SMALLMONEY:
			break;
		case tds::tagDataType::NVARCHAR:
			break;
		case tds::tagDataType::NCHAR:
			break;
		case tds::tagDataType::UDT:
			break;
		case tds::tagDataType::XML:
			break;
		default:
			assert(false);
			break;
		}
		return true;
	}

	bool CSqlBatch::ParseNBCRow() {
		CDBColumnInfo *cinfo = m_vCol.data();
		unsigned short cols = (unsigned short)m_vCol.size();
		unsigned char *nulls = m_vNull.data();
		tagDataType *pdt = m_vDT.data();
		if (m_posCol == INVALID_COL) {
			if (m_buffer.GetSize() <= m_vNull.size()) {
				return false;
			}
			m_buffer.Pop(nulls, (unsigned int)m_vNull.size());
			m_posCol = 0;
		}
		VARTYPE vt;
		bool done = IsDone();
		for (unsigned short n = m_posCol; n < cols && m_buffer.GetSize(); ++n, ++m_posCol, ++cinfo) {
			bool is_null = (nulls[n >> 3] & (1 << (n % 8)));
			if (is_null) {
				vt = VT_NULL;
				m_out << vt;
				continue;
			}
			tagDataType dt = pdt[m_posCol];
			if (!done) {
				if (m_buffer.GetSize() <= sizeof(PacketHeader) + sizeof(m_Done) + sizeof(tagTokenType)) {
					return false;
				}
				switch (dt)
				{
				case tagDataType::SQL_VARIANT:
					if (m_buffer.GetSize() < cinfo->ColumnSize + sizeof(unsigned int)) {
						return false;
					}
					break;
				case tagDataType::BINARY:
				case tagDataType::CHAR:
				case tagDataType::VARBINARY:
				case tagDataType::VARCHAR:
				case tagDataType::NVARCHAR:
				case tagDataType::NCHAR:
				{
					unsigned short len = *((unsigned char*)m_buffer.GetBuffer());
					if ((unsigned int)(len + 2) > m_buffer.GetSize()) {
						return false;
					}
				}
				break;
				default:
					break;
				}
			}
			if (!ParseData(dt, cinfo)) {
				return false;
			}
		}
		m_posCol = INVALID_COL;
		m_tt = tagTokenType::ttZero;
		return true;
	}

	bool CSqlBatch::ParseOrder() {
		unsigned short len;
		if (m_buffer.GetSize() < sizeof(len)) {
			return false;
		}
		len = *(unsigned short*)m_buffer.GetBuffer();
		if (m_buffer.GetSize() < len + sizeof(len)) {
			return false;
		}
		m_buffer.Pop(2);
		const unsigned short *start = (const unsigned short *)m_buffer.GetBuffer();
		m_vOrder.assign(start, start + (len >> 1));
		m_buffer.Pop(len);
		m_tt = tagTokenType::ttZero;
		return true;
	}

	void CSqlBatch::OnResponse(const unsigned char *data, unsigned int length) {
		assert(length >= sizeof(ResponseHeader));
		memcpy(&ResponseHeader, data, sizeof(ResponseHeader));
		ResponseHeader.Length = ChangeEndian(ResponseHeader.Length);
		assert(ResponseHeader.Length == length);
		ResponseHeader.Spid = ChangeEndian(ResponseHeader.Spid);
		data += sizeof(ResponseHeader);
		length -= sizeof(ResponseHeader);
		m_buffer.Push(data, length);
		while (m_lenLarge && m_buffer.GetSize()) {
			unsigned int len;
			m_buffer >> len;
			assert(m_buffer.GetSize() >= len);
			m_out.Push(m_buffer.GetBuffer(), len);
			m_buffer.Pop(len);
			m_lenLarge -= len;
			if (!m_lenLarge) {
				++m_posCol;
				if (m_posCol == m_vCol.size()) {
					m_posCol = INVALID_COL;
					m_tt = tagTokenType::ttZero;
				}
			}
			else {
				assert(m_lenLarge < 0x7fffffff);
			}
		}
		if (!m_lenLarge && m_endLarge == UINT_NULL_LEN) {
			if (m_buffer.GetSize() >= sizeof(m_endLarge)) {
				m_buffer >> m_endLarge;
				assert(!m_endLarge);
			}
			else {
				return;
			}
		}
		while (m_buffer.GetSize()) {
			if (m_tt == tagTokenType::ttZero) {
				m_buffer >> m_tt;
			}
			switch (m_tt) {
			case tagTokenType::ttORDER:
				if (!ParseOrder()) {
					return;
				}
				break;
			case tagTokenType::ttNBCROW:
				if (!ParseNBCRow()) {
					return;
				}
				break;
			case tagTokenType::ttROW:
				if (!ParseRow()) {
					return;
				}
				break;
			case tagTokenType::ttCOLMETADATA:
				if (!ParseMeta()) {
					return;
				}
				break;
			case tagTokenType::ttTDS_ERROR:
			case tagTokenType::ttINFO:
				if (!ParseErrorInfo()) {
					return;
				}
				break;
			case tagTokenType::ttENVCHANGE:
				if (!ParseEventChange()) {
					return;
				}
				break;
			case tagTokenType::ttDONE:
				if (!ParseDone()) {
					return;
				}
				break;
			default:
				assert(false);
				break;
			}
			if (m_tt != tagTokenType::ttZero) {
				break;
			}
		}
	}

	bool CSqlBatch::ParseDone() {
		if (m_buffer.GetSize() >= sizeof(m_Done)) {
			m_buffer >> m_Done;
			m_tt = tagTokenType::ttZero;
			if (m_Done.Status == tagDoneStatus::dsFinal || (m_Done.Status & tagDoneStatus::dsMore) == tagDoneStatus::dsMore) {
				m_posCol = INVALID_COL;
				memset(&m_collation, 0, sizeof(m_collation));
				m_cols = 0;
				m_vCol.clear();
				m_vDT.clear();
				m_vNull.clear();
				m_out.SetSize(0);
			}
			return true;
		}
		return false;
	}

	bool CSqlBatch::ParseErrorInfo() {
		if (m_buffer.GetSize() > 2) {
			unsigned short *header = (unsigned short *)m_buffer.GetBuffer();
			if (*header <= m_buffer.GetSize()) {
				unsigned short len;
				TokenInfo ti;
				m_buffer >> len >> ti.SQLErrorNumber >> ti.State >> ti.Class;
				m_buffer >> len;
				const char16_t *str = (const char16_t *)m_buffer.GetBuffer();
				ti.ErrorMessage.assign(str, str + len);
				m_buffer.Pop(((unsigned int)len) << 1);
				unsigned char byteLen;
				m_buffer >> byteLen;
				str = (const char16_t *)m_buffer.GetBuffer();
				ti.ServerName.assign(str, str + byteLen);
				m_buffer.Pop(((unsigned int)byteLen) << 1);
				m_buffer >> ti.ProcessNameLength >> ti.LineNumber;
				m_vInfo.push_back(ti);
				m_tt = tagTokenType::ttZero;
				return true;
			}
		}
		return false;
	}

	bool CSqlBatch::ParseEventChange() {
		if (m_buffer.GetSize() > 2) {
			unsigned short *header = (unsigned short *)m_buffer.GetBuffer();
			if (*header <= m_buffer.GetSize()) {
				unsigned short len;
				unsigned char b;
				tagEnvchangeType type;
				m_buffer >> len >> type;
				switch (type) {
				case tagEnvchangeType::database:
				{
					TokenEventChange tec;
					m_buffer >> b;
					tec.Type = type;
					const char16_t *str = (const char16_t *)m_buffer.GetBuffer();
					tec.NewValue.assign(str, str + b);
					m_buffer.Pop(((unsigned int)b) << 1);
					m_buffer >> b;
					if (b) {
						str = (const char16_t *)m_buffer.GetBuffer();
						tec.OldValue.assign(str, str + b);
						m_buffer.Pop(((unsigned int)b) << 1);
					}
					m_vEventChange.push_back(tec);
				}
				break;
				default:
					assert(false);
					break;
				}
				m_tt = tagTokenType::ttZero;
				return true;
			}
		}
		return false;
	}
}
