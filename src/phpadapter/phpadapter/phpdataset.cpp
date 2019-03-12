#include "stdafx.h"
#include "phpdataset.h"
#include "phptable.h"
#include "phpdbcolumninfo.h"

namespace PA
{

    CPhpDataSet::CPhpDataSet(SPA::CDataSet & ds) : m_ds(ds) {
    }

    void CPhpDataSet::__construct(Php::Parameters & params) {
    }

    void CPhpDataSet::__destruct() {
    }

    void CPhpDataSet::AddEmptyRowset(Php::Parameters & params) {
        SPA::UDB::CDBColumnInfoArray vCol;
        const Php::Value &meta = params[0];
        int count = meta.length();
        for (int n = 0; n < count; ++n) {
            Php::Value col = meta.get(n);
            if (!col.instanceOf(SPA_CS_NS + PHP_DB_COLUMN_INFO)) {
                throw Php::Exception("An array of column info structures expected");
            }
            SPA::UDB::CDBColumnInfo ci;
            ci.DBPath = SPA::Utilities::ToWide(col.get("DBPath").stringValue());
            ci.TablePath = SPA::Utilities::ToWide(col.get("TablePath").stringValue());
            ci.DisplayName = SPA::Utilities::ToWide(col.get("DisplayName").stringValue());
            ci.OriginalName = SPA::Utilities::ToWide(col.get("OriginalName").stringValue());
            ci.DeclaredType = SPA::Utilities::ToWide(col.get("DeclaredType").stringValue());
            ci.Collation = SPA::Utilities::ToWide(col.get("Collation").stringValue());
            ci.ColumnSize = (unsigned int) col.get(PHP_COLUMN_SIZE).numericValue();
            ci.Flags = (unsigned int) col.get(PHP_COLUMN_FLAGS).numericValue();
            ci.DataType = (VARTYPE) col.get(PHP_DATATYPE).numericValue();
            ci.Precision = (unsigned char) col.get(PHP_COLUMN_PRECSISON).numericValue();
            ci.Scale = (unsigned char) col.get(PHP_COLUMN_SCALE).numericValue();
            vCol.push_back(std::move(ci));
        }
        m_ds.AddEmptyRowset(vCol);
    }

    void CPhpDataSet::CheckResult(size_t res) {
        switch ((int) res) {
            case 0:
                break;
            case SPA::CTable::BAD_ORDINAL:
                throw Php::Exception("Bad column ordinal value");
            case SPA::CTable::BAD_DATA_TYPE:
                throw Php::Exception("Bad data type");
            case SPA::CTable::OPERATION_NOT_SUPPORTED:
                throw Php::Exception("Operation not supported");
            case SPA::CTable::COMPARISON_NOT_SUPPORTED:
                throw Php::Exception("Comparision not supported");
            case SPA::CTable::NO_TABLE_NAME_GIVEN:
                throw Php::Exception("Table name not given");
            case SPA::CTable::NO_TABLE_FOUND:
                throw Php::Exception("Table not found");
            default:
                break;
        }
    }

    void CPhpDataSet::AddRows(Php::Parameters & params) {
        std::wstring dbName = SPA::Utilities::ToWide(params[0].stringValue());
        std::wstring tableName = SPA::Utilities::ToWide(params[1].stringValue());
        SPA::UDB::CDBVariantArray vData;
        const Php::Value &arr = params[2];
        int count = arr.length();
        for (int n = 0; n < count; ++n) {
            SPA::UDB::CDBVariant vt;
            ToVariant(arr.get(n), vt);
            vData.push_back(std::move(vt));
        }
        size_t res = m_ds.AddRows(dbName.c_str(), tableName.c_str(), vData);
        CheckResult(res);
    }

