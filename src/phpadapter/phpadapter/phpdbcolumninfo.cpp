
#include "stdafx.h"
#include "phpdbcolumninfo.h"

namespace PA
{
	Php::Value From(const SPA::UDB::CDBColumnInfo &ColInfo) {
		Php::Value col;
		col.set("DBPath", SPA::Utilities::ToUTF8(ColInfo.DBPath));
		col.set("TablePath", SPA::Utilities::ToUTF8(ColInfo.TablePath));
		col.set("DisplayName", SPA::Utilities::ToUTF8(ColInfo.DisplayName));
		col.set("OriginalName", SPA::Utilities::ToUTF8(ColInfo.OriginalName));
		col.set("DeclaredType", SPA::Utilities::ToUTF8(ColInfo.DeclaredType));
		col.set("Collation", SPA::Utilities::ToUTF8(ColInfo.Collation));
		col.set(PHP_COLUMN_SIZE, (int64_t)ColInfo.ColumnSize);
		col.set(PHP_COLUMN_FLAGS, (int64_t)ColInfo.Flags);
		col.set(PHP_DATATYPE, ColInfo.DataType);
		col.set(PHP_COLUMN_PRECSISON, ColInfo.Precision);
		col.set(PHP_COLUMN_SCALE, ColInfo.Scale);
		return col;
	}
}
