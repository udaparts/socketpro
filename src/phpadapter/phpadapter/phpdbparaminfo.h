#ifndef _PHP_DB_PARAMETER_INFO_H_
#define _PHP_DB_PARAMETER_INFO_H_

namespace PA {

	class CPhpDBParamInfo : public Php::Base {
	public:
		CPhpDBParamInfo();
		CPhpDBParamInfo(const CPhpDBParamInfo &pi) = delete;

	public:
		CPhpDBParamInfo& operator=(const CPhpDBParamInfo &pi) = delete;
		static void RegisterInto(Php::Namespace &cs);
		Php::Value __get(const Php::Value &name);
		void __destruct();
		static bool Supported(VARTYPE vt);

	private:
		void __construct(Php::Parameters &params);

	private:
		SPA::UDB::CParameterInfo m_pi;
	};

} //namespace PA

#endif