    void CPhpDataSet::RegisterInto(Php::Namespace & spa) {
        Php::Class<CPhpDataSet> ds(PHP_DATASET);
        ds.method<&CPhpDataSet::__construct>(PHP_CONSTRUCT, Php::Private);
        ds.method<&CPhpDataSet::Empty>(PHP_EMPTY);
        ds.method<&CPhpDataSet::AddEmptyRowset>("AddEmptyRowset",{
            Php::ByVal("meta", Php::Type::Array)
        });
        ds.method<&CPhpDataSet::AddRows>("AddRows",{
            Php::ByVal(PHP_DB_NAME, Php::Type::String),
            Php::ByVal(PHP_TABLE_NAME, Php::Type::String),
            Php::ByVal("data", Php::Type::Array)
        });
        ds.method<&CPhpDataSet::GetColumMeta>("GetColumMeta",{
            Php::ByVal(PHP_DB_NAME, Php::Type::String),
            Php::ByVal(PHP_TABLE_NAME, Php::Type::String)
        });
        ds.method<&CPhpDataSet::GetRowCount>("GetRowCount",{
            Php::ByVal(PHP_DB_NAME, Php::Type::String),
            Php::ByVal(PHP_TABLE_NAME, Php::Type::String)
        });
        ds.method<&CPhpDataSet::GetColumnCount>("GetColumnCount",{
            Php::ByVal(PHP_DB_NAME, Php::Type::String),
            Php::ByVal(PHP_TABLE_NAME, Php::Type::String)
        });
        ds.method<&CPhpDataSet::FindKeys>("FindKeys",{
            Php::ByVal(PHP_DB_NAME, Php::Type::String),
            Php::ByVal(PHP_TABLE_NAME, Php::Type::String)
        });
        ds.method<&CPhpDataSet::FindOrdinal>("FindOrdinal",{
            Php::ByVal(PHP_DB_NAME, Php::Type::String),
            Php::ByVal(PHP_TABLE_NAME, Php::Type::String),
            Php::ByVal(PHP_COLUMN_NAME, Php::Type::String)
        });
        ds.method<&CPhpDataSet::UpdateARow>("UpdateARow",{
            Php::ByVal(PHP_DB_NAME, Php::Type::String),
            Php::ByVal(PHP_TABLE_NAME, Php::Type::String),
            Php::ByVal("keys", Php::Type::Null)
        });
        ds.method<&CPhpDataSet::DeleteARow>("DeleteARow",{
            Php::ByVal(PHP_DB_NAME, Php::Type::String),
            Php::ByVal(PHP_TABLE_NAME, Php::Type::String),
            Php::ByVal("keys", Php::Type::Null)
        });
        ds.method<&CPhpDataSet::Between>("Between",{
            Php::ByVal(PHP_DB_NAME, Php::Type::String),
            Php::ByVal(PHP_TABLE_NAME, Php::Type::String),
            Php::ByVal(PHP_ORDINAL, Php::Type::Numeric),
            Php::ByVal(PHP_VARIANT_V0, Php::Type::Null),
            Php::ByVal(PHP_VARIANT_V1, Php::Type::Null)
        });
        ds.method<&CPhpDataSet::FindNull>("FindNull",{
            Php::ByVal(PHP_DB_NAME, Php::Type::String),
            Php::ByVal(PHP_TABLE_NAME, Php::Type::String),
            Php::ByVal(PHP_ORDINAL, Php::Type::Numeric)
        });
        ds.method<&CPhpDataSet::In>("In",{
            Php::ByVal(PHP_DB_NAME, Php::Type::String),
            Php::ByVal(PHP_TABLE_NAME, Php::Type::String),
            Php::ByVal(PHP_ORDINAL, Php::Type::Numeric),
            Php::ByVal(PHP_VARIANT_V, Php::Type::Null)
        });
        ds.method<&CPhpDataSet::NotIn>("NotIn",{
            Php::ByVal(PHP_DB_NAME, Php::Type::String),
            Php::ByVal(PHP_TABLE_NAME, Php::Type::String),
            Php::ByVal(PHP_ORDINAL, Php::Type::Numeric),
            Php::ByVal(PHP_VARIANT_V, Php::Type::Null)
        });
        ds.method<&CPhpDataSet::Find>("Find",{
            Php::ByVal(PHP_DB_NAME, Php::Type::String),
            Php::ByVal(PHP_TABLE_NAME, Php::Type::String),
            Php::ByVal(PHP_ORDINAL, Php::Type::Numeric),
            Php::ByVal(PHP_TABLE_OP, Php::Type::Numeric),
            Php::ByVal(PHP_VARIANT_V, Php::Type::Null)
        });
        spa.add(ds);
    }

    Php::Value CPhpDataSet::Find(Php::Parameters & params) {
        std::wstring dbName = SPA::Utilities::ToWide(params[0].stringValue());
        std::wstring tableName = SPA::Utilities::ToWide(params[1].stringValue());
        unsigned int ordinal = (unsigned int) params[2].numericValue();
        int64_t n = params[3].numericValue();
        if (n < SPA::CTable::equal || n > SPA::CTable::is_null) {
            throw Php::Exception("Bad operation value");
        }
        SPA::CTable::Operator op = (SPA::CTable::Operator)n;
        SPA::UDB::CDBVariant vt;
        ToVariant(params[4], vt);
        std::shared_ptr<SPA::CTable> pTable(new SPA::CTable);
        auto res = m_ds.Find(dbName.c_str(), tableName.c_str(), ordinal, op, vt, *pTable);
        CheckResult((size_t) res);
        return Php::Object((SPA_NS + PHP_TABLE).c_str(), new CPhpTable(pTable));
    }

