#include "StdAfx.h"
#include "method.h"

CMethod::CMethod(short sRequestID)
{
	m_bSlow = false;
	m_dtReturn = dtVoid;
	m_sReqestID = sRequestID;
	m_nOutput = 0;
}

CMethod::~CMethod()
{

}

CMethod::CMethod(const CMethod &Method)
{
	m_strDeclaration = Method.m_strDeclaration;
	m_strName = Method.m_strName;
	m_dtReturn = Method.m_dtReturn;
	m_bSlow = Method.m_bSlow;
	m_lang = Method.m_lang;
	m_sReqestID = Method.m_sReqestID;
	m_aParameter = Method.m_aParameter;
	m_strErrorMessage = Method.m_strErrorMessage;
	m_strReturnType = Method.m_strReturnType;
	m_strServiceName = Method.m_strServiceName;
	m_strRtnProcessing = Method.m_strRtnProcessing;
	m_strSynFun = Method.m_strSynFun;
	m_strDefClass = Method.m_strDefClass;
	m_nOutput = Method.m_nOutput;
}

CMethod& CMethod::operator=(const CMethod &Method)
{
	m_strDeclaration = Method.m_strDeclaration;
	m_strName = Method.m_strName;
	m_bSlow = Method.m_bSlow;
	m_lang = Method.m_lang;
	m_dtReturn = Method.m_dtReturn;
	m_sReqestID = Method.m_sReqestID;
	m_aParameter = Method.m_aParameter;
	m_strErrorMessage = Method.m_strErrorMessage;
	m_strReturnType = Method.m_strReturnType;
	m_strServiceName = Method.m_strServiceName;
	m_strRtnProcessing = Method.m_strRtnProcessing;
	m_strSynFun = Method.m_strSynFun;
	m_strDefClass = Method.m_strDefClass;
	m_nOutput = Method.m_nOutput;
	return *this;
}

bool CMethod::operator==(const CMethod &Method)
{
	if(m_lang == lVBNet)
	{
		//VB.NET case-insensitive
		if(m_strName.CompareNoCase(Method.m_strName) != 0)
			return false;
	}
	else
	{
		if(m_strName != Method.m_strName)
			return false;
	}
	
	return true; //Two methods share the same name

	if(m_aParameter.GetSize() != Method.m_aParameter.GetSize())
		return false;

	int n;
	for(n=0; n<m_aParameter.GetSize(); n++)
	{
		if(m_aParameter[n].m_dt != Method.m_aParameter[n].m_dt)
			return false;
	}

	return true;
}

bool CMethod::operator!=(const CMethod &Method)
{
	return (*this == Method) ? false : true;
}

bool CMethod::IsValid(const CString &strName)
{
	CString strNum(_T("0123456789_"));
	CString strFirst(_T("qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM"));
	if(strName.GetLength() == 0)
		return false;
	TCHAR tChar = strName[0];
	if(strFirst.Find(tChar) == -1)
		return false;
	int n; 
	int nLen = strName.GetLength();
	for(n=1; n<nLen; n++)
	{
		tChar = strName[n];
		if(strFirst.Find(tChar) == -1 && strNum.Find(tChar) == -1)
		{
			return false;
		}
	}
	return true;
}

tagDataType CMethod::GetDataType(const CString &strType)
{
	if(strType.GetLength() == 0)
		return dtUnknown;
	if(strType == "string")
	{
		return dtString;
	}
	if(strType == "astring")
	{
		return dtAString;
	}
	else if(strType == "Guid")
	{
		return dtGuid;
	}
	else if(strType == "bool")
	{
		return dtBool;
	}
	else if(strType == "object")
	{
		return dtObject;
	}
	else if(strType == "decimal")
	{
		return dtDecimal;
	}
	else if(strType == "double")
	{
		return dtDouble;
	}
	else if(strType == "float")
	{
		return dtFloat;
	}
	else if(strType == "long")
	{
		return dtLong;
	}
	else if(strType == "ulong")
	{
		return dtULong;
	}
	else if(strType == "int")
	{
		return dtInt;
	}
	else if(strType == "uint")
	{
		return dtUInt;
	}
	else if(strType == "char")
	{
		return dtWChar;
	}
	else if(strType == "short")
	{
		return dtShort;
	}
	else if(strType == "ushort")
	{
		return dtUShort;
	}
	else if(strType == "byte")
	{
		return dtByte;
	}
	else if(strType == "sbyte")
	{
		return dtAChar;
	}
	else if(strType == "void")
	{
		return dtVoid;
	}
	else
	{
		if(IsValid(strType))
			return dtUDT;
	}

	return dtUnknown;
}

bool CMethod::SetDeclaration(LPCTSTR strDeclaration)
{
	m_aParameter.RemoveAll();
	m_strErrorMessage.Empty();
	CString strPType;
	CString strPName;
	CString strLine = strDeclaration;
	strLine.Trim(_T(' '));
	int nTwoFSlash = strLine.Find(_T("//"));
	if(nTwoFSlash != -1)
	{
		strLine = strLine.Left(nTwoFSlash);
		strLine.Trim(_T(' '));
	}
	
	int nDollar = strLine.Find(_T('$'));
	if(nDollar == 0)
	{
		m_bSlow = true;
		strLine = strLine.Mid(1);
	}
	else if(nDollar == -1)
	{
		m_bSlow = false;
	}
	else
	{
		m_strErrorMessage = _T("Position wrong for sign $"); 
		return false;
	}
	
	int nLeftP = strLine.Find(_T('('));
	if(nLeftP == -1)
	{
		m_strErrorMessage = _T("Left paratheis missing"); 
		return false;
	}
	
	int nRightP = strLine.Find(_T(')'));
	if(nRightP == -1)
	{
		m_strErrorMessage = _T("Right paratheis missing"); 
		return false;
	}

	if(nRightP <= nLeftP)
	{
		m_strErrorMessage = _T("Bad function in line %d"); 
		return false;
	}

	strLine.Trim(_T(' '));

	int nBlank = strLine.Find(_T(' '));
	if(nBlank == -1 || nBlank > nRightP)
	{
		m_strErrorMessage = _T("Can't get function return type in line %d"); 
		return false;
	}
	
	CString strRtn = strLine.Left(nBlank);
	strRtn.Trim(_T(' '));
	m_dtReturn = GetDataType(strRtn);
	if(m_dtReturn == dtUnknown)
	{
		m_strErrorMessage = _T("Function return type missing or invalid"); 
		return false;
	}
	if(m_dtReturn == dtUDT)
	{
		m_strReturnType = strRtn;
	}

	strLine = strLine.Mid(nBlank + 1);
	strLine.Trim(_T(' '));

	nLeftP = strLine.Find(_T('('));
	if(nLeftP <= 0)
	{
		m_strErrorMessage = _T("Function name missing"); 
		return false;
	}
	
	m_strName = strLine.Left(nLeftP);
	m_strName.Trim(_T(' '));
	if(m_strName.GetLength() == 0)
	{
		m_strErrorMessage = _T("Function name missing"); 
		return false;
	}
	if(!IsValid(m_strName))
	{
		m_strErrorMessage = _T("Function name invalid"); 
		return false;
	}

	strLine = strLine.Mid(nLeftP);
	strLine.Trim(_T(' '));
	
	nLeftP = strLine.Find(_T('('));
	nRightP = strLine.Find(_T(')'));
	if(nLeftP < 0 || nLeftP >= nRightP)
	{
		m_strErrorMessage = _T("Invalid function"); 
		return false;
	}
	strLine = strLine.Mid(1);
	strLine.Trim(_T(' '));
	int nComa = strLine.Find(_T(','));
	if(nComa >= nRightP)
	{
		m_strErrorMessage = _T("Invalid function"); 
		return false;
	}
	if(nComa == -1)
	{
		nComa = strLine.Find(_T(')'));
	}
	while(nComa != -1)
	{
		CParameter Parameter;
		::memset(&Parameter, 0, sizeof(Parameter));
		CString strParam = strLine.Left(nComa);
		strLine = strLine.Mid(nComa + 1);
		strLine.Trim(_T(' '));
		strParam.Trim(_T(' '));
		if(strParam.GetLength() == 0)
			break;
		if(strParam.Find(_T("in ")) == 0)
		{
			Parameter.m_pt = ptIn;
			strParam = strParam.Mid(3);
		}
		else if(strParam.Find(_T("out ")) == 0)
		{
			Parameter.m_pt = ptOut;
			strParam = strParam.Mid(4);
		}
		else if(strParam.Find(_T("inout ")) == 0)
		{
			/*Parameter.m_pt = ptInOut;
			strParam = strParam.Mid(6);*/
			m_strErrorMessage = _T("Parameter input direction inout not supported"); 
			return false;
		}
		else
		{
			//m_strErrorMessage = _T("Parameter input direction (in, inout or out) missing"); 
			Parameter.m_pt = ptIn;
		}
		strParam.Trim(_T(' '));
		nBlank = strParam.Find(_T(' '));
		if(nBlank == -1)
		{
			m_strErrorMessage = _T("Parameter type or name missing"); 
			return false;
		}
		
		strPType = strParam.Left(nBlank);
		strPName = strParam.Mid(nBlank + 1);
		strPType.Trim(_T(' '));
		strPName.Trim(_T(' '));
		
		Parameter.m_dt = GetDataType(strPType);
		if(Parameter.m_dt == dtUnknown)
		{
			m_strErrorMessage = _T("Parameter data type missing or invalid"); 
			return false;
		}

		if(Parameter.m_dt == dtVoid)
		{
			m_strErrorMessage = _T("Parameter data type can't be the type void"); 
			return false;
		}

		if(!IsValid(strPName))
		{
			m_strErrorMessage = _T("Parameter name missing or invalid"); 
			return false;
		}
		
		if(strPName.GetLength() > 255)
		{
			m_strErrorMessage = _T("Parameter name is too long"); 
			return false;
		}
		
		if(AlreadyExist(strPName))
		{
			m_strErrorMessage = _T("Two parameter names are same"); 
			return false;
		}

		if(strPType.GetLength() > 255)
		{
			m_strErrorMessage = _T("Parameter type is too long"); 
			return false;
		}
		::_tcscpy_s(Parameter.m_strType, strPType);
		::_tcscpy_s(Parameter.m_strName, strPName);
		m_aParameter.Add(Parameter);

		nComa = strLine.Find(_T(','));
		if(nComa >= nRightP)
		{
			m_strErrorMessage = _T("Invalid function"); 
			return false;
		}
		if(nComa == -1)
		{
			nComa = strLine.Find(_T(')'));
		}
	}
	strLine.Trim(_T(' '));
	if(strLine.GetLength() != 1 || strLine[0] != _T(';'))
	{
		m_strErrorMessage = _T("Invalid function or end char ';' missing"); 
		return false;
	}
	m_strDeclaration = strDeclaration;
	return true;
}

