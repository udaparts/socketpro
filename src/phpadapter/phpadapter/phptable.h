#ifndef SPA_PHP_DATA_TABLE_H
#define SPA_PHP_DATA_TABLE_H

namespace PA {

	class CPhpTable : public Php::Base {
	public:
		CPhpTable(SPA::CTable &table);

	public:
		void __construct(Php::Parameters &params);
		static void RegisterInto(Php::Namespace &spa);

	private:
		SPA::CTable &m_table;
	};

} //namespace PA 

#endif