#include "stdafx.h"
#include "phptable.h"

namespace PA {

	CPhpTable::CPhpTable(SPA::CTable &table) : m_table(table) {
	}

	void CPhpTable::__construct(Php::Parameters &params) {
	}

	void CPhpTable::RegisterInto(Php::Namespace &spa) {
		Php::Class<CPhpTable> table(PHP_TABLE);
		table.method(PHP_CONSTRUCT, &CPhpTable::__construct, Php::Private);

		spa.add(table);
	}

}