bool CMethod::AlreadyExist(const CString strParameterName)
{
	int n;
	for(n=0; n<m_aParameter.GetSize(); n++)
	{
		if(strParameterName.CompareNoCase(m_aParameter[n].m_strName) == 0)
		{
			return true;
		}
	}
	return false;
}


void CMethod::GetInputParameters(CSimpleArray<CParameter> &aParameter)
{
	int n;
	aParameter.RemoveAll();
	for(n=0; n<m_aParameter.GetSize(); n++)
	{
		if(m_aParameter[n].m_pt == ptIn || m_aParameter[n].m_pt == ptInOut)
		{
			aParameter.Add(m_aParameter[n]);
		}
	}
}

CString CMethod::GetMethodForPeerVBNet()
{
	int n;
	int nSize;
	CString strMethod(_T("\t<RequestAttr("));
	CString str;
	str.Format(_T("%s.id%s%s"), m_strDefClass, m_strName, m_strServiceName);
	strMethod += str;
	if(IsSlow())
	{
		strMethod += _T(", True");
	}

	strMethod += _T(")> ");
	tagDataType dt = GetReturn();
	if(dt == dtVoid)
	{
		strMethod += _T("Private Sub ");
	}
	else
	{
		strMethod += _T("Private Function ");
	}
	strMethod += GetName();
	strMethod += _T("(");
	nSize = m_aParameter.GetSize();
	for(n=0; n<nSize; n++)
	{
		CParameter &p = m_aParameter[n];
		if(n)
		{
			strMethod += _T(", ");
		}
		if(p.m_pt == ptIn)
		{
			strMethod += _T("ByVal ");
		}
		else
		{
			strMethod += _T("ByRef ");
		}

		strMethod += p.m_strName;
		strMethod += _T(" As ");

		CString strParam = GetInputString(p.m_dt);
		if(strParam == _T("UDT"))
		{
			strMethod += p.m_strType;
		}
		else
		{
			strMethod += strParam;
		}
	}

	if(dt != dtVoid)
	{
		
		strMethod += _T(") as ");

		CString strParam = GetInputString(dt);
		if(strParam == _T("UDT"))
		{
			strMethod += GetReturnType();
		}
		else
		{
			strMethod += strParam;
		}
		strMethod += _T("\n");
	}
	else
		strMethod += _T(")\n");

	strMethod += _T("\t\t' TODO: Implement this method\n");
	strMethod += _T("\t\tThrow New NotImplementedException()\n");

	if(dt == dtVoid)
		strMethod += _T("\tEnd Sub\n");
	else
		strMethod += _T("\tEnd Function\n");

	return strMethod;
}

CString CMethod::GetMethodForPeerCSharp()
{
	int n;
	int nSize;
	CString strMethod(_T("\t[RequestAttr("));
	CString str;
	str.Format(_T("%s.id%s%s"), m_strDefClass, m_strName, m_strServiceName);
	strMethod += str;
	if(IsSlow())
	{
		strMethod += _T(", true");
	}

	strMethod += _T(")]\r\n");
	strMethod += _T("\tprivate ");

	tagDataType dt = GetReturn();
	CString strParam = GetInputString(dt);
	if(strParam == _T("UDT"))
	{
		strMethod += GetReturnType();
	}
	else
	{
		strMethod += strParam;
	}
	strMethod += " ";

	strMethod += GetName();
	strMethod += _T("(");
	nSize = m_aParameter.GetSize();
	for(n=0; n<nSize; n++)
	{
		CParameter &p = m_aParameter[n];
		if(n)
		{
			strMethod += _T(", ");
		}
		if(p.m_pt == ptOut)
		{
			strMethod += _T("out ");
		}
		else if(p.m_pt == ptInOut)
		{
			strMethod += _T("ref ");
		}
		CString strParam = GetInputString(p.m_dt);
		if(strParam == _T("UDT"))
		{
			strMethod += p.m_strType;
		}
		else
		{
			strMethod += strParam;
		}
		strMethod += _T(" ");
		strMethod += p.m_strName;
	}

	strMethod += _T(")\r\n\t{\r\n");

	strMethod += _T("\t\t// TODO: Implement this method\n");
	strMethod += _T("\t\tthrow new NotImplementedException();\n");
	strMethod += _T("\t}\n");

	return strMethod;
}

CString CMethod::GetMethodForPeerCpp()
{
	int n;
	int nSize;
	CString strMethod(_T("\tvoid "));
	strMethod += GetName();
	strMethod += _T("(");
	nSize = m_aParameter.GetSize();
	for(n=0; n<nSize; n++)
	{
		CParameter &p = m_aParameter[n];
		if(n)
		{
			strMethod += _T(", ");
		}
		if(p.m_pt == ptOut)
		{
			strMethod += _T("/*out*/");
		}
		else if(p.m_pt == ptInOut)
		{
			strMethod += _T("/*inout*/");
		}
		CString strParam = GetInputString(p.m_dt);
		if(strParam == _T("UDT"))
		{
			if(p.m_pt == ptIn)
			{
				strMethod += _T("const ");
			}
			strMethod += p.m_strType;
			strMethod += _T(" &");
		}
		else if(strParam == _T("GUID"))
		{
			if(p.m_pt == ptIn)
			{
				strMethod += _T("const ");
			}
			strMethod += _T("GUID");
			strMethod += _T(" &");
		}
		else if(strParam == _T("UVariant"))
		{
			if(p.m_pt == ptIn)
			{
				strMethod += _T("const ");
			}
			strMethod += _T("UVariant");
			strMethod += _T(" &");
		}
		else if(strParam == _T("string"))
		{
			if(p.m_pt == ptIn)
			{
				strMethod += _T("const ");
			}
			strMethod += _T("std::wstring");
			strMethod += _T(" &");
		}
		else if(strParam == _T("sbyte[]"))
		{
			if(p.m_pt == ptIn)
			{
				strMethod += _T("const ");
			}
			strMethod += _T("std::string");
			strMethod += _T(" &");
		}
		else
		{
			strMethod += strParam;
			if(p.m_pt != ptIn)
			{
				strMethod += _T(" &");
			}
			else
			{
				strMethod += _T(" ");
			}
		}
		strMethod += p.m_strName;
	}

	tagDataType dt = GetReturn();
	if(dt != dtVoid)
	{
		if(nSize)
		{
			strMethod += _T(", ");
		}
		strMethod += _T("/*out*/");
		CString strParam = GetInputString(dt);
		if(strParam == _T("UDT"))
		{
			strMethod += GetReturnType();
		}
		else if(strParam == _T("GUID"))
		{
			strMethod += _T("GUID");
		}
		else if(strParam == _T("UVariant"))
		{
			strMethod += _T("UVariant");
		}
		else if(strParam == _T("string"))
		{
			strMethod += _T("std::wstring");
		}
		else if(strParam == _T("sbyte[]"))
		{
			strMethod += _T("std::string");
		}
		else
		{
			strMethod += strParam;
		}
		strMethod += _T(" &");
		strMethod += GetName();
		strMethod += _T("Rtn");
	}
	strMethod += _T(")\n");
	strMethod += _T("\t{\n\t\t// TODO: Implement this method\n\t}\n");
	return strMethod;
}


void CMethod::GetOutputParameters(CSimpleArray<CParameter> &aParameter)
{
	int n;
	aParameter.RemoveAll();
	for(n=0; n<m_aParameter.GetSize(); n++)
	{
		if(m_aParameter[n].m_pt == ptOut || m_aParameter[n].m_pt == ptInOut)
		{
			aParameter.Add(m_aParameter[n]);
		}
	}
}

CString CMethod::GetInputString(tagDataType dt)
{
	switch(dt)
	{
	case dtVoid:
		if(m_lang != lVBNet)
		{
			return _T("void");
		}
		break;
	case dtBool:
		if(m_lang == lVBNet)
		{
			return _T("Boolean");
		}
		return _T("bool");
		break;
	case dtByte:
		if(m_lang == lCpp)
			return _T("unsigned char");
		else if(m_lang == lCSharp)
			return _T("byte");
		else
			return _T("Byte");
		break;
	case dtAChar:
		if(m_lang == lCpp)
			return _T("char");
		else if(m_lang == lCSharp)
			return _T("sbyte");
		else
			return _T("SByte");
		break;
	case dtUShort:
		if(m_lang == lCpp)
			return _T("unsigned short");
		else if(m_lang == lCSharp)
			return _T("ushort");
		else
			return _T("UShort");
		break;
	case dtShort:
		if(m_lang == lVBNet)
			return _T("Short");
		return _T("short");
		break;
	case dtWChar: 
		if(m_lang == lCpp)
			return _T("wchar_t");
		return _T("char");
		break;
	case dtUInt:
		if(m_lang == lCpp)
			return _T("unsigned int");
		else if(m_lang == lCSharp)
			return _T("uint");
		else
			return _T("UInteger");
		break;
	case dtInt:
		if(m_lang == lVBNet)
		{
			return _T("Integer");
		}
		return _T("int");
		break;
	case dtULong:
		if(m_lang == lCpp)
			return _T("SPA::UINT64");
		else if(m_lang == lCSharp)
			return _T("ulong");
		else
			return _T("ULong");
		break;
	case dtLong:
		if(m_lang == lCpp)
			return _T("SPA::INT64");
		else if(m_lang == lCSharp)
			return _T("long");
		else
			return _T("Long");
		break;
	case dtFloat:
		if(m_lang == lVBNet)
			return _T("Single");
		return _T("float");
		break;
	case dtDouble:
		if(m_lang == lVBNet)
			return _T("Double");
		return _T("double");
		break;
	case dtDecimal:
		if(m_lang == lCpp)
			return _T("DECIMAL");
		else if(m_lang == lCSharp)
			return _T("decimal");
		else
			return _T("Decimal");
		break;
	case dtString:
		return _T("string");
		break;
	case dtAString:
		if(m_lang == lVBNet)
			return _T("SByte()");
		return _T("sbyte[]");
		break;
	case dtGuid:
		if(m_lang == lCpp)
			return _T("GUID");
		else
			return _T("Guid");
		break;
	case dtObject:
		if(m_lang == lCpp)
			return _T("UVariant");
		else if(m_lang == lCSharp)
			return _T("object");
		else 
			return _T("Object");
		break;
	default:
		break;
	}
	return _T("UDT");
}