    Php::Value CPhpDataSet::In(Php::Parameters & params) {
        std::wstring dbName = SPA::Utilities::ToWide(params[0].stringValue());
        std::wstring tableName = SPA::Utilities::ToWide(params[1].stringValue());
        unsigned int ordinal = (unsigned int) params[2].numericValue();
        SPA::UDB::CDBVariantArray v;
        ToArray(params[3], v);
        std::shared_ptr<SPA::CTable> pTable(new SPA::CTable);
        auto res = m_ds.In(dbName.c_str(), tableName.c_str(), ordinal, v, *pTable);
        CheckResult((size_t) res);
        return Php::Object((SPA_NS + PHP_TABLE).c_str(), new CPhpTable(pTable));
    }

    Php::Value CPhpDataSet::NotIn(Php::Parameters & params) {
        std::wstring dbName = SPA::Utilities::ToWide(params[0].stringValue());
        std::wstring tableName = SPA::Utilities::ToWide(params[1].stringValue());
        unsigned int ordinal = (unsigned int) params[2].numericValue();
        SPA::UDB::CDBVariantArray v;
        ToArray(params[3], v);
        std::shared_ptr<SPA::CTable> pTable(new SPA::CTable);
        auto res = m_ds.NotIn(dbName.c_str(), tableName.c_str(), ordinal, v, *pTable);
        CheckResult((size_t) res);
        return Php::Object((SPA_NS + PHP_TABLE).c_str(), new CPhpTable(pTable));
    }

    Php::Value CPhpDataSet::FindNull(Php::Parameters & params) {
        std::wstring dbName = SPA::Utilities::ToWide(params[0].stringValue());
        std::wstring tableName = SPA::Utilities::ToWide(params[1].stringValue());
        unsigned int ordinal = (unsigned int) params[2].numericValue();
        std::shared_ptr<SPA::CTable> pTable(new SPA::CTable);
        auto res = m_ds.FindNull(dbName.c_str(), tableName.c_str(), ordinal, *pTable);
        CheckResult((size_t) res);
        return Php::Object((SPA_NS + PHP_TABLE).c_str(), new CPhpTable(pTable));
    }

    Php::Value CPhpDataSet::Between(Php::Parameters & params) {
        std::wstring dbName = SPA::Utilities::ToWide(params[0].stringValue());
        std::wstring tableName = SPA::Utilities::ToWide(params[1].stringValue());
        unsigned int ordinal = (unsigned int) params[2].numericValue();
        SPA::UDB::CDBVariant vt0, vt1;
        ToVariant(params[3], vt0);
        ToVariant(params[4], vt1);
        std::shared_ptr<SPA::CTable> pTable(new SPA::CTable);
        auto res = m_ds.Between(dbName.c_str(), tableName.c_str(), ordinal, vt0, vt1, *pTable);
        CheckResult((size_t) res);
        return Php::Object((SPA_NS + PHP_TABLE).c_str(), new CPhpTable(pTable));
    }

    Php::Value CPhpDataSet::UpdateARow(Php::Parameters & params) {
        std::wstring dbName = SPA::Utilities::ToWide(params[0].stringValue());
        std::wstring tableName = SPA::Utilities::ToWide(params[1].stringValue());
        Php::Value keys = params[2];
        std::vector<CComVariant> v;
        ToArray(keys, v);
        auto res = m_ds.UpdateARow(dbName.c_str(), tableName.c_str(), v.data(), v.size());
        CheckResult(res);
        return (int64_t) res;
    }

    void CPhpDataSet::ToArray(const Php::Value &v, SPA::UDB::CDBVariantArray & vData) {
        vData.clear();
        if (v.isArray()) {
            int count = v.length();
            for (int n = 0; n < count; ++n) {
                Php::Value key = v.get(n);
                CComVariant vt;
                ToVariant(key, vt);
                vData.push_back(vt);
            }
        } else {
            CComVariant vt;
            ToVariant(v, vt);
            vData.push_back(vt);
        }
    }

    void CPhpDataSet::ToArray(const Php::Value &keys, std::vector<CComVariant> &v) {
        v.clear();
        if (keys.isArray()) {
            int count = keys.length();
            for (int n = 0; n < count; ++n) {
                Php::Value key = keys.get(n);
                CComVariant vt;
                ToVariant(key, vt);
                v.push_back(vt);
            }
        } else {
            CComVariant vt;
            ToVariant(keys, vt);
            v.push_back(vt);
        }
    }

