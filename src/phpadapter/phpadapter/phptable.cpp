#include "stdafx.h"
#include "phptable.h"
#include "phpdbcolumninfo.h"
#include "phpbuffer.h"
#include "phpdataset.h"

namespace PA
{

    CPhpTable::CPhpTable(std::shared_ptr<SPA::CTable> table) : m_table(table) {
    }

    void CPhpTable::__construct(Php::Parameters & params) {
    }

    void CPhpTable::__destruct() {
    }

    Php::Value CPhpTable::__get(const Php::Value & name) {
        if (name == "Meta") {
            int index = 0;
            Php::Array vArr;
            auto meta = m_table->GetMeta();
            for (auto &col : meta) {
                vArr.set(index, From(col));
                ++index;
            }
            return vArr;
        } else if (name == "Keys") {
            int index = 0;
            Php::Array vArr;
            auto km = m_table->GetKeys();
            for (auto &p : km) {
                vArr.set(index, From(p.second));
                ++index;
            }
            return vArr;
        } else if (name == "Cols" || name == "ColumnCount") {
            return (int64_t) m_table->GetMeta().size();
        } else if (name == "Rows" || name == "RowCount") {
            return (int64_t) m_table->GetDataMatrix().size();
        } else if (name == "Data" || name == "DataMatrix") {
            int row = 0;
            Php::Array dm;
            CPhpBuffer buff;
            auto &mat = m_table->GetDataMatrix();
            for (auto pr : mat) {
                int col = 0;
                Php::Array ar;
                for (auto &one : *pr) {
                    *buff.GetBuffer() << one;
                    ar.set(col, buff.LoadObject());
                    ++col;
                }
                dm.set(row, ar);
                ++row;
            }
            return dm;
        } else if (name == PHP_POINTER_ADDRESS) {
            return (int64_t) m_table.get();
        } else {
            return Php::Base::__get(name);
        }
    }

    void CPhpTable::RegisterInto(Php::Namespace & spa) {
        Php::Class<CPhpTable> table(PHP_TABLE);
        table.method<&CPhpTable::__construct>(PHP_CONSTRUCT, Php::Private);

        //SPA::CTable::Operator
        table.property("equal", (int)SPA::CTable::Operator::equal, Php::Const);
        table.property("great", (int)SPA::CTable::Operator::great, Php::Const);
        table.property("less", (int)SPA::CTable::Operator::less, Php::Const);
        table.property("great_equal", (int)SPA::CTable::Operator::great_equal, Php::Const);
        table.property("less_equal", (int)SPA::CTable::Operator::less_equal, Php::Const);
        table.property("not_equal", (int)SPA::CTable::Operator::not_equal, Php::Const);
        table.property("is_null", (int)SPA::CTable::Operator::is_null, Php::Const);

        table.method<&CPhpTable::Find>("Find",{
            Php::ByVal(PHP_ORDINAL, Php::Type::Numeric),
            Php::ByVal(PHP_TABLE_OP, Php::Type::Numeric),
            Php::ByVal(PHP_VARIANT_V, Php::Type::Null),
            Php::ByVal(PHP_COPYDATA, Php::Type::Bool, false)
        });
        table.method<&CPhpTable::In>("In",{
            Php::ByVal(PHP_ORDINAL, Php::Type::Numeric),
            Php::ByVal(PHP_VARIANT_V, Php::Type::Null),
            Php::ByVal(PHP_COPYDATA, Php::Type::Bool, false)
        });
        table.method<&CPhpTable::NotIn>("NotIn",{
            Php::ByVal(PHP_ORDINAL, Php::Type::Numeric),
            Php::ByVal(PHP_VARIANT_V, Php::Type::Null),
            Php::ByVal(PHP_COPYDATA, Php::Type::Bool, false)
        });
        table.method<&CPhpTable::Between>("Between",{
            Php::ByVal(PHP_ORDINAL, Php::Type::Numeric),
            Php::ByVal(PHP_VARIANT_V0, Php::Type::Null),
            Php::ByVal(PHP_VARIANT_V1, Php::Type::Null),
            Php::ByVal(PHP_COPYDATA, Php::Type::Bool, false)
        });
        table.method<&CPhpTable::FindNull>("FindNull",{
            Php::ByVal(PHP_ORDINAL, Php::Type::Numeric),
            Php::ByVal(PHP_COPYDATA, Php::Type::Bool, false)
        });
        table.method<&CPhpTable::FindOrdinal>("FindOrdinal",{
            Php::ByVal(PHP_COLUMN_NAME, Php::Type::String)
        });
        table.method<&CPhpTable::Sort>("Sort",{
            Php::ByVal(PHP_ORDINAL, Php::Type::Numeric),
            Php::ByVal("desc", Php::Type::Bool, false)
        });
        table.method<&CPhpTable::Append>("Append",{
            Php::ByVal("table", Php::Type::Object)
        });
        spa.add(table);
    }