CString	CMethod::GetPeerCallVBNetEx(int &nInput, int &nOutput)
{
	int n;
	int nSize;
	nInput = 0;
	nOutput = 0;
	CString strCallMethod;
	nSize = m_aParameter.GetSize();
	for(n=0; n<nSize; n++)
	{
		if(strCallMethod.GetLength() > 0)
			strCallMethod += _T(", ");
		CParameter &p = m_aParameter[n];
		CString strParam = GetInputString(p.m_dt);
		if(strParam == _T("UDT"))
		{
			strCallMethod += p.m_strType;
		}
		else if(strParam == _T("Guid"))
		{
			strCallMethod += _T("Guid");
		}
		else if(strParam == _T("Object"))
		{
			strCallMethod += _T("Object");
		}
		else if(strParam == _T("string"))
		{
			strCallMethod += _T("String");
		}
		else
		{
			strCallMethod += strParam;
		}
		if(p.m_pt == ptOut)
			nOutput++;
		else
			nInput++;
	}

	tagDataType dt = GetReturn();
	if(dt != dtVoid)
	{
		if(strCallMethod.GetLength() > 0)
			strCallMethod += _T(", ");
		CString strParam = GetInputString(dt);
		if(strParam == _T("UDT"))
		{
			strCallMethod += GetReturnType();
		}
		else
		{
			strCallMethod += strParam;
		}
		nOutput++;
	}

	return strCallMethod;
}

CString	CMethod::GetPeerCallVBNet()
{
	int n;
	int nSize;
	CString strCallMethod(_T("\t\t\t"));
	CString strPop;
	CString strPush(_T("\t\t\tSendResult(sRequestID"));
	CString strInstance;
	CString strPeerCall;
	strCallMethod += GetName();
	strCallMethod += _T("(");
	nSize = m_aParameter.GetSize();
	for(n=0; n<nSize; n++)
	{
		if(n)
		{
			strCallMethod += _T(", ");
		}
		CParameter &p = m_aParameter[n];
		CString strParam = GetInputString(p.m_dt);
		if(strParam == _T("UDT"))
		{
			strInstance.Format(_T("\t\t\tDim %s As %s = new %s()\n"), p.m_strName, p.m_strType, p.m_strType);
		}
		else if(strParam == _T("Guid"))
		{
			strInstance.Format(_T("\t\t\tDim %s As Guid = new Guid();\n"), p.m_strName);
		}
		else if(strParam == _T("Object"))
		{
			strInstance.Format(_T("\t\t\tDim %s As Object = Nothing\n"), p.m_strName);
		}
		else if(strParam == _T("string"))
		{
			strInstance.Format(_T("\t\t\tDim %s as String = Nothing\n"), p.m_strName);
		}
		else
		{
			strInstance.Format(_T("\t\t\tDim %s As %s\n"), p.m_strName, strParam);
		}
		strPeerCall += strInstance;
		if(p.m_pt != ptOut)
		{
			strPop += _T("\t\t\tm_UQueue.Load(");
			strPop += p.m_strName;
			if(p.m_dt == dtUDT)
			{
				strPop += _T(") 'Don't forget implementing IUSerializer\n");
			}
			else
			{
				strPop += _T(")\n");
			}
		}

		strCallMethod += p.m_strName;

		if(p.m_pt != ptIn)
		{
			strPush += _T(", ");
			strPush += p.m_strName;
		}
	}

	tagDataType dt = GetReturn();
	if(dt != dtVoid)
	{
		CString strRtn;
		strRtn.Format(_T("%sRtn"), GetName());
		CString strParam = GetInputString(dt);
		if(strParam == _T("UDT"))
		{
			strInstance.Format(_T("\t\t\tDim %s As %s = new %s()\n"), strRtn, GetReturnType(), GetReturnType());
		}
		else if(strParam == _T("Guid"))
		{
			strInstance.Format(_T("\t\t\tDim %s As Guid = new Guid()\n"), strRtn);
		}
		else if(strParam == _T("Object"))
		{
			strInstance.Format(_T("\t\t\tDim %s As Object = Nothing\n"), strRtn);
		}
		else if(strParam == _T("string"))
		{
			strInstance.Format(_T("\t\t\tDim %s As String = Nothing\n"), strRtn);
		}
		else
		{
			strInstance.Format(_T("\t\t\tDim %s As %s\n"), strRtn, strParam);
		}
		strPeerCall += strInstance;

		strPush += _T(", ");
		strPush += GetName();
		strPush += _T("Rtn");

		if(m_aParameter.GetSize())
		{
			strCallMethod += _T(", ");
		}
		strCallMethod += GetName();
		strCallMethod += _T("Rtn");
	}
	strCallMethod += _T(")\n");
	strPeerCall += strPop;
	strPeerCall += strCallMethod;
	strPeerCall += strPush;
	strPeerCall += _T(")\n");
	return strPeerCall;
}

CString	CMethod::GetPeerCallCSharpEx(int &nInput, int &nOutput)
{
	int n;
	int nSize;
	nInput = 0;
	nOutput = 0;
	CString strCallMethod;
	nSize = m_aParameter.GetSize();
	for(n=0; n<nSize; n++)
	{
		if(strCallMethod.GetLength() > 0)
			strCallMethod += _T(", ");
		CParameter &p = m_aParameter[n];
		CString strParam = GetInputString(p.m_dt);
		if(strParam == _T("UDT"))
		{
			strCallMethod += p.m_strType;
		}
		else if(strParam == _T("Guid"))
		{
			strCallMethod += _T("Guid");
		}
		else if(strParam == _T("object"))
		{
			strCallMethod += _T("object");
		}
		else if(strParam == _T("string"))
		{
			strCallMethod += _T("string");
		}
		else
		{
			strCallMethod += strParam;
		}
		if(p.m_pt == ptOut)
			nOutput++;
		else
			nInput++;
	}

	tagDataType dt = GetReturn();
	if(dt != dtVoid)
	{
		if(strCallMethod.GetLength() > 0)
			strCallMethod += _T(", ");
		CString strParam = GetInputString(dt);
		if(strParam == _T("UDT"))
		{
			strCallMethod += GetReturnType();
		}
		else
		{
			strCallMethod += strParam;
		}
		nOutput++;
	}

	return strCallMethod;
}

CString	CMethod::GetPeerCallCSharp()
{
	int n;
	int nSize;
	CString strCallMethod(_T("\t\t\t"));
	CString strPop;
	CString strPush(_T("\t\t\tSendResult(sRequestID"));
	CString strInstance;
	CString strPeerCall;
	strCallMethod += GetName();
	strCallMethod += _T("(");
	nSize = m_aParameter.GetSize();
	for(n=0; n<nSize; n++)
	{
		if(n)
		{
			strCallMethod += _T(", ");
		}
		CParameter &p = m_aParameter[n];
		CString strParam = GetInputString(p.m_dt);

		if(strParam == _T("UDT"))
		{
			/*if(p.m_pt == ptOut)
				strInstance.Format(_T("\t\t\t%s %s;\n"), p.m_strType, p.m_strName);
			else*/
				strInstance.Format(_T("\t\t\t%s %s = new %s();\n"), p.m_strType, p.m_strName, p.m_strType);
		}
		else if(strParam == _T("Guid"))
		{
			/*if(p.m_pt == ptOut)
				strInstance.Format(_T("\t\t\t%s %s;\n"), _T("Guid"), p.m_strName);
			else*/
				strInstance.Format(_T("\t\t\t%s %s = new Guid();\n"), _T("Guid"), p.m_strName);
		}
		else if(strParam == _T("object"))
		{
			/*if(p.m_pt == ptOut)
				strInstance.Format(_T("\t\t\t%s %s;\n"), _T("object"), p.m_strName);
			else*/
				strInstance.Format(_T("\t\t\t%s %s = null;\n"), _T("object"), p.m_strName);
		}
		else if(strParam == _T("string"))
		{
			/*if(p.m_pt == ptOut)
				strInstance.Format(_T("\t\t\t%s %s;\n"), _T("string"), p.m_strName);
			else*/
				strInstance.Format(_T("\t\t\t%s %s = null;\n"), _T("string"), p.m_strName);
		}
		else
		{
			/*if(p.m_pt == ptOut)
				strInstance.Format(_T("\t\t\t%s %s;\n"), strParam, p.m_strName);
			else */if(p.m_dt == dtBool)
				strInstance.Format(_T("\t\t\t%s %s = false;\n"), strParam, p.m_strName);
			else if(p.m_dt == dtWChar)
				strInstance.Format(_T("\t\t\t%s %s = (char)0;\n"), strParam, p.m_strName);
			else
				strInstance.Format(_T("\t\t\t%s %s = 0;\n"), strParam, p.m_strName);
		}
		strPeerCall += strInstance;
		if(p.m_pt != ptOut)
		{
			strPop += _T("\t\t\tm_UQueue.Load(out ");
			strPop += p.m_strName;
			if(p.m_dt == dtUDT)
			{
				strPop += _T("); //Don't forget implementing IUSerializer\n");
			}
			else
			{
				strPop += _T(");\n");
			}
		}

		if(p.m_pt == ptInOut)
			strCallMethod += _T("ref ");
		else if(p.m_pt == ptOut)
			strCallMethod += _T("out ");

		strCallMethod += p.m_strName;

		if(p.m_pt != ptIn)
		{
			strPush += _T(", ");
			strPush += p.m_strName;
		}
	}

	tagDataType dt = GetReturn();
	if(dt != dtVoid)
	{
		CString strRtn;
		strRtn.Format(_T("%sRtn"), GetName());
		CString strParam = GetInputString(dt);
		if(strParam == _T("UDT"))
		{
			strInstance.Format(_T("\t\t\t%s %s = new %s();\n"), GetReturnType(), strRtn, GetReturnType());
		}
		else if(strParam == _T("Guid"))
		{
			strInstance.Format(_T("\t\t\t%s %s = new Guid();\n"), _T("Guid"), strRtn);
		}
		else if(strParam == _T("object"))
		{
			strInstance.Format(_T("\t\t\t%s %s = null;\n"), _T("object"), strRtn);
		}
		else if(strParam == _T("string"))
		{
			strInstance.Format(_T("\t\t\t%s %s = null;\n"), _T("string"), strRtn);
		}
		else if(strParam == _T("bool"))
		{
			strInstance.Format(_T("\t\t\t%s %s = false;\n"), strParam, strRtn);
		}
		else
		{
			strInstance.Format(_T("\t\t\t%s %s = 0;\n"), strParam, strRtn);
		}
		strPeerCall += strInstance;
		strPush += _T(", ");
		strPush += GetName();
		if(strParam == _T("UDT"))
		{
			strPush += _T("Rtn/*Don't forget implementing IUSerializer*/");
		}
		else
		{
			strPush += _T("Rtn");
		}
		if(m_aParameter.GetSize())
		{
			strCallMethod += _T(", ");
		}
		strCallMethod += _T("out ");
		strCallMethod += GetName();
		strCallMethod += _T("Rtn");
	}
	strCallMethod += _T(");\n");
	strPeerCall += strPop;
	strPeerCall += strCallMethod;
	strPeerCall += strPush;
	strPeerCall += _T(");\n");

	return strPeerCall;
}

