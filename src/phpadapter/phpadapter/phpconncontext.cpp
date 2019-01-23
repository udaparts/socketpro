#include "stdafx.h"
#include "phpconncontext.h"
#include "phpbuffer.h"

namespace PA {

	CPhpConnContext::CPhpConnContext() {
	}

	void CPhpConnContext::__construct(Php::Parameters &params) {
		size_t args = params.size();
		if (args > 7) {
			ToVariant(params[7], AnyData);
		}
		if (args > 6) {
			V6 = params[6].boolValue();
		}
		if (args > 5) {
			Zip = params[5].boolValue();
		}
		if (args > 4) {
			int64_t em = params[4].numericValue();
			if (em < 0 || em > SPA::tagEncryptionMethod::TLSv1) {
				throw Php::Exception("Bad encryption method value");
			}
			EncrytionMethod = (SPA::tagEncryptionMethod)em;
		}
		Host = params[0].stringValue();
		Port = (unsigned int)params[1].numericValue();
		std::string s = params[2].stringValue();
		UserId = SPA::Utilities::ToWide(s.c_str(), s.size());
		s = params[3].stringValue();
		Password = SPA::Utilities::ToWide(s.c_str(), s.size());
	}

	Php::Value CPhpConnContext::__get(const Php::Value &name) {
		if (name == "Host") {
			return Host;
		}
		else if (name == "Port") {
			return (int64_t)Port;
		}
		else if (name == "UserId") {
			return SPA::Utilities::ToUTF8(UserId.c_str(), UserId.size());
		}
		else if (name == "Pwd" || name == "Password") {
			return SPA::Utilities::ToUTF8(Password.c_str(), Password.size());
		}
		else if (name == "EM" || name == "EncrytionMethod") {
			return (int64_t)EncrytionMethod;
		}
		else if (name == "Zip") {
			return Zip;
		}
		else if (name == "V6") {
			return V6;
		}
		else if (name == "Any" || name == "AnyData") {
			SPA::CScopeUQueue sb;
			if (AnyData.vt == VT_DATE) {
				sb << AnyData.vt << AnyData.ullVal;
			}
			else {
				sb << AnyData;
			}
			CPhpBuffer buff(sb.Get());
			return buff.LoadObject();
		}
		return Php::Base::__get(name);
	}

	void CPhpConnContext::__set(const Php::Value &name, const Php::Value &value) {
		if (name == "Host") {
			Host = value.stringValue();
		}
		else if (name == "Port") {
			Port = (unsigned int)value.numericValue();
		}
		else if (name == "UserId") {
			std::string s = value.stringValue();
			UserId = SPA::Utilities::ToWide(s.c_str(), s.size());
		}
		else if (name == "Pwd" || name == "Password") {
			std::string s = value.stringValue();
			Password = SPA::Utilities::ToWide(s.c_str(), s.size());
		}
		else if (name == "EM" || name == "EncrytionMethod") {
			int64_t em = value.numericValue();
			if (em < 0 || em > SPA::tagEncryptionMethod::TLSv1) {
				throw Php::Exception("Bad encryption method value");
			}
			EncrytionMethod = (SPA::tagEncryptionMethod)em;
		}
		else if (name == "Zip") {
			Zip = value.boolValue();
		}
		else if (name == "V6") {
			V6 = value.boolValue();
		}
		else if (name == "Any" || name == "AnyData") {
			ToVariant(value, AnyData);
		}
		else {
			Php::Base::__set(name, value);
		}
	}

	void CPhpConnContext::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpConnContext> cc(PHP_CONN_CONTEXT);
		cc.method(PHP_CONSTRUCT, &CPhpConnContext::__construct, {
			Php::ByVal("host", Php::Type::String),
			Php::ByVal("port", Php::Type::Numeric),
			Php::ByVal("userId", Php::Type::String),
			Php::ByVal("password", Php::Type::String),
			Php::ByVal("em", Php::Type::Numeric, false),
			Php::ByVal("zip", Php::Type::Bool, false),
			Php::ByVal("v6", Php::Type::Bool, false),
			Php::ByVal("anyData", Php::Type::Null, false)
		});
		cs.add(cc);
	}

} //namespace PA