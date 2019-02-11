
#ifndef SPA_PHP_DATA_SET_H
#define SPA_PHP_DATA_SET_H

namespace PA {

	class CPhpDataSet : public Php::Base {
	public:
		CPhpDataSet(SPA::CDataSet &ds);
		CPhpDataSet(const CPhpDataSet &ds) = delete;

	public:
		CPhpDataSet& operator=(const CPhpDataSet &ds) = delete;
		void __construct(Php::Parameters &params);
		static void RegisterInto(Php::Namespace &spa);

	private:
		SPA::CDataSet &m_ds;
	};

} //namespace PA

#endif