CString	CMethod::GetPeerCallCppEx(int &nInput, int &nOutput)
{
	int n;
	int nSize;
	nInput = 0;
	nOutput = 0;
	CString strCallMethod;
	nSize = m_aParameter.GetSize();
	for(n=0; n<nSize; n++)
	{
		if(strCallMethod.GetLength() > 0)
			strCallMethod += _T(", ");
		CParameter &p = m_aParameter[n];
		CString strParam = GetInputString(p.m_dt);
		if(strParam == _T("UDT"))
		{
			strCallMethod += p.m_strType;
		}
		else if(strParam == _T("GUID"))
		{
			strCallMethod += _T("GUID");
		}
		else if(strParam == _T("UVariant"))
		{
			strCallMethod += _T("UVariant");
		}
		else if(strParam == _T("sbyte[]"))
		{
			strCallMethod += _T("std::string");
		}
		else if(strParam == _T("string"))
		{
			strCallMethod += _T("std::wstring");
		}
		else
		{
			strCallMethod += strParam;
		}
		if(p.m_pt == ptOut)
			nOutput++;
		else
			nInput++;
	}

	tagDataType dt = GetReturn();
	if(dt != dtVoid)
	{
		if(strCallMethod.GetLength() > 0)
			strCallMethod += _T(", ");
		CString strParam = GetInputString(dt);
		if(strParam == _T("UDT"))
		{
			strCallMethod += GetReturnType();
		}
		else if(strParam == _T("GUID"))
		{
			strCallMethod += _T("GUID");
		}
		else if(strParam == _T("UVariant"))
		{
			strCallMethod += _T("UVariant");
		}
		else if(strParam == _T("sbyte[]"))
		{
			strCallMethod += _T("std::string");
		}
		else if(strParam == _T("string"))
		{
			strCallMethod += _T("std::wstring");
		}
		else
		{
			strCallMethod += strParam;
		}
		nOutput++;
	}
	return strCallMethod;
}

CString	CMethod::GetPeerCallCpp()
{
	int n;
	int nSize;
	CString strCallMethod(_T("\t\t\t"));
	CString strPop;
	CString strPush(_T("\t\t\tSendResult(reqId"));
	CString strInstance;
	CString strPeerCall;
	strCallMethod += GetName();
	strCallMethod += _T("(");
	nSize = m_aParameter.GetSize();
	for(n=0; n<nSize; n++)
	{
		if(n)
		{
			strCallMethod += _T(", ");
		}
		CParameter &p = m_aParameter[n];
		CString strParam = GetInputString(p.m_dt);
		if(strParam == _T("UDT"))
		{
			strInstance.Format(_T("\t\t\t%s %s;\n"), p.m_strType, p.m_strName);
		}
		else if(strParam == _T("GUID"))
		{
			strInstance.Format(_T("\t\t\t%s %s;\n"), _T("GUID"), p.m_strName);
		}
		else if(strParam == _T("UVariant"))
		{
			strInstance.Format(_T("\t\t\t%s %s;\n"), _T("UVariant"), p.m_strName);
		}
		else if(strParam == _T("sbyte[]"))
		{
			strInstance.Format(_T("\t\t\t%s %s;\n"), _T("std::string"), p.m_strName);
		}
		else if(strParam == _T("string"))
		{
			strInstance.Format(_T("\t\t\t%s %s;\n"), _T("std::wstring"), p.m_strName);
		}
		else
		{
			strInstance.Format(_T("\t\t\t%s %s;\n"), strParam, p.m_strName);
		}
		strPeerCall += strInstance;
		if(p.m_pt != ptOut)
		{
			strPop += _T("\t\t\tm_UQueue >> ");
			strPop += p.m_strName;
			if(p.m_dt == dtUDT)
			{
				strPop += _T("; //Don't forget implementing operators >> and <<\n");
			}
			else
			{
				strPop += _T(";\n");
			}
		}

		strCallMethod += p.m_strName;

		if(p.m_pt != ptIn)
		{
			strPush += _T(", ");
			strPush += p.m_strName;
			if(p.m_dt == dtUDT)
			{
				strPush += _T("/*Don't forget implementing operators >> and <<*/");
			}
		}
	}

	tagDataType dt = GetReturn();
	if(dt != dtVoid)
	{
		CString strRtn;
		strRtn.Format(_T("%sRtn"), GetName());
		CString strParam = GetInputString(dt);
		if(strParam == _T("UDT"))
		{
			strInstance.Format(_T("\t\t\t%s %s;\n"), GetReturnType(), strRtn);
		}
		else if(strParam == _T("GUID"))
		{
			strInstance.Format(_T("\t\t\t%s %s;\n"), _T("GUID"), strRtn);
		}
		else if(strParam == _T("UVariant"))
		{
			strInstance.Format(_T("\t\t\t%s %s;\n"), _T("UVariant"), strRtn);
		}
		else if(strParam == _T("sbyte[]"))
		{
			strInstance.Format(_T("\t\t\t%s %s;\n"), _T("std::string"), strRtn);
		}
		else if(strParam == _T("string"))
		{
			strInstance.Format(_T("\t\t\t%s %s;\n"), _T("std::wstring"), strRtn);
		}
		else
		{
			strInstance.Format(_T("\t\t\t%s %s;\n"), strParam, strRtn);
		}
		strPeerCall += strInstance;
		strPush += _T(", ");
		strPush += GetName();
		if(dt == dtUDT)
			strPush += _T("Rtn /*Don't forget implementing operators >> and <<*/");
		else
			strPush += _T("Rtn");
		if(m_aParameter.GetSize())
			strCallMethod += _T(", ");
		strCallMethod += GetName();
		strCallMethod += _T("Rtn");
	}
	strCallMethod += _T(");\n");
	strPeerCall += strPop;
	strPeerCall += strCallMethod;
	strPeerCall += strPush;
	strPeerCall += _T(");\n");

	return strPeerCall;
}

CString CMethod::GetClientAsynInputMethodVBNet()
{
	int n;
	int nSize;
	CString strTemp;
	CString strMethod;
	CString strParam;
	m_strRtnProcessing.Empty();
	CSimpleArray<CParameter> aOutParameter;
	CSimpleArray<CParameter> aParameter;

	GetOutputParameters(aOutParameter);
	nSize = aOutParameter.GetSize();
	m_nOutput = nSize;
	for(n=0; n<nSize; n++)
	{
		strMethod += _T("\tProtected ");
		strMethod += _T("m_");
		strMethod += aOutParameter[n].m_strName;
		strMethod += _T("_");
		strMethod += GetName();
		strMethod += _T(" As ");
		strParam = GetInputString(aOutParameter[n].m_dt);
		if(strParam == _T("UDT"))
		{
			strMethod += aOutParameter[n].m_strType;
			strMethod += _T(" = new ");
			strMethod += aOutParameter[n].m_strType;
			strMethod += _T("()");
		}
		else if(strParam == _T("Guid"))
		{
			strMethod += _T("Guid = new Guid()");
		}
		else if(strParam == _T("Object"))
		{
			strMethod += _T("Object");
		}
		else if(strParam == _T("string"))
		{
			strMethod += _T("String");
		}
		else
		{
			strMethod += strParam;
		}
		strMethod += _T("\n");
		
		m_strRtnProcessing += _T("\t\t\tUQueue.Load(");
		
		m_strRtnProcessing += _T("m_");
		m_strRtnProcessing += aOutParameter[n].m_strName;
		m_strRtnProcessing += _T("_");
		m_strRtnProcessing += GetName();
		if(aOutParameter[n].m_pt == ptOut && GetInputString(aOutParameter[n].m_dt) == _T("UDT"))
			m_strRtnProcessing += _T(") 'Don't forget implementing IUSerializer\n");
		else
			m_strRtnProcessing += _T(")\n");
	}
	
	tagDataType dt = GetReturn();
	if(dt != dtVoid)
	{
		strMethod += _T("\tProtected m_");
		strMethod += GetName();
		strMethod += _T("Rtn As ");
		strParam = GetInputString(dt);
		if(strParam == _T("UDT"))
		{
			strMethod += GetReturnType();
			strMethod += _T(" = new ");
			strMethod += GetReturnType();
			strMethod += _T("()");
		}
		else if(strParam == _T("Guid"))
		{
			strMethod += _T("Guid = new Guid()");
		}
		else if(strParam == _T("Object"))
		{
			strMethod += _T("Object");
		}
		else if(strParam == _T("string"))
		{
			strMethod += _T("String");
		}
		else
		{
			strMethod += strParam;
		}
		m_strRtnProcessing += _T("\t\t\tUQueue.Load(");
		m_strRtnProcessing += _T("m_");
		m_strRtnProcessing += GetName();
		if(dt == dtUDT)
		{
			m_strRtnProcessing += _T("Rtn) 'Don't forget implementing IUSerializer\n");
		}
		else
		{
			m_strRtnProcessing += _T("Rtn)\n");
		}
		strMethod += _T("\n");
	}

	GetInputParameters(aParameter);
	nSize = aParameter.GetSize();
	strMethod += (_T("\tProtected Sub "));
	strMethod += GetName();
	strMethod += _T("Async");
	strMethod += _T("(");

	for(n=0; n<nSize; n++)
	{
		if(n)
		{
			strMethod += _T(", ");
		}
		strParam = GetInputString(aParameter[n].m_dt);
		strMethod += _T("ByVal ");
		if(strParam == _T("UDT"))
		{
			strMethod += aParameter[n].m_strName;
			strMethod += _T(" AS ");
			strMethod += aParameter[n].m_strType;
		}
		else if(strParam == _T("Guid"))
		{
			strMethod += aParameter[n].m_strName;
			strMethod += _T(" As Guid");
		}
		else if(strParam == _T("Object"))
		{
			strMethod += aParameter[n].m_strName;
			strMethod += _T(" As Object");
		}
		else if(strParam == _T("string"))
		{
			strMethod += aParameter[n].m_strName;
			strMethod += _T(" As String");
		}
		else
		{
			strMethod += aParameter[n].m_strName;
			strMethod += _T(" As ");
			strMethod += strParam;
		}
	}
	strMethod += _T(")\n");
	CString strCase;
	strCase.Format(_T("\t\tcase %s.id%s%s\n"), m_strDefClass, GetName(), GetServiceName());
	m_strRtnProcessing = strCase + m_strRtnProcessing;
	CString strHead(_T("\t\t'make sure that the handler is attached to a client socket before calling the below statement\n"));
	if(nSize > 0) 
	{
		CString strComment;
		CString strP;
		for(n=0; n<nSize; n++)
		{
			strP += _T(", ");
			strP += aParameter[n].m_strName;
			if(aParameter[n].m_dt == dtUDT)
			{
				strComment += _T("'");
				strComment += aParameter[n].m_strName;
				strComment += _T(", Don't forget implementing IUSerializer");
			}
		}
		strTemp.Format(_T("\t\tSendRequest(%s.id%s%s%s) %s"), m_strDefClass, GetName(), GetServiceName(), strP, strComment);
	}
	else
	{
		strTemp.Format(_T("\t\tSendRequest(%s.id%s%s)"), m_strDefClass, GetName(), GetServiceName());
	}
	strTemp = strHead + strTemp;
	strMethod += strTemp;
	strMethod += _T("\n\tEnd Sub");	
	return strMethod;
}