    Php::Value CPhpDataSet::DeleteARow(Php::Parameters & params) {
        std::wstring dbName = SPA::Utilities::ToWide(params[0].stringValue());
        std::wstring tableName = SPA::Utilities::ToWide(params[1].stringValue());
        Php::Value keys = params[2];
        std::vector<CComVariant> v;
        ToArray(keys, v);
        auto res = m_ds.DeleteARow(dbName.c_str(), tableName.c_str(), v.data(), (unsigned int) v.size());
        CheckResult(res);
        return (int64_t) res;
    }

    Php::Value CPhpDataSet::FindOrdinal(Php::Parameters & params) {
        std::wstring dbName = SPA::Utilities::ToWide(params[0].stringValue());
        std::wstring tableName = SPA::Utilities::ToWide(params[1].stringValue());
        std::wstring colName = SPA::Utilities::ToWide(params[2].stringValue());
        auto res = m_ds.FindOrdinal(dbName.c_str(), tableName.c_str(), colName.c_str());
        CheckResult(res);
        return (int64_t) res;
    }

    Php::Value CPhpDataSet::FindKeys(Php::Parameters & params) {
        int index = 0;
        Php::Array vArr;
        std::wstring dbName = SPA::Utilities::ToWide(params[0].stringValue());
        std::wstring tableName = SPA::Utilities::ToWide(params[1].stringValue());
        auto km = m_ds.FindKeys(dbName.c_str(), tableName.c_str());
        for (auto &p : km) {
            vArr.set(index, From(p.second));
            ++index;
        }
        return vArr;
    }

    Php::Value CPhpDataSet::GetColumMeta(Php::Parameters & params) {
        int index = 0;
        Php::Array vArr;
        std::wstring dbName = SPA::Utilities::ToWide(params[0].stringValue());
        std::wstring tableName = SPA::Utilities::ToWide(params[1].stringValue());
        auto meta = m_ds.GetColumMeta(dbName.c_str(), tableName.c_str());
        for (auto &col : meta) {
            vArr.set(index, From(col));
            ++index;
        }
        return vArr;
    }

    Php::Value CPhpDataSet::GetRowCount(Php::Parameters & params) {
        std::wstring dbName = SPA::Utilities::ToWide(params[0].stringValue());
        std::wstring tableName = SPA::Utilities::ToWide(params[1].stringValue());
        auto res = m_ds.GetRowCount(dbName.c_str(), tableName.c_str());
        CheckResult(res);
        return (int64_t) res;
    }

    Php::Value CPhpDataSet::GetColumnCount(Php::Parameters & params) {
        std::wstring dbName = SPA::Utilities::ToWide(params[0].stringValue());
        std::wstring tableName = SPA::Utilities::ToWide(params[1].stringValue());
        auto res = m_ds.GetColumnCount(dbName.c_str(), tableName.c_str());
        CheckResult(res);
        return (int64_t) res;
    }

    void CPhpDataSet::Empty() {
        m_ds.Empty();
    }

    Php::Value CPhpDataSet::__get(const Php::Value & name) {
        if (name == "DbTable" || name == "DBTablePair") {
            Php::Value map;
            int index = 0;
            auto dt = m_ds.GetDBTablePair();
            for (auto &p : dt) {
                std::string key = SPA::Utilities::ToUTF8(p.first);
                std::string val = SPA::Utilities::ToUTF8(p.second);
                Php::Value v;
                v.set(key, val);
                map.set(index, v);
                ++index;
            }
            return map;
        } else if (name == PHP_EMPTY || name == "IsEmpty") {
            return m_ds.IsEmpty();
        } else if (name == "DataCaseSensitive") {
            return m_ds.GetDataCaseSensitive();
        } else if (name == "DBMS" || name == "DBManagementSystem") {
            return m_ds.GetDBManagementSystem();
        } else if (name == "DBNameCaseSensitive") {
            return m_ds.GetDBNameCaseSensitive();
        } else if (name == "DBIp" || name == "DBServerIp") {
            return m_ds.GetDBServerIp();
        } else if (name == "DBServerName") {
            std::wstring s = m_ds.GetDBServerName();
            return SPA::Utilities::ToUTF8(s);
        } else if (name == "FieldCaseSensitive" || name == "FieldNameCaseSensitive") {
            return m_ds.GetFieldNameCaseSensitive();
        } else if (name == "TableCaseSensitive" || name == "TableNameCaseSensitive") {
            return m_ds.GetTableNameCaseSensitive();
        } else if (name == "Updater") {
            std::wstring s = m_ds.GetUpdater();
            return SPA::Utilities::ToUTF8(s);
        } else {
            return Php::Base::__get(name);
        }
    }
}
