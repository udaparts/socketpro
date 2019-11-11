
#include "stdafx.h"
#include "phpdbcolumninfo.h"

namespace PA
{

    Php::Value From(const SPA::UDB::CDBColumnInfo & ColInfo) {
        Php::Value col;
#ifdef WIN32_64
        col.set("DBPath", SPA::Utilities::ToUTF8(ColInfo.DBPath));
        col.set("TablePath", SPA::Utilities::ToUTF8(ColInfo.TablePath));
        col.set("DisplayName", SPA::Utilities::ToUTF8(ColInfo.DisplayName));
        col.set("OriginalName", SPA::Utilities::ToUTF8(ColInfo.OriginalName));
        col.set("DeclaredType", SPA::Utilities::ToUTF8(ColInfo.DeclaredType));
        col.set("Collation", SPA::Utilities::ToUTF8(ColInfo.Collation));
#else
        col.set("DBPath", SPA::Utilities::ToUTF8(ColInfo.DBPath.c_str(), ColInfo.DBPath.size()));
        col.set("TablePath", SPA::Utilities::ToUTF8(ColInfo.TablePath.c_str(), ColInfo.TablePath.size()));
        col.set("DisplayName", SPA::Utilities::ToUTF8(ColInfo.DisplayName.c_str(), ColInfo.DisplayName.size()));
        col.set("OriginalName", SPA::Utilities::ToUTF8(ColInfo.OriginalName.c_str(), ColInfo.OriginalName.size()));
        col.set("DeclaredType", SPA::Utilities::ToUTF8(ColInfo.DeclaredType.c_str(), ColInfo.DeclaredType.size()));
        col.set("Collation", SPA::Utilities::ToUTF8(ColInfo.Collation.c_str(), ColInfo.Collation.size()));
#endif
        col.set(PHP_COLUMN_SIZE, (int64_t) ColInfo.ColumnSize);
        col.set(PHP_COLUMN_FLAGS, (int64_t) ColInfo.Flags);
        col.set(PHP_DATATYPE, ColInfo.DataType);
        col.set(PHP_COLUMN_PRECSISON, ColInfo.Precision);
        col.set(PHP_COLUMN_SCALE, ColInfo.Scale);
        return col;
    }
}
