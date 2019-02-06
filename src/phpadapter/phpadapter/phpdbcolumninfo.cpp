
#include "stdafx.h"
#include "phpdbcolumninfo.h"

namespace PA {

	CPhpDBColumnInfo::CPhpDBColumnInfo(const SPA::UDB::CDBColumnInfo &ColInfo) : m_ColInfo(ColInfo) {
	}

	void CPhpDBColumnInfo::__construct(Php::Parameters &params) {

	}

	void CPhpDBColumnInfo::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpDBColumnInfo> reg(PHP_DB_COLUMN_IFO);
		reg.method(PHP_CONSTRUCT, &CPhpDBColumnInfo::__construct, Php::Private);
		reg.property("NOT_NULL", (int64_t)SPA::UDB::CDBColumnInfo::FLAG_NOT_NULL, Php::Const);
		reg.property("UNIQUE", (int64_t)SPA::UDB::CDBColumnInfo::FLAG_UNIQUE, Php::Const);
		reg.property("PRIMARY_KEY", (int64_t)SPA::UDB::CDBColumnInfo::FLAG_PRIMARY_KEY, Php::Const);
		reg.property("AUTOINCREMENT", (int64_t)SPA::UDB::CDBColumnInfo::FLAG_AUTOINCREMENT, Php::Const);
		reg.property("NOT_WRITABLE", (int64_t)SPA::UDB::CDBColumnInfo::FLAG_NOT_WRITABLE, Php::Const);
		reg.property("ROWID", (int64_t)SPA::UDB::CDBColumnInfo::FLAG_ROWID, Php::Const);
		reg.property("XML", (int64_t)SPA::UDB::CDBColumnInfo::FLAG_XML, Php::Const);
		reg.property("JSON", (int64_t)SPA::UDB::CDBColumnInfo::FLAG_JSON, Php::Const);
		reg.property("CASE_SENSITIVE", (int64_t)SPA::UDB::CDBColumnInfo::FLAG_CASE_SENSITIVE, Php::Const);
		reg.property("ENUM", (int64_t)SPA::UDB::CDBColumnInfo::FLAG_IS_ENUM, Php::Const);
		reg.property("SET", (int64_t)SPA::UDB::CDBColumnInfo::FLAG_IS_SET, Php::Const);
		reg.property("UNSIGNED", (int64_t)SPA::UDB::CDBColumnInfo::FLAG_IS_UNSIGNED, Php::Const);
		reg.property("BIT", (int64_t)SPA::UDB::CDBColumnInfo::FLAG_IS_BIT, Php::Const);
		cs.add(reg);
	}

	Php::Value CPhpDBColumnInfo::__get(const Php::Value &name) {
		if (name == "DBPath") {
			return SPA::Utilities::ToUTF8(m_ColInfo.DBPath.c_str(), m_ColInfo.DBPath.size());
		}
		else if (name == "TablePath") {
			return SPA::Utilities::ToUTF8(m_ColInfo.TablePath.c_str(), m_ColInfo.TablePath.size());
		}
		else if (name == "DisplayName") {
			return SPA::Utilities::ToUTF8(m_ColInfo.DisplayName.c_str(), m_ColInfo.DisplayName.size());
		}
		else if (name == "OriginalName") {
			return SPA::Utilities::ToUTF8(m_ColInfo.OriginalName.c_str(), m_ColInfo.OriginalName.size());
		}
		else if (name == "DeclaredType") {
			return SPA::Utilities::ToUTF8(m_ColInfo.DeclaredType.c_str(), m_ColInfo.DeclaredType.size());
		}
		else if (name == "Collation") {
			return SPA::Utilities::ToUTF8(m_ColInfo.Collation.c_str(), m_ColInfo.Collation.size());
		}
		else if (name == "ColumnSize") {
			return (int64_t)m_ColInfo.ColumnSize;
		}
		else if (name == "Flags") {
			return (int64_t)m_ColInfo.Flags;
		}
		else if (name == "DataType") {
			return m_ColInfo.DataType;
		}
		else if (name == "Precision") {
			return m_ColInfo.Precision;
		}
		else if (name == "Scale") {
			return m_ColInfo.Scale;
		}
		return Php::Base::__get(name);
	}
}