CString CMethod::GetClientAsynInputMethodCSharp()
{
	int n;
	int nSize;
	CString strTemp;
	CString strMethod;
	CString strParam;
	m_strRtnProcessing.Empty();
	CSimpleArray<CParameter> aOutParameter;
	CSimpleArray<CParameter> aParameter;
	
	GetOutputParameters(aOutParameter);
	nSize = aOutParameter.GetSize();
	m_nOutput = nSize;
	for(n=0; n<nSize; n++)
	{
		strMethod += _T("protected ");
		strParam = GetInputString(aOutParameter[n].m_dt);
		if(strParam == _T("UDT"))
		{
			strMethod += aOutParameter[n].m_strType;
		}
		else if(strParam == _T("Guid"))
		{
			strMethod += _T("Guid");
		}
		else if(strParam == _T("object"))
		{
			strMethod += _T("object");
		}
		else if(strParam == _T("string"))
		{
			strMethod += _T("string");
		}
		else
		{
			strMethod += strParam;
		}
		
		m_strRtnProcessing += _T("\t\t\tUQueue.Load(out ");
		
		m_strRtnProcessing += _T("m_");
		m_strRtnProcessing += aOutParameter[n].m_strName;
		m_strRtnProcessing += _T("_");
		m_strRtnProcessing += GetName();
		
		if(aOutParameter[n].m_pt == ptOut && GetInputString(aOutParameter[n].m_dt) == _T("UDT"))
			m_strRtnProcessing += _T(");/* Don't forget implementing IUSerializer*/\n");
		else
			m_strRtnProcessing += _T(");\n");

		//m_strRtnProcessing += _T(");\n");

		strMethod += _T(" m_");
		strMethod += aOutParameter[n].m_strName;
		strMethod += _T("_");
		strMethod += GetName();
		strMethod += _T(";\n\t");
	}
	
	tagDataType dt = GetReturn();
	if(dt != dtVoid)
	{
		strMethod += _T("protected ");
		strParam = GetInputString(dt);
		if(strParam == _T("UDT"))
		{
			strMethod += GetReturnType();
		}
		else if(strParam == _T("Guid"))
		{
			strMethod += _T("Guid");
		}
		else if(strParam == _T("object"))
		{
			strMethod += _T("object");
		}
		else if(strParam == _T("string"))
		{
			strMethod += _T("string");
		}
		else
		{
			strMethod += strParam;
		}
		m_strRtnProcessing += _T("\t\t\tUQueue.Load(out ");
	
		m_strRtnProcessing += _T("m_");
		m_strRtnProcessing += GetName();
		if(dt == dtUDT)
		{
			m_strRtnProcessing += _T("Rtn); //Don't forget implementing IUSerializer\n");
		}
		else
		{
			m_strRtnProcessing += _T("Rtn);\n");
		}
		strMethod += _T(" m_");
		strMethod += GetName();
		strMethod += _T("Rtn;\n\t");
	}

	GetInputParameters(aParameter);
	nSize = aParameter.GetSize();
	strMethod += (_T("protected void "));
	strMethod += GetName();
	strMethod += _T("Async");
	strMethod += _T("(");

	for(n=0; n<nSize; n++)
	{
		if(n)
		{
			strMethod += _T(", ");
		}
		strParam = GetInputString(aParameter[n].m_dt);
		if(strParam == _T("UDT"))
		{
			strMethod += aParameter[n].m_strType;
			strMethod += _T(" ");
			strMethod += aParameter[n].m_strName;
		}
		else if(strParam == _T("Guid"))
		{
			strMethod += _T("Guid");
			strMethod += _T(" ");
			strMethod += aParameter[n].m_strName;
		}
		else if(strParam == _T("object"))
		{
			strMethod += _T("object");
			strMethod += _T(" ");
			strMethod += aParameter[n].m_strName;
		}
		else if(strParam == _T("string"))
		{
			strMethod += _T("string");
			strMethod += _T(" ");
			strMethod += aParameter[n].m_strName;
		}
		else
		{
			strMethod += strParam;
			strMethod += _T(" ");
			strMethod += aParameter[n].m_strName;
		}
	}
	strMethod += _T(")\n\t{\n");
	CString strCase;
	strCase.Format(_T("\t\tcase %s.id%s%s:\n"), m_strDefClass, GetName(), GetServiceName());
	m_strRtnProcessing = strCase + m_strRtnProcessing;
	if(nSize > 0) 
	{
		CString strP;
		CString strHead(_T("\t\t//make sure that the handler is attached to a client socket before calling the below statement\n"));
		for(n=0; n<nSize; n++)
		{
			strP += _T(", ");
			strP += aParameter[n].m_strName;
			if(aParameter[n].m_dt == dtUDT)
			{
				strP += _T("/*Don't forget implementing IUSerializer*/");
			}
		}
		strTemp.Format(_T("\t\tSendRequest(%s.id%s%s%s);"), m_strDefClass, GetName(), GetServiceName(), strP);
		strTemp = strHead + strTemp;
	}
	else
	{
		CString strHead(_T("\t\t//make sure that the handler is attached to a client socket before calling the below statement\n"));
		strTemp.Format(_T("\t\tSendRequest(%s.id%s%s);"), m_strDefClass, GetName(), GetServiceName());
		strTemp = strHead + strTemp;
	}
	m_strRtnProcessing += _T("\t\t\tbreak;\n");
	strMethod += strTemp;
	strMethod += _T("\n\t}");	
	return strMethod;
}

