
#include "stdafx.h"
#include "phpdbcolumninfo.h"
#include "phpdbparaminfo.h"

namespace PA
{

    CPhpDBColumnInfo::CPhpDBColumnInfo() {
    }

    CPhpDBColumnInfo::CPhpDBColumnInfo(const SPA::UDB::CDBColumnInfo & ColInfo) : m_ColInfo(ColInfo) {
    }

    void CPhpDBColumnInfo::__destruct() {
    }

    void CPhpDBColumnInfo::__construct(Php::Parameters & params) {
        m_ColInfo.DBPath = SPA::Utilities::ToWide(params[0].stringValue());
        m_ColInfo.TablePath = SPA::Utilities::ToWide(params[1].stringValue());
        m_ColInfo.DisplayName = SPA::Utilities::ToWide(params[2].stringValue());
        m_ColInfo.OriginalName = m_ColInfo.DisplayName;
        VARTYPE vt = (VARTYPE) params[3].numericValue();
        if (!CPhpDBParamInfo::Supported(vt)) {
            throw Php::Exception("Bad data type value");
        }
        m_ColInfo.DataType = vt;
        size_t args = params.size();
        if (args > 4) {
            m_ColInfo.ColumnSize = (unsigned int) params[4].numericValue();
        }
        if (args > 5) {
            m_ColInfo.Flags = (unsigned int) params[5].numericValue();
        }
        if (args > 6) {
            m_ColInfo.Precision = (unsigned int) params[6].numericValue();
        }
        if (args > 7) {
            m_ColInfo.Scale = (unsigned int) params[7].numericValue();
        }
    }

    void CPhpDBColumnInfo::RegisterInto(Php::Namespace & cs) {
        Php::Class<CPhpDBColumnInfo> reg(PHP_DB_COLUMN_INFO);
        reg.method<&CPhpDBColumnInfo::__construct>(PHP_CONSTRUCT,{
            Php::ByVal(PHP_DB_NAME, Php::Type::String),
            Php::ByVal(PHP_TABLE_NAME, Php::Type::String),
            Php::ByVal(PHP_COLUMN_NAME, Php::Type::String),
            Php::ByVal("datatype", Php::Type::Numeric),
            Php::ByVal("colSize", Php::Type::Numeric, false),
            Php::ByVal("flags", Php::Type::Numeric, false),
            Php::ByVal("precision", Php::Type::Numeric, false),
            Php::ByVal("scale", Php::Type::Numeric, false)
        });
        reg.property("NOT_NULL", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_NOT_NULL, Php::Const);
        reg.property("UNIQUE", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_UNIQUE, Php::Const);
        reg.property("PRIMARY_KEY", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_PRIMARY_KEY, Php::Const);
        reg.property("AUTOINCREMENT", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_AUTOINCREMENT, Php::Const);
        reg.property("NOT_WRITABLE", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_NOT_WRITABLE, Php::Const);
        reg.property("ROWID", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_ROWID, Php::Const);
        reg.property("XML", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_XML, Php::Const);
        reg.property("JSON", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_JSON, Php::Const);
        reg.property("CASE_SENSITIVE", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_CASE_SENSITIVE, Php::Const);
        reg.property("ENUM", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_IS_ENUM, Php::Const);
        reg.property("SET", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_IS_SET, Php::Const);
        reg.property("UNSIGNED", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_IS_UNSIGNED, Php::Const);
        reg.property("BIT", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_IS_BIT, Php::Const);
        cs.add(reg);
    }

    Php::Value CPhpDBColumnInfo::__get(const Php::Value & name) {
        if (name == "DBPath") {
            return SPA::Utilities::ToUTF8(m_ColInfo.DBPath);
        } else if (name == "TablePath") {
            return SPA::Utilities::ToUTF8(m_ColInfo.TablePath);
        } else if (name == "DisplayName") {
            return SPA::Utilities::ToUTF8(m_ColInfo.DisplayName);
        } else if (name == "OriginalName") {
            return SPA::Utilities::ToUTF8(m_ColInfo.OriginalName);
        } else if (name == "DeclaredType") {
            return SPA::Utilities::ToUTF8(m_ColInfo.DeclaredType);
        } else if (name == "Collation") {
            return SPA::Utilities::ToUTF8(m_ColInfo.Collation);
        } else if (name == PHP_COLUMN_SIZE) {
            return (int64_t) m_ColInfo.ColumnSize;
        } else if (name == PHP_COLUMN_FLAGS) {
            return (int64_t) m_ColInfo.Flags;
        } else if (name == PHP_DATATYPE) {
            return m_ColInfo.DataType;
        } else if (name == PHP_COLUMN_PRECSISON) {
            return m_ColInfo.Precision;
        } else if (name == PHP_COLUMN_SCALE) {
            return m_ColInfo.Scale;
        }
        return Php::Base::__get(name);
    }
}
