#include "stdafx.h"
#include "phpdataset.h"
#include "phptable.h"

namespace PA {

	CPhpDataSet::CPhpDataSet(SPA::CDataSet &ds) : m_ds(ds) {
	}

	void CPhpDataSet::__construct(Php::Parameters &params) {
	}

	void CPhpDataSet::RegisterInto(Php::Namespace &spa) {
		Php::Class<CPhpDataSet> ds(PHP_DATASET);
		ds.method(PHP_CONSTRUCT, &CPhpDataSet::__construct, Php::Private);

		spa.add(ds);
	}

}
