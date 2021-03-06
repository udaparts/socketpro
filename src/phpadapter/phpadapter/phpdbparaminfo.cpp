#include "stdafx.h"
#include "phpdbparaminfo.h"

namespace PA
{

    CPhpDBParamInfo::CPhpDBParamInfo() {
    }

    void CPhpDBParamInfo::__destruct() {
    }

    void CPhpDBParamInfo::__construct(Php::Parameters & params) {
        int64_t n = params[0].numericValue();
        if (n < (int) SPA::UDB::tagParameterDirection::pdInput || n > (int) SPA::UDB::tagParameterDirection::pdReturnValue) {
            throw Php::Exception("Bad parameter direction value");
        }
        m_pi.Direction = (SPA::UDB::tagParameterDirection)n;
        n = params[1].numericValue();
        if (!Supported((VARTYPE) n)) {
            throw Php::Exception("Bad data type value");
        }
        m_pi.DataType = (VARTYPE) n;
        size_t args = params.size();
        if (args > 2) {
            n = params[2].numericValue();
            m_pi.ColumnSize = (unsigned int) n;
        }
        if (args > 3) {
            n = params[3].numericValue();
            if (n < 0 || n > 31) {
                throw Php::Exception("Bad precision value");
            }
            m_pi.Precision = (unsigned char) n;
        }
        if (args > 4) {
            n = params[4].numericValue();
            if (n < 0 || n > 31) {
                throw Php::Exception("Bad scale value");
            }
            m_pi.Scale = (unsigned char) n;
        }
        if (args > 5) {
            std::string s = params[5].stringValue();
            Trim(s);
            m_pi.ParameterName = SPA::Utilities::ToUTF16(s.c_str(), s.size());
        }
    }

    bool CPhpDBParamInfo::Supported(VARTYPE vt) {
        switch (vt) {
            case VT_I2:
            case VT_I1:
            case VT_I4:
            case VT_I8:
            case VT_INT:
            case VT_UI2:
            case VT_UI1:
            case VT_UI4:
            case VT_UI8:
            case VT_UINT:
            case VT_R4:
            case VT_R8:
            case VT_DATE:
            case VT_BSTR:
            case VT_BOOL:
            case VT_VARIANT:
            case VT_DECIMAL:
            case VT_CLSID:
            case SPA::VT_XML:
            case (VT_ARRAY | VT_I1):
            case (VT_ARRAY | VT_UI1):
                return true;
            default:
                break;
        }
        return false;
    }

    void CPhpDBParamInfo::RegisterInto(Php::Namespace & cs) {
        Php::Class<CPhpDBParamInfo> reg(PHP_DB_PARAMETER_INFO);
        reg.property("VT_EMPTY", VT_EMPTY, Php::Const);
        reg.property("VT_NULL", VT_NULL, Php::Const);
        reg.property("VT_I2", VT_I2, Php::Const);
        reg.property("VT_I4", VT_I4, Php::Const);
        reg.property("VT_R4", VT_R4, Php::Const);
        reg.property("VT_R8", VT_R8, Php::Const);
        reg.property("VT_CY", VT_CY, Php::Const);
        reg.property("VT_DATE", VT_DATE, Php::Const);
        reg.property("VT_BSTR", VT_BSTR, Php::Const);
        reg.property("VT_BOOL", VT_BOOL, Php::Const);
        reg.property("VT_VARIANT", VT_VARIANT, Php::Const);
        reg.property("VT_DECIMAL", VT_DECIMAL, Php::Const);
        reg.property("VT_I1", VT_I1, Php::Const);
        reg.property("VT_UI1", VT_UI1, Php::Const);
        reg.property("VT_UI2", VT_UI2, Php::Const);
        reg.property("VT_UI4", VT_UI4, Php::Const);
        reg.property("VT_I8", VT_I8, Php::Const);
        reg.property("VT_UI8", VT_UI8, Php::Const);
        reg.property("VT_INT", VT_INT, Php::Const);
        reg.property("VT_UINT", VT_UINT, Php::Const);
        reg.property("VT_CLSID", VT_CLSID, Php::Const);
        reg.property("VT_ARRAY", VT_ARRAY, Php::Const);
        reg.property("VT_XML", SPA::VT_XML, Php::Const);

        //tagParameterDirection
        reg.property("pdUnknown", (int) SPA::UDB::tagParameterDirection::pdUnknown, Php::Const);
        reg.property("pdInput", (int) SPA::UDB::tagParameterDirection::pdInput, Php::Const);
        reg.property("pdOutput", (int) SPA::UDB::tagParameterDirection::pdOutput, Php::Const);
        reg.property("pdInputOutput", (int) SPA::UDB::tagParameterDirection::pdInputOutput, Php::Const);
        reg.property("pdReturnValue", (int) SPA::UDB::tagParameterDirection::pdReturnValue, Php::Const);

        reg.method<&CPhpDBParamInfo::__construct>(PHP_CONSTRUCT,{
            Php::ByVal("direction", Php::Type::Numeric),
            Php::ByVal("dataType", Php::Type::Numeric),
            Php::ByVal("columnSize", Php::Type::Numeric, false),
            Php::ByVal("precision", Php::Type::Numeric, false),
            Php::ByVal("scale", Php::Type::Numeric, false),
            Php::ByVal("name", Php::Type::String, false)
        });
        cs.add(reg);
    }

    Php::Value CPhpDBParamInfo::__get(const Php::Value & name) {
        if (name == "Direction") {
            return (int) m_pi.Direction;
        } else if (name == PHP_DATATYPE) {
            return m_pi.DataType;
        } else if (name == PHP_COLUMN_SIZE) {
            return (int64_t) m_pi.ColumnSize;
        } else if (name == PHP_COLUMN_PRECSISON) {
            return m_pi.Precision;
        } else if (name == PHP_COLUMN_SCALE) {
            return m_pi.Scale;
        } else if (name == "Name" || name == "ParameterName") {
            std::string s = SPA::Utilities::ToUTF8(m_pi.ParameterName.c_str(), m_pi.ParameterName.size());
            return s;
        }
        return Php::Base::__get(name);
    }

} //namespace PA