CString CMethod::GetClientAsynInputMethodCpp()
{
	int n;
	int nSize;
	CString strTemp;
	CString strMethod;
	CString strParam;
	m_strRtnProcessing.Empty();
	CSimpleArray<CParameter> aOutParameter;
	CSimpleArray<CParameter> aParameter;
	
	GetOutputParameters(aOutParameter);
	nSize = aOutParameter.GetSize();
	m_nOutput = nSize;
	for(n=0; n<nSize; n++)
	{
		strParam = GetInputString(aOutParameter[n].m_dt);
		if(strParam == _T("UDT"))
		{
			strMethod += aOutParameter[n].m_strType;
		}
		else if(strParam == _T("GUID"))
		{
			strMethod += _T("GUID");
		}
		else if(strParam == _T("UVariant"))
		{
			strMethod += _T("UVariant");
		}
		else if(strParam == _T("string"))
		{
			strMethod += _T("std::wstring");
		}
		else if(strParam == _T("sbyte[]"))
		{
			strMethod += _T("std::string");
		}
		else
		{
			strMethod += strParam;
		}

		m_strRtnProcessing += _T("\t\t\tUQueue >> ");
		m_strRtnProcessing += _T("m_");
		m_strRtnProcessing += aOutParameter[n].m_strName;
		m_strRtnProcessing += _T("_");
		m_strRtnProcessing += GetName();
		if(aOutParameter[n].m_pt == ptOut && GetInputString(aOutParameter[n].m_dt) == _T("UDT"))
			m_strRtnProcessing += _T("; /*Don't forget implementing operators >> and <<*/\n");
		else
			m_strRtnProcessing += _T(";\n");

		strMethod += _T(" m_");
		strMethod += aOutParameter[n].m_strName;
		strMethod += _T("_");
		strMethod += GetName();
		strMethod += _T(";\n\t");
	}
	
	tagDataType dt = GetReturn();
	if(dt != dtVoid)
	{
		strParam = GetInputString(dt);
		if(strParam == _T("UDT"))
		{
			strMethod += this->GetReturnType();
		}
		else if(strParam == _T("GUID"))
		{
			strMethod += _T("GUID");
		}
		else if(strParam == _T("UVariant"))
		{
			strMethod += _T("UVariant");
		}
		else if(strParam == _T("string"))
		{
			strMethod += _T("std::wstring");
		}
		else if(strParam == _T("sbyte[]"))
		{
			strMethod += _T("std::string");
		}
		else
		{
			strMethod += strParam;
		}
		m_strRtnProcessing += _T("\t\t\tUQueue >> ");
		m_strRtnProcessing += _T("m_");
		m_strRtnProcessing += GetName();
		if(dt == dtUDT)
		{
			m_strRtnProcessing += _T("Rtn; //Don't forget implementing operators >> and <<\n");
		}
		else
		{
			m_strRtnProcessing += _T("Rtn;\n");
		}
		strMethod += _T(" m_");
		strMethod += GetName();
		strMethod += _T("Rtn;\n\t");
	}

	GetInputParameters(aParameter);
	nSize = aParameter.GetSize();
	strMethod += (_T("void "));
	strMethod += GetName();
	strMethod += _T("Async");
	strMethod += _T("(");

	for(n=0; n<nSize; n++)
	{
		if(n)
		{
			strMethod += _T(", ");
		}
		strParam = GetInputString(aParameter[n].m_dt);
		if(strParam == _T("UDT"))
		{
			strMethod += _T("const ");
			strMethod += aParameter[n].m_strType;
			strMethod += _T(" &");
			strMethod += aParameter[n].m_strName;
		}
		else if(strParam == _T("GUID"))
		{
			strMethod += _T("const ");
			strMethod += _T("GUID");
			strMethod += _T(" &");
			strMethod += aParameter[n].m_strName;
		}
		else if(strParam == _T("UVariant"))
		{
			strMethod += _T("const ");
			strMethod += _T("UVariant");
			strMethod += _T(" &");
			strMethod += aParameter[n].m_strName;
		}
		else if(strParam == _T("string"))
		{
			strMethod += _T("const wchar_t*");
			strMethod += _T(" ");
			strMethod += aParameter[n].m_strName;
		}
		else if(strParam == _T("sbyte[]"))
		{
			strMethod += _T("const char*");
			strMethod += _T(" ");
			strMethod += aParameter[n].m_strName;
		}
		else
		{
			strMethod += strParam;
			strMethod += _T(" ");
			strMethod += aParameter[n].m_strName;
		}
	}
	strMethod += _T(")\n\t{\n");
	CString strCase;
	strCase.Format(_T("\t\tcase id%s%s:\n"), GetName(), GetServiceName());
	m_strRtnProcessing = strCase + m_strRtnProcessing;
	CString strHead(_T("\t\t//make sure that the handler is attached to a client socket before calling the below statement\n"));
	if(nSize > 0) 
	{
		CString strP;
		for(n=0; n<nSize; n++)
		{
			strP += _T(", ");
			strP += aParameter[n].m_strName;
			if(aParameter[n].m_dt == dtUDT)
			{
				strP += _T("/*Don't forget implementing operators >> and <<*/");
			}
		}
		strTemp.Format(_T("\t\tSendRequest(id%s%s%s);"), GetName(), GetServiceName(), strP);
	}
	else
	{
		strTemp.Format(_T("\t\tSendRequest(id%s%s);"), GetName(), GetServiceName());
	}
	strTemp = strHead + strTemp;
	m_strRtnProcessing += _T("\t\t\tbreak;\n");
	strMethod += strTemp;
	strMethod += _T("\n\t}");	
	return strMethod;
}

LPCTSTR CMethod::GetSynFunVBNetNew()
{
	int n;
	int nSize;
	CString strTemp;
	CString strParam;
	CSimpleArray<CParameter> aOutParameter;
	GetOutputParameters(aOutParameter);
	nSize = aOutParameter.GetSize();
	m_nOutput = nSize;
	m_strSynFun = _T("\tPublic ");
	tagDataType dt = GetReturn();
	if(dt == dtVoid)
	{
		m_strSynFun += _T("Sub ");
	}
	else
	{
		m_strSynFun += _T("Function ");
	}

	m_strSynFun += GetName();
	m_strSynFun += _T("(");
	
	CSimpleArray<CString> lstUDT;

	nSize = m_aParameter.GetSize();
	for(n=0; n<nSize; n++)
	{
		CParameter &p = m_aParameter[n];
		if(n > 0)
		{
			m_strSynFun += _T(", ");
		}
		strParam = GetInputString(p.m_dt);
		if(p.m_pt == ptIn)
		{
			m_strSynFun += _T("ByVal ");
		}
		else
		{
			m_strSynFun += _T("ByRef ");
		}
		m_strSynFun += p.m_strName;
		m_strSynFun += _T(" As ");
		if(strParam == _T("UDT"))
		{
			m_strSynFun += p.m_strType;
			if(lstUDT.Find(p.m_strType) == -1)
				lstUDT.Add(p.m_strType);
		}
		else if(strParam == _T("Guid"))
		{
			m_strSynFun += _T("Guid");
		}
		else if(strParam == _T("Object"))
		{
			m_strSynFun += _T("Object");
		}
		else if(strParam == _T("string"))
		{
			m_strSynFun += _T("String");
		}
		else
		{
			m_strSynFun += strParam;
		}
	}
	m_strSynFun += _T(") ");
	if(dt != dtVoid)
	{
		m_strSynFun += _T("As ");
		strParam = GetInputString(GetReturn());
		if(strParam == _T("UDT"))
		{
			m_strSynFun += GetReturnType();
		}
		else if(strParam == _T("Guid"))
		{
			m_strSynFun += _T("Guid");
		}
		else if(strParam == _T("Object"))
		{
			m_strSynFun += _T("Object");
		}
		else if(strParam == _T("string"))
		{
			m_strSynFun += _T("String");
		}
		else
		{
			m_strSynFun += strParam;
		}
	}

	int k, kSize = lstUDT.GetSize();
	if(kSize > 0)
	{
		m_strSynFun += _T("\n\t\t");
		m_strSynFun += _T("'Please implement IUSerializer for the class");
		if(kSize > 1)
			m_strSynFun += _T("es ");
		else
			m_strSynFun += _T(" ");
		for(k=0; k<kSize; ++k)
		{
			if(k && k < kSize - 1)
				m_strSynFun += _T(", ");
			else if(k && k == kSize - 1)
				m_strSynFun += _T(" and ");
			m_strSynFun += lstUDT[k];
		}
	}
	
	if(GetReturn() != dtVoid)
	{
		m_strSynFun += _T("\n");
		m_strSynFun += _T("\t\tDim ");
		m_strSynFun += GetName();
		m_strSynFun += _T("Rtn");
		m_strSynFun += _T(" As ");

		strParam = GetInputString(GetReturn());
		if(strParam == _T("UDT"))
		{
			m_strSynFun += GetReturnType();
			m_strSynFun += _T(" = Nothing 'Please implement IUSerializer for the class!!!");
		}
		else if(strParam == _T("Guid"))
		{
			m_strSynFun += _T("Guid");
		}
		else if(strParam == _T("Object"))
		{
			m_strSynFun += _T("Object = Nothing");
		}
		else if(strParam == _T("string"))
		{
			m_strSynFun += _T("String = Nothing");
		}
		else
		{
			m_strSynFun += strParam;
		}
	}
	m_strSynFun += _T("\n");
	m_strSynFun += _T("\t\t");
	m_strSynFun += _T("Dim bProcessRy as Boolean = ProcessR");
	TCHAR strR[20] = {0};
	::sprintf(strR, "%d", GetOutputCount() + (GetReturn() != dtVoid));
	m_strSynFun += strR;
	m_strSynFun += _T("(");
	CString str;
	str.Format(_T("%s.id%s%s"), m_strDefClass, GetName(), GetServiceName());
	m_strSynFun += str;
	CSimpleArray<CParameter> aInput;
	GetInputParameters(aInput);
	nSize = aInput.GetSize();
	for(n=0; n<nSize; n++)
	{
		m_strSynFun += _T(", ");
		CParameter &p = aInput[n];
		m_strSynFun += p.m_strName;
	}

	GetOutputParameters(aInput);
	nSize = aInput.GetSize();
	for(n=0; n<nSize; n++)
	{
		CParameter &p = aInput[n];
		strTemp.Format(_T(", %s"), p.m_strName);
		m_strSynFun += strTemp;
	}
	if(GetReturn() != dtVoid)
	{
		m_strSynFun += _T(", ");
		m_strSynFun += GetName();
		m_strSynFun += _T("Rtn");
	}
	m_strSynFun += _T(")\n");

	if(GetReturn() != dtVoid)
	{
		m_strSynFun += _T("\t\treturn ");
		m_strSynFun += GetName();
		m_strSynFun += _T("Rtn\n");
	}
	if(GetReturn() == dtVoid)
	{
		m_strSynFun += _T("\tEnd Sub\n");
	}
	else
	{
		m_strSynFun += _T("\tEnd Function\n");
	}
	return m_strSynFun;
}


