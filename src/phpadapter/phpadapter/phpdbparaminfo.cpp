#include "stdafx.h"
#include "phpdbparaminfo.h"

namespace PA {

	CPhpDBParamInfo::CPhpDBParamInfo() {
	}

	void CPhpDBParamInfo::__construct(Php::Parameters &params) {
		int64_t n = params[0].numericValue();
		if (n < SPA::UDB::pdInput || n > SPA::UDB::pdReturnValue) {
			throw Php::Exception("Bad parameter direction value");
		}
		m_pi.Direction = (SPA::UDB::tagParameterDirection)n;
		n = params[1].numericValue();
		if ((n > VT_NULL && n <= VT_BSTR) || n == SPA::VT_XML || n == VT_BOOL || n == VT_VARIANT || (n >= VT_DECIMAL && n <= VT_UINT)) {	
		}
		else {
			throw Php::Exception("Bad data type value");
		}
		m_pi.DataType = (VARTYPE)n;
		size_t args = params.size();
		if (args > 2) {
			n = params[2].numericValue();
			m_pi.ColumnSize = (unsigned int)n;
		}
		if (args > 3) {
			n = params[3].numericValue();
			if (n < 0 || n > 31) {
				throw Php::Exception("Bad precision value");
			}
			m_pi.Precision = (unsigned char)n;
		}
		if (args > 4) {
			n = params[4].numericValue();
			if (n < 0 || n > 31) {
				throw Php::Exception("Bad scale value");
			}
			m_pi.Scale = (unsigned char)n;
		}
		if (args > 5) {
			std::string s = params[5].stringValue();
			Trim(s);
			m_pi.ParameterName = SPA::Utilities::ToWide(s.c_str(), s.size());
		}
	}

	void CPhpDBParamInfo::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpDBParamInfo> reg("CParamInfo");
		reg.method(PHP_CONSTRUCT, &CPhpDBParamInfo::__construct, {
			Php::ByVal("direction", Php::Type::Numeric),
			Php::ByVal("dataType", Php::Type::Numeric),
			Php::ByVal("columnSize", Php::Type::Numeric, false),
			Php::ByVal("precision", Php::Type::Numeric, false),
			Php::ByVal("scale", Php::Type::Numeric, false),
			Php::ByVal("name", Php::Type::String, false)
		});
		cs.add(reg);
	}

	Php::Value CPhpDBParamInfo::__get(const Php::Value &name) {
		if (name == "Direction") {
			return m_pi.Direction;
		}
		else if (name == "DataType") {
			return m_pi.DataType;
		}
		else if (name == "ColumnSize") {
			return (int64_t)m_pi.ColumnSize;
		}
		else if (name == "Precision") {
			return m_pi.Precision;
		}
		else if (name == "Scale") {
			return m_pi.Scale;
		}
		else if (name == "ParameterName") {
			std::string s = SPA::Utilities::ToUTF8(m_pi.ParameterName.c_str(), m_pi.ParameterName.size());
			return s;
		}
		return Php::Base::__get(name);
	}

} //namespace PA