    Php::Value CPhpTable::Append(Php::Parameters & params) {
        const Php::Value &tbl = params[0];
        if (!tbl.instanceOf(SPA_NS + PHP_TABLE)) {
            throw Php::Exception("A CTable object expected");
        }
        SPA::CTable *pTable = (SPA::CTable*)tbl.get(PHP_POINTER_ADDRESS).numericValue();
        auto res = m_table->Append(*pTable);
        CPhpDataSet::CheckResult((size_t) res);
        return res;
    }

    Php::Value CPhpTable::Sort(Php::Parameters & params) {
        bool desc = false;
        unsigned int ordinal = (unsigned int) params[0].numericValue();
        if (params.size() > 1) {
            desc = params[1].boolValue();
        }
        auto res = m_table->Sort(ordinal, desc);
        CPhpDataSet::CheckResult((size_t) res);
        return res;
    }

    Php::Value CPhpTable::FindOrdinal(Php::Parameters & params) {
        SPA::CDBString colName = SPA::Utilities::ToUTF16(params[0].stringValue().c_str());
        auto res = m_table->FindOrdinal(colName.c_str());
        CPhpDataSet::CheckResult(res);
        return (int64_t) res;
    }

    Php::Value CPhpTable::Between(Php::Parameters & params) {
        bool copyData = false;
        unsigned int ordinal = (unsigned int) params[0].numericValue();
        SPA::UDB::CDBVariant vt0, vt1;
        ToVariant(params[1], vt0);
        ToVariant(params[2], vt1);
        if (params.size() > 3) {
            copyData = params[3].boolValue();
        }
        std::shared_ptr<SPA::CTable> pTable(new SPA::CTable);
        auto res = m_table->Between(ordinal, vt0, vt1, *pTable, copyData);
        CPhpDataSet::CheckResult((size_t) res);
        return Php::Object((SPA_NS + PHP_TABLE).c_str(), new CPhpTable(pTable));
    }

    Php::Value CPhpTable::FindNull(Php::Parameters & params) {
        bool copyData = false;
        unsigned int ordinal = (unsigned int) params[0].numericValue();
        if (params.size() > 1) {
            copyData = params[1].boolValue();
        }
        std::shared_ptr<SPA::CTable> pTable(new SPA::CTable);
        auto res = m_table->FindNull(ordinal, *pTable, copyData);
        CPhpDataSet::CheckResult((size_t) res);
        return Php::Object((SPA_NS + PHP_TABLE).c_str(), new CPhpTable(pTable));
    }

    Php::Value CPhpTable::In(Php::Parameters & params) {
        bool copyData = false;
        unsigned int ordinal = (unsigned int) params[0].numericValue();
        SPA::UDB::CDBVariantArray v;
        CPhpDataSet::ToArray(params[1], v);
        if (params.size() > 2) {
            copyData = params[2].boolValue();
        }
        std::shared_ptr<SPA::CTable> pTable(new SPA::CTable);
        auto res = m_table->In(ordinal, v, *pTable, copyData);
        CPhpDataSet::CheckResult((size_t) res);
        return Php::Object((SPA_NS + PHP_TABLE).c_str(), new CPhpTable(pTable));
    }

    Php::Value CPhpTable::NotIn(Php::Parameters & params) {
        bool copyData = false;
        unsigned int ordinal = (unsigned int) params[0].numericValue();
        SPA::UDB::CDBVariantArray v;
        CPhpDataSet::ToArray(params[1], v);
        if (params.size() > 2) {
            copyData = params[2].boolValue();
        }
        std::shared_ptr<SPA::CTable> pTable(new SPA::CTable);
        auto res = m_table->NotIn(ordinal, v, *pTable, copyData);
        CPhpDataSet::CheckResult((size_t) res);
        return Php::Object((SPA_NS + PHP_TABLE).c_str(), new CPhpTable(pTable));
    }

    Php::Value CPhpTable::Find(Php::Parameters & params) {
        bool copyData = false;
        unsigned int ordinal = (unsigned int) params[0].numericValue();
        int64_t n = params[1].numericValue();
        if (n < (int)SPA::CTable::Operator::equal || n > (int)SPA::CTable::Operator::is_null) {
            throw Php::Exception("Bad operation value");
        }
        SPA::CTable::Operator op = (SPA::CTable::Operator)n;
        SPA::UDB::CDBVariant vt;
        ToVariant(params[2], vt);
        if (params.size() > 3) {
            copyData = params[3].boolValue();
        }
        std::shared_ptr<SPA::CTable> pTable(new SPA::CTable);
        auto res = m_table->Find(ordinal, op, vt, *pTable, copyData);
        CPhpDataSet::CheckResult((size_t) res);
        return Php::Object((SPA_NS + PHP_TABLE).c_str(), new CPhpTable(pTable));
    }
}