LPCTSTR CMethod::GetSynFunVBNet()
{
	int n;
	int nSize;
	CString strTemp;
	CString strParam;
	m_strSynFun = _T("\tPublic ");
	tagDataType dt = GetReturn();
	if(dt == dtVoid)
	{
		m_strSynFun += _T("Sub ");
	}
	else
	{
		m_strSynFun += _T("Function ");
	}

	m_strSynFun += GetName();
	m_strSynFun += _T("(");

	nSize = m_aParameter.GetSize();
	for(n=0; n<nSize; n++)
	{
		CParameter &p = m_aParameter[n];
		if(n > 0)
		{
			m_strSynFun += _T(", ");
		}
		strParam = GetInputString(p.m_dt);
		if(p.m_pt == ptIn)
		{
			m_strSynFun += _T("ByVal ");
		}
		else
		{
			m_strSynFun += _T("ByRef ");
		}
		m_strSynFun += p.m_strName;
		m_strSynFun += _T(" As ");
		if(strParam == _T("UDT"))
		{
			m_strSynFun += p.m_strType;
		}
		else if(strParam == _T("Guid"))
		{
			m_strSynFun += _T("Guid");
		}
		else if(strParam == _T("Object"))
		{
			m_strSynFun += _T("Object");
		}
		else if(strParam == _T("string"))
		{
			m_strSynFun += _T("String");
		}
		else
		{
			m_strSynFun += strParam;
		}
	}
	m_strSynFun += _T(") ");
	if(dt != dtVoid)
	{
		m_strSynFun += _T("As ");
		strParam = GetInputString(GetReturn());
		if(strParam == _T("UDT"))
		{
			m_strSynFun += GetReturnType();
		}
		else if(strParam == _T("Guid"))
		{
			m_strSynFun += _T("Guid");
		}
		else if(strParam == _T("Object"))
		{
			m_strSynFun += _T("Object");
		}
		else if(strParam == _T("string"))
		{
			m_strSynFun += _T("String");
		}
		else
		{
			m_strSynFun += strParam;
		}
	}

	m_strSynFun += _T("\n");
	m_strSynFun += _T("\t\t");
	m_strSynFun += GetName();
	m_strSynFun += _T("Async(");
	
	CSimpleArray<CParameter> aInput;
	GetInputParameters(aInput);
	nSize = aInput.GetSize();
	for(n=0; n<nSize; n++)
	{
		if(n > 0)
		{
			m_strSynFun += _T(", ");
		}
		CParameter &p = aInput[n];
		m_strSynFun += p.m_strName;
	}
	m_strSynFun += _T(")\n");
	m_strSynFun += _T("\t\tWaitAll()\n");
	
	GetOutputParameters(aInput);
	nSize = aInput.GetSize();
	for(n=0; n<nSize; n++)
	{
		CParameter &p = aInput[n];
		strTemp.Format(_T("\t\t%s = m_%s_%s\n"), p.m_strName, p.m_strName, GetName());
		m_strSynFun += strTemp;
	}

	if(GetReturn() != dtVoid)
	{
		m_strSynFun += _T("\t\treturn ");
		m_strSynFun += _T("m_");
		m_strSynFun += GetName();
		m_strSynFun += _T("Rtn\n");
	}
	if(GetReturn() == dtVoid)
	{
		m_strSynFun += _T("\tEnd Sub\n");
	}
	else
	{
		m_strSynFun += _T("\tEnd Function\n");
	}
	return m_strSynFun;
}

LPCTSTR CMethod::GetSynFunCSharpNew()
{
	int n;
	int nSize;
	CString strTemp;
	CString strParam;
	CSimpleArray<CParameter> aOutParameter;
	GetOutputParameters(aOutParameter);
	nSize = aOutParameter.GetSize();
	m_nOutput = nSize;

	m_strSynFun = _T("\tpublic ");
	tagDataType dt = GetReturn();
	if(dt == dtVoid)
	{
		m_strSynFun += _T("void ");
	}
	else
	{
		strParam = GetInputString(GetReturn());
		if(strParam == _T("UDT"))
		{
			m_strSynFun += GetReturnType();
			m_strSynFun += _T(" /*Please implement IUSerializer for the class!!!*/");
		}
		else if(strParam == _T("Guid"))
		{
			m_strSynFun += _T("Guid ");
		}
		else if(strParam == _T("object"))
		{
			m_strSynFun += _T("object ");
		}
		else if(strParam == _T("string"))
		{
			m_strSynFun += _T("string ");
		}
		else
		{
			m_strSynFun += strParam;
			m_strSynFun += _T(" ");
		}
	}
	m_strSynFun += GetName();
	m_strSynFun += _T("(");
	
	nSize = m_aParameter.GetSize();
	for(n=0; n<nSize; n++)
	{
		CParameter &p = m_aParameter[n];
		if(n > 0)
		{
			m_strSynFun += _T(", ");
		}
		strParam = GetInputString(p.m_dt);
		if(p.m_pt == ptInOut)
		{
			m_strSynFun += _T("ref ");
		}
		else if(p.m_pt == ptOut)
		{
			m_strSynFun += _T("out ");
		}
		if(strParam == _T("UDT"))
		{
			m_strSynFun += p.m_strType;
			m_strSynFun += _T(" /*Please implement IUSerializer for the class!!!*/");
		}
		else if(strParam == _T("Guid"))
		{
			m_strSynFun += _T("Guid ");
		}
		else if(strParam == _T("object"))
		{
			m_strSynFun += _T("object ");
		}
		else if(strParam == _T("string"))
		{
			m_strSynFun += _T("string ");
		}
		else
		{
			m_strSynFun += strParam;
			m_strSynFun += _T(" ");
		}
		m_strSynFun += p.m_strName;
	}
	
	m_strSynFun += _T(")\n\t{\n");

	if(GetReturn() != dtVoid)
	{
		m_strSynFun += _T("\t\t");
		strParam = GetInputString(GetReturn());
		if(strParam == _T("UDT"))
			m_strSynFun += GetReturnType();
		else 
			m_strSynFun += strParam;
		m_strSynFun += _T(" ");
		m_strSynFun += GetName();
		m_strSynFun += _T("Rtn;\n");
	}
	
	m_strSynFun += _T("\t\t");
	m_strSynFun += _T("bool bProcessRy = ProcessR");
	TCHAR strR[20] = {0};
	::sprintf(strR, "%d", GetOutputCount() + (GetReturn() != dtVoid));
	m_strSynFun += strR;
	m_strSynFun += _T("(");

	CString str;
	str.Format(_T("%s.id%s%s"), m_strDefClass, GetName(), GetServiceName());
	m_strSynFun += str;
	
	CSimpleArray<CParameter> aInput;
	GetInputParameters(aInput);
	nSize = aInput.GetSize();
	for(n=0; n<nSize; n++)
	{
		m_strSynFun += _T(", ");
		CParameter &p = aInput[n];
		m_strSynFun += p.m_strName;
	}
	
	GetOutputParameters(aInput);
	nSize = aInput.GetSize();
	for(n=0; n<nSize; n++)
	{
		CParameter &p = aInput[n];
		strTemp.Format(_T(", out %s"), p.m_strName);
		m_strSynFun += strTemp;
	}
	if(GetReturn() != dtVoid)
	{
		m_strSynFun += _T(", out ");
		m_strSynFun += GetName();
		m_strSynFun += _T("Rtn");
	}
	m_strSynFun += _T(");\n");

	if(GetReturn() != dtVoid)
	{
		m_strSynFun += _T("\t\treturn ");
		m_strSynFun += GetName();
		m_strSynFun += _T("Rtn;\n");
	}
	m_strSynFun += _T("\t}\n");
	return m_strSynFun;
}

LPCTSTR CMethod::GetSynFunCSharp()
{
	int n;
	int nSize;
	CString strTemp;
	CString strParam;
	m_strSynFun = _T("\tpublic ");
	tagDataType dt = GetReturn();
	if(dt == dtVoid)
	{
		m_strSynFun += _T("void ");
	}
	else
	{
		strParam = GetInputString(GetReturn());
		if(strParam == _T("UDT"))
		{
			m_strSynFun += GetReturnType();
			m_strSynFun += _T(" ");
		}
		else if(strParam == _T("Guid"))
		{
			m_strSynFun += _T("Guid ");
		}
		else if(strParam == _T("object"))
		{
			m_strSynFun += _T("object ");
		}
		else if(strParam == _T("string"))
		{
			m_strSynFun += _T("string ");
		}
		else
		{
			m_strSynFun += strParam;
			m_strSynFun += _T(" ");
		}
	}
	m_strSynFun += GetName();
	m_strSynFun += _T("(");
	
	nSize = m_aParameter.GetSize();
	for(n=0; n<nSize; n++)
	{
		CParameter &p = m_aParameter[n];
		if(n > 0)
		{
			m_strSynFun += _T(", ");
		}
		strParam = GetInputString(p.m_dt);
		if(p.m_pt == ptInOut)
		{
			m_strSynFun += _T("ref ");
		}
		else if(p.m_pt == ptOut)
		{
			m_strSynFun += _T("out ");
		}
		if(strParam == _T("UDT"))
		{
			m_strSynFun += p.m_strType;
			m_strSynFun += _T(" ");
		}
		else if(strParam == _T("Guid"))
		{
			m_strSynFun += _T("Guid ");
		}
		else if(strParam == _T("object"))
		{
			m_strSynFun += _T("object ");
		}
		else if(strParam == _T("string"))
		{
			m_strSynFun += _T("string ");
		}
		else
		{
			m_strSynFun += strParam;
			m_strSynFun += _T(" ");
		}
		m_strSynFun += p.m_strName;
	}
	
	m_strSynFun += _T(")\n\t{\n");
	
	m_strSynFun += _T("\t\t");
	m_strSynFun += GetName();
	m_strSynFun += _T("Async(");
	
	CSimpleArray<CParameter> aInput;
	GetInputParameters(aInput);
	nSize = aInput.GetSize();
	for(n=0; n<nSize; n++)
	{
		if(n > 0)
		{
			m_strSynFun += _T(", ");
		}
		CParameter &p = aInput[n];
		m_strSynFun += p.m_strName;
	}
	m_strSynFun += _T(");\n");
	m_strSynFun += _T("\t\tWaitAll();\n");
	
	GetOutputParameters(aInput);
	nSize = aInput.GetSize();
	for(n=0; n<nSize; n++)
	{
		CParameter &p = aInput[n];
		strTemp.Format(_T("\t\t%s = m_%s_%s;\n"), p.m_strName, p.m_strName, GetName());
		m_strSynFun += strTemp;
	}

	if(GetReturn() != dtVoid)
	{
		m_strSynFun += _T("\t\treturn ");
		m_strSynFun += _T("m_");
		m_strSynFun += GetName();
		m_strSynFun += _T("Rtn;\n");
	}
	m_strSynFun += _T("\t}\n");
	return m_strSynFun;
}

