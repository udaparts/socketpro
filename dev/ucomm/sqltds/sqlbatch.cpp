#include "sqlbatch.h"

namespace tds {

	CSqlBatch::CSqlBatch(bool meta) : m_buffer(*m_sb), m_tt(tagTokenType::ttZero), m_meta(meta), m_cols(0) {
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
		CReqBase::Reset();
	}

	bool CSqlBatch::GetClientMessage(unsigned char packet_id, const char16_t *sql, SPA::CUQueue &buffer) {
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

	void CSqlBatch::OnResponse(const unsigned char *data, unsigned int length) {
		assert(length >= sizeof(ResponseHeader));
		memcpy(&ResponseHeader, data, sizeof(ResponseHeader));
		ResponseHeader.Length = ChangeEndian(ResponseHeader.Length);
		assert(ResponseHeader.Length == length);
		data += sizeof(ResponseHeader);
		length -= sizeof(ResponseHeader);
		m_buffer.Push(data, length);
		while (m_buffer.GetSize()) {
			if (m_tt == tagTokenType::ttZero) {
				m_buffer >> m_tt;
			}
			switch (m_tt)
			{
			case tagTokenType::ttCOLMETADATA:
				do {
					if (!m_cols) {
						if (m_buffer.GetSize() < 2) {
							return;
						}
						m_vCol.clear();
						m_buffer >> m_cols;
					}
					if (m_buffer.GetSize() < 10) {
						return;
					}
					unsigned char col_len;
					unsigned short col_name_len;
					
					MetaInfoHeader cmh;
					CDBColumnInfo ci;
					MetaInfoHeader *mih = (MetaInfoHeader*)m_buffer.GetBuffer();
					switch (mih->SqlType)
					{
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
							return;
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
							return;
						}
						tbl_name_len = *(unsigned short*)m_buffer.GetBuffer(Offset);
						tbl_name_len <<= 1;
						Offset += (sizeof(tbl_name_len) + tbl_name_len);
						if (Offset >= m_buffer.GetSize()) {
							return;
						}
						col_name_len = *m_buffer.GetBuffer(Offset);
						col_name_len <<= 1;
						if (col_name_len + Offset + sizeof(col_len) > m_buffer.GetSize()) {
							return;
						}
						m_buffer >> cmh >> max_len >> tbl_name_parts >> tbl_name_len;
						ci.ColumnSize = max_len;
						if (m_meta) {
							// collation ????

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
							return;
						}
						tbl_name_len = *(unsigned short*)m_buffer.GetBuffer(Offset);
						tbl_name_len <<= 1;
						Offset += (sizeof(tbl_name_len) + tbl_name_len);
						if (Offset >= m_buffer.GetSize()) {
							return;
						}
						col_name_len = *m_buffer.GetBuffer(Offset);
						col_name_len <<= 1;
						if (col_name_len + Offset + sizeof(col_len) > m_buffer.GetSize()) {
							return;
						}
						Collation collation;
						m_buffer >> cmh >> max_len >> collation >> tbl_name_parts >> tbl_name_len;
						ci.ColumnSize = max_len;
						if (m_meta) {
							// collation ????

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
							return;
						}
						Collation collation;
						m_buffer >> cmh >> col_chars >> collation >> col_len;
						if ((cmh.SqlType == tagDataType::NCHAR || cmh.SqlType == tagDataType::NVARCHAR) && col_chars != 0xffff) {
							col_chars >>= 1;
						}
						ci.ColumnSize = col_chars;
						if (m_meta) {
							//collation ????
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
							return;
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
					case tagDataType::MONEYN: //smallmoney
					case tagDataType::UNIQUEIDENTIFIER:
					{
						unsigned char bytes;
						constexpr unsigned short Offset = sizeof(MetaInfoHeader) + sizeof(bytes);
						col_name_len = *m_buffer.GetBuffer(Offset);
						col_name_len <<= 1;
						if (col_name_len + Offset + sizeof(col_len) > m_buffer.GetSize()) {
							return;
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
					case tagDataType::XML:
						{
							unsigned char schema;
							unsigned short Offset = sizeof(MetaInfoHeader) + sizeof(schema);
							col_name_len = *m_buffer.GetBuffer(Offset);
							col_name_len <<= 1;
							if (col_name_len + Offset + sizeof(col_len) > m_buffer.GetSize()) {
								return;
							}
							m_buffer >> cmh >> schema >> col_len;
							if (schema) {
								Offset = col_name_len;
								unsigned short pos = *m_buffer.GetBuffer(Offset); //schema
								Offset += (sizeof(col_len) + (pos << 1));
								if (m_buffer.GetSize() <= Offset) {
									return;
								}
								unsigned short type_len = *(unsigned short*)m_buffer.GetBuffer(Offset); //collection
								Offset += (sizeof(type_len) + (type_len << 1));
								if (m_buffer.GetSize() <= Offset) {
									return;
								}
								pos = *m_buffer.GetBuffer(Offset); //column name
								Offset += (sizeof(col_len) + (pos << 1));
								if (m_buffer.GetSize() < Offset) {
									return;
								}
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
						}
						break;
					case tagDataType::SQL_VARIANT:
					{
						unsigned int bytes;
						col_name_len = *m_buffer.GetBuffer(sizeof(MetaInfoHeader) + sizeof(bytes));
						col_name_len <<= 1;
						if (col_name_len + sizeof(MetaInfoHeader) + sizeof(bytes) + sizeof(col_len) > m_buffer.GetSize()) {
							return;
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
							return;
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
					ci.DataType = GetVarType(cmh.SqlType);
					ci.Flags = ToUDBFlags(cmh.Flags);
					if (cmh.SqlType == tagDataType::XML) {
						ci.Flags |= CDBColumnInfo::FLAG_XML;
					}
					if (!m_meta) {
						m_vCol.push_back(ci);
					}
					else {
						ci.DeclaredType = GetSqlDeclaredType(cmh.SqlType, ci.ColumnSize);
						m_vCol.push_back(std::move(ci));
					}
					m_vDT.push_back(mih->SqlType);
					--m_cols;
				} while (m_cols);
				m_tt = tagTokenType::ttZero;
				break;
			case tagTokenType::ttTDS_ERROR:
			case tagTokenType::ttINFO:
				if (m_buffer.GetSize() > 2)
				{
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
					}
				}
				break;
			case tagTokenType::ttENVCHANGE:
				if (m_buffer.GetSize() > 2)
				{
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
					}
				}
				break;
			case tagTokenType::ttDONE:
				if (m_buffer.GetSize() >= sizeof(m_Done)) {
					m_buffer >> m_Done;
					m_tt = tagTokenType::ttZero;
				}
				break;
			default:
				break;
			}
			if (m_tt != tagTokenType::ttZero) {
				break;
			}
		}
	}
}