LPCTSTR CMethod::GetSynFunCpp()
{
	int n;
	int nSize;
	CString strTemp;
	CString strParam;
	m_strSynFun = _T("\t");
	tagDataType dt = GetReturn();
	if(dt == dtVoid)
	{
		m_strSynFun += _T("void ");
	}
	else
	{
		strParam = GetInputString(GetReturn());
		if(strParam == _T("UDT"))
		{
			m_strSynFun += _T("const ");
			m_strSynFun += GetReturnType();
			m_strSynFun += _T("& ");
		}
		else if(strParam == _T("GUID"))
		{
			m_strSynFun += _T("const GUID& ");
		}
		else if(strParam == _T("UVariant"))
		{
			m_strSynFun += _T("const UVariant& ");
		}
		else if(strParam == _T("astring"))
		{
			m_strSynFun += _T("const std::string& ");
		}
		else if(strParam == _T("string"))
		{
			m_strSynFun += _T("const std::wstring& ");
		}
		else
		{
			m_strSynFun += strParam;
			m_strSynFun += _T(" ");
		}
	}
	m_strSynFun += GetName();
	m_strSynFun += _T("(");
	
	nSize = m_aParameter.GetSize();
	for(n=0; n<nSize; n++)
	{
		CParameter &p = m_aParameter[n];
		if(n > 0)
		{
			m_strSynFun += _T(", ");
		}
		strParam = GetInputString(p.m_dt);
		if(strParam == _T("UDT"))
		{
			if(p.m_pt == ptIn)
			{
				m_strSynFun += _T("const ");
			}
			else if(p.m_pt == ptOut)
			{
				m_strSynFun += _T("/*out*/");
			}
			else
			{
				m_strSynFun += _T("/*inout*/");
			}
			m_strSynFun += p.m_strType;
			m_strSynFun += _T(" &");
		}
		else if(strParam == _T("GUID"))
		{
			if(p.m_pt == ptIn)
			{
				m_strSynFun += _T("const ");
			}
			else if(p.m_pt == ptOut)
			{
				m_strSynFun += _T("/*out*/");
			}
			else
			{
				m_strSynFun += _T("/*inout*/");
			}
			m_strSynFun += _T("GUID &");
		}
		else if(strParam == _T("UVariant"))
		{
			if(p.m_pt == ptIn)
			{
				m_strSynFun += _T("const ");
			}
			else if(p.m_pt == ptOut)
			{
				m_strSynFun += _T("/*out*/");
			}
			else
			{
				m_strSynFun += _T("/*inout*/");
			}
			m_strSynFun += _T("UVariant &");
		}
		else if(strParam == _T("string"))
		{
			if(p.m_pt == ptIn)
			{
				m_strSynFun += _T("const wchar_t* ");
			}
			else if(p.m_pt == ptOut)
			{
				m_strSynFun += _T("/*out*/std::wstring &");
			}
			else
			{
				m_strSynFun += _T("/*inout*/std::wstring &");
			}
		}
		else if(strParam == _T("sbyte[]"))
		{
			if(p.m_pt == ptIn)
			{
				m_strSynFun += _T("const char* ");
			}
			else if(p.m_pt == ptOut)
			{
				m_strSynFun += _T("/*out*/std::string &");
			}
			else
			{
				m_strSynFun += _T("/*inout*/std::string &");
			}
		}
		else
		{
			if(p.m_pt == ptOut)
			{
				m_strSynFun += _T("/*out*/");
			}
			else if(p.m_pt == ptInOut)
			{
				m_strSynFun += _T("/*inout*/");
			}
			m_strSynFun += strParam;
			if(p.m_pt == ptIn)
			{
				m_strSynFun += _T(" ");
			}
			else
			{
				m_strSynFun += _T(" &");
			}
		}
		m_strSynFun += p.m_strName;
	}
	
	m_strSynFun += _T(")\n\t{\n");
	
	m_strSynFun += _T("\t\t");
	m_strSynFun += GetName();
	m_strSynFun += _T("Async(");
	
	CSimpleArray<CParameter> aInput;
	GetInputParameters(aInput);
	nSize = aInput.GetSize();
	for(n=0; n<nSize; n++)
	{
		if(n > 0)
		{
			m_strSynFun += _T(", ");
		}
		CParameter &p = aInput[n];
		m_strSynFun += p.m_strName;
	}
	m_strSynFun += _T(");\n");
	m_strSynFun += _T("\t\tWaitAll();\n");
	
	GetOutputParameters(aInput);
	nSize = aInput.GetSize();
	for(n=0; n<nSize; n++)
	{
		CParameter &p = aInput[n];
		strTemp.Format(_T("\t\t%s = m_%s_%s;\n"), p.m_strName, p.m_strName, GetName());
		m_strSynFun += strTemp;
	}

	if(GetReturn() != dtVoid)
	{
		m_strSynFun += _T("\t\treturn ");
		m_strSynFun += _T("m_");
		m_strSynFun += GetName();
		m_strSynFun += _T("Rtn;\n");
	}
	m_strSynFun += _T("\t}\n");
	return m_strSynFun;
}

LPCTSTR CMethod::GetSynFunCppNew()
{
	int n;
	int nSize;
	CString strTemp;
	CString strParam;
	CSimpleArray<CParameter> aOutParameter;
	GetOutputParameters(aOutParameter);
	nSize = aOutParameter.GetSize();
	m_nOutput = nSize;

	m_strSynFun = _T("\t");
	tagDataType dt = GetReturn();
	if(dt == dtVoid)
	{
		m_strSynFun += _T("void ");
	}
	else
	{
		strParam = GetInputString(GetReturn());
		if(strParam == _T("UDT"))
		{
			m_strSynFun += GetReturnType();
			m_strSynFun += _T("/*Please override >> && << for the class!!!*/ ");
		}
		else if(strParam == _T("GUID"))
		{
			m_strSynFun += _T("GUID ");
		}
		else if(strParam == _T("UVariant"))
		{
			m_strSynFun += _T("UVariant ");
		}
		else if(strParam == _T("string"))
		{
			m_strSynFun += _T("std::wstring ");
		}
		else if(strParam == _T("sbyte[]"))
		{
			m_strSynFun += _T("std::string ");
		}
		else
		{
			m_strSynFun += strParam;
			m_strSynFun += _T(" ");
		}
	}
	m_strSynFun += GetName();
	m_strSynFun += _T("(");
	
	nSize = m_aParameter.GetSize();
	for(n=0; n<nSize; n++)
	{
		CParameter &p = m_aParameter[n];
		if(n > 0)
		{
			m_strSynFun += _T(", ");
		}
		strParam = GetInputString(p.m_dt);
		if(strParam == _T("UDT"))
		{
			if(p.m_pt == ptIn)
			{
				m_strSynFun += _T("const ");
			}
			else if(p.m_pt == ptOut)
			{
				m_strSynFun += _T("/*out*/");
			}
			else
			{
				m_strSynFun += _T("/*inout*/");
			}
			m_strSynFun += p.m_strType;
			m_strSynFun += _T("/*Please override >> && << for the class!!!*/&");
		}
		else if(strParam == _T("GUID"))
		{
			if(p.m_pt == ptIn)
			{
				m_strSynFun += _T("const ");
			}
			else if(p.m_pt == ptOut)
			{
				m_strSynFun += _T("/*out*/");
			}
			else
			{
				m_strSynFun += _T("/*inout*/");
			}
			m_strSynFun += _T("GUID &");
		}
		else if(strParam == _T("UVariant"))
		{
			if(p.m_pt == ptIn)
			{
				m_strSynFun += _T("const ");
			}
			else if(p.m_pt == ptOut)
			{
				m_strSynFun += _T("/*out*/");
			}
			else
			{
				m_strSynFun += _T("/*inout*/");
			}
			m_strSynFun += _T("UVariant &");
		}
		else if(strParam == _T("string"))
		{
			if(p.m_pt == ptIn)
			{
				m_strSynFun += _T("const wchar_t* ");
			}
			else if(p.m_pt == ptOut)
			{
				m_strSynFun += _T("/*out*/std::wstring &");
			}
			else
			{
				m_strSynFun += _T("/*inout*/std::wstring &");
			}
		}
		else if(strParam == _T("sbyte[]"))
		{
			if(p.m_pt == ptIn)
			{
				m_strSynFun += _T("const char* ");
			}
			else if(p.m_pt == ptOut)
			{
				m_strSynFun += _T("/*out*/std::string &");
			}
			else
			{
				m_strSynFun += _T("/*inout*/std::string &");
			}
		}
		else
		{
			if(p.m_pt == ptOut)
			{
				m_strSynFun += _T("/*out*/");
			}
			else if(p.m_pt == ptInOut)
			{
				m_strSynFun += _T("/*inout*/");
			}
			m_strSynFun += strParam;
			if(p.m_pt == ptIn)
			{
				m_strSynFun += _T(" ");
			}
			else
			{
				m_strSynFun += _T(" &");
			}
		}
		m_strSynFun += p.m_strName;
	}
	
	m_strSynFun += _T(")\n\t{\n");
	if(GetReturn() != dtVoid)
	{
		m_strSynFun += _T("\t\t");
		strParam = GetInputString(GetReturn());
		if(strParam == _T("UDT"))
		{
			m_strSynFun += GetReturnType();
			m_strSynFun += _T(" ");
		}
		else if(strParam == _T("GUID"))
		{
			m_strSynFun += _T("GUID ");
		}
		else if(strParam == _T("UVariant"))
		{
			m_strSynFun += _T("UVariant ");
		}
		else if(strParam == _T("string"))
		{
			m_strSynFun += _T("std::wstring ");
		}
		else if(strParam == _T("sbyte[]"))
		{
			m_strSynFun += _T("std::string ");
		}
		else
		{
			m_strSynFun += strParam;
			m_strSynFun += _T(" ");
		}
		m_strSynFun += GetName();
		m_strSynFun += _T("Rtn;\n");
	}
	m_strSynFun += _T("\t\tbool bProcessRy = ProcessR");
	TCHAR strR[20] = {0};
	::sprintf(strR, "%d", GetOutputCount() + (GetReturn() != dtVoid));
	m_strSynFun += strR;
	m_strSynFun += _T("(");
	
	CString str;
	str.Format(_T("id%s%s"), GetName(), GetServiceName());
	m_strSynFun += str;
	
	CSimpleArray<CParameter> aInput;
	GetInputParameters(aInput);
	nSize = aInput.GetSize();
	for(n=0; n<nSize; n++)
	{
		m_strSynFun += _T(", ");
		CParameter &p = aInput[n];
		m_strSynFun += p.m_strName;
	}
	GetOutputParameters(aInput);
	nSize = aInput.GetSize();
	for(n=0; n<nSize; n++)
	{
		CParameter &p = aInput[n];
		strTemp.Format(_T(", %s"), p.m_strName);
		m_strSynFun += strTemp;
	}

	if(GetReturn() != dtVoid)
	{
		m_strSynFun += _T(", ");
		m_strSynFun += GetName();
		m_strSynFun += _T("Rtn");
	}

	m_strSynFun += _T(");\n");

	if(GetReturn() != dtVoid)
	{
		m_strSynFun += _T("\t\treturn ");
		m_strSynFun += GetName();
		m_strSynFun += _T("Rtn;\n");
	}
	m_strSynFun += _T("\t}\n");
	return m_strSynFun;
}
