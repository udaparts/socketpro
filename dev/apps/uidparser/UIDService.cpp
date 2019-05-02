#include "StdAfx.h"
#include "uidservice.h"

CUIDService::CUIDService(long lSvsID)
{
	m_lSvsID = lSvsID;
}

CUIDService::CUIDService(const CUIDService& UIDService)
{
	m_lSvsID = UIDService.m_lSvsID;
	m_strDeclaration = UIDService.m_strDeclaration;
	m_strName = UIDService.m_strName;
	m_aMethods = UIDService.m_aMethods;
	m_strErrorMessage = UIDService.m_strErrorMessage;
	m_lang = UIDService.m_lang;
	m_strBaseSvs = UIDService.m_strBaseSvs;
}

CUIDService::~CUIDService(void)
{

}

CUIDService& CUIDService::operator=(const CUIDService& UIDService)
{
	m_aMethods.RemoveAll();
	m_lSvsID = UIDService.m_lSvsID;
	m_strDeclaration = UIDService.m_strDeclaration;
	m_strName = UIDService.m_strName;
	m_aMethods = UIDService.m_aMethods;
	m_strErrorMessage = UIDService.m_strErrorMessage;
	m_lang = UIDService.m_lang;
	m_strBaseSvs = UIDService.m_strBaseSvs;
	return *this;
}

ULONG CUIDService::GetSvsID()
{
	return m_lSvsID;
}

int CUIDService::GetCountOfSlowMethods()
{
	int nCount = 0;
	int n;
	for(n=0; n<m_aMethods.GetSize(); n++)
	{
		if(m_aMethods[n].IsSlow())
		{
			nCount += 1;
		}
	}
	return nCount;
}

bool CUIDService::AlreadyExist(const CString &strMethodName)
{
	int n;
	for(n=0; n<m_aMethods.GetSize(); n++)
	{
		if(strMethodName.CompareNoCase(m_aMethods[n].GetName()) == 0)
			return true;
	}
	return false;
}

CString CUIDService::GetPeerSwitchVBNet(bool bSlow)
{
	int n;
	CString strSwitch(_T("\t\tSelect Case (sRequestID)\n"));
	int nSize = m_aMethods.GetSize();
	for(n=0; n<nSize; n++)
	{
		int nInput, nOutput;
		CMethod &method = m_aMethods[n];
		if(method.IsSlow() != bSlow)
			continue;
		CString strID;
		strID.Format(_T("%s.id%s%s"), method.GetDefClass(), method.GetName(), GetName());
		strSwitch += _T("\t\tCase ");
		strSwitch += strID;
		strSwitch += _T("\n");
		
		CString strTemp = method.GetPeerCallVBNetEx(nInput, nOutput);
		CString str;
		if(nInput == 0 && nOutput == 0)
			str.Format(_T("\t\t\tM_I0_R0(AddressOf %s)\n"), method.GetName());
		else
		{
			str.Format(_T("\t\t\tM_I%d_R%d(Of %s)(AddressOf %s)\n"), nInput, nOutput, strTemp, method.GetName());
		}
		strSwitch += str;
		//strSwitch += method.GetPeerCallVBNet();
	}
	strSwitch += _T("\t\tCase Else\n\t\tEnd Select\n");
	return strSwitch;
}

CString CUIDService::GetPeerSwitchCSharp(bool bSlow)
{
	int n;
	CString strSwitch(_T("\t\tswitch(sRequestID)\n\t\t{\n"));
	int nSize = m_aMethods.GetSize();
	for(n=0; n<nSize; n++)
	{
		CMethod &method = m_aMethods[n];
		if(method.IsSlow() != bSlow)
			continue;
		int nInput, nOutput;
		CString strID;
		strID.Format(_T("%s.id%s%s:\n"), method.GetDefClass(), method.GetName(), GetName());
		strSwitch += _T("\t\tcase ");
		strSwitch += strID;
		CString strTemp = method.GetPeerCallCSharpEx(nInput, nOutput);
		CString str;
		if(nInput == 0 && nOutput == 0)
		{
			str.Format(_T("\t\t\tM_I0_R0(%s);\n"), method.GetName());
		}
		else
			str.Format(_T("\t\t\tM_I%d_R%d<%s>(%s);\n"), nInput, nOutput, strTemp, method.GetName());
		str += _T("\t\t\tbreak;\n");
		strSwitch += str;
/*		strSwitch += strID;
		strSwitch += _T(":\n\t\t{\n");
		strSwitch += method.GetPeerCallCSharp();
		strSwitch += _T("\t\t}\n\t\t\tbreak;\n");*/
	}
	strSwitch += _T("\t\tdefault:\n\t\t\tbreak;\n\t\t}\n");
	return strSwitch;

/*	CString strSwitch(_T("\t\tswitch(sRequestID)\n\t\t{\n"));
	int n;
	int nSize = m_aMethods.GetSize();
	for(n=0; n<nSize; n++)
	{
		CMethod &method = m_aMethods[n];
		if(method.IsSlow() != bSlow)
			continue;
		CString strID;
		strID.Format(_T("%s.id%s%s"), method.GetDefClass(), method.GetName(), GetName());
		strSwitch += _T("\t\tcase ");
		strSwitch += strID;
		strSwitch += _T(":\n\t\t{\n");
		strSwitch += method.GetPeerCallCSharp();
		strSwitch += _T("\t\t}\n\t\t\tbreak;\n");
	}
	strSwitch += _T("\t\tdefault:\n\t\t\tbreak;\n\t\t}\n");
	return strSwitch;*/
}

CString CUIDService::GetPeerSwitchCpp(bool bSlow)
{
	CString strSwitch(_T("\t\tBEGIN_SWITCH(reqId)\n"));
	int n;
	int nSize = m_aMethods.GetSize();
	for(n=0; n<nSize; n++)
	{
		CMethod &method = m_aMethods[n];
		if(method.IsSlow() != bSlow)
			continue;
		int nInput, nOutput;
		CString strTemp = method.GetPeerCallCppEx(nInput, nOutput);
		CString strID;
		strID.Format(_T("\t\t\tM_I%d_R%d(id%s%s, %s"), nInput, nOutput, method.GetName(), GetName(), method.GetName());
		if(strTemp.GetLength() > 0)
		{
			strID += _T(", ");
			strID += strTemp;
		}
		strID += _T(")\n");
		strSwitch += strID;
	}
	strSwitch += _T("\t\tEND_SWITCH\n");


/*	CString strSwitch(_T("\t\tswitch(reqId)\n\t\t{\n"));
	int n;
	int nSize = m_aMethods.GetSize();
	for(n=0; n<nSize; n++)
	{
		CMethod &method = m_aMethods[n];
		if(method.IsSlow() != bSlow)
			continue;
		CString strID;
		strID.Format(_T("id%s%s"), method.GetName(), GetName());
		strSwitch += _T("\t\tcase ");
		strSwitch += strID;
		strSwitch += _T(":\n\t\t{\n");
		strSwitch += method.GetPeerCallCpp();
		strSwitch += _T("\t\t}\n\t\t\tbreak;\n");
	}
	strSwitch += _T("\t\tdefault:\n\t\t\tbreak;\n\t\t}\n");*/
	return strSwitch;
}


bool CUIDService::SetDeclaration(LPCTSTR strDecalaration)
{
	m_aMethods.RemoveAll();
	bool bSuc = true;
	CString strDec = strDecalaration;
	strDec.Trim(_T(' '));
	int nIndex;
	int nIndex2;
	int nColon;
	CString strLeft;
	CString strTemp;
	strDec.Replace(cTab, cBlank);
	strDec.Replace(cLine, cBlank);

	m_strDeclaration = strDec;
	strDec.Trim(_T(' '));

	strDec.Replace(cReturn, cBlank);
	do
	{
		nColon = strDec.Find(_T(':'));
		nIndex = strDec.Find(_T("]"));
		if(nIndex == nColon)
		{
			m_strErrorMessage = _T("ServiceID is not ended with ']', or no ':' is found!");
			bSuc = false;
			break;
		}
		
		if(nIndex >= 0 && nColon >= 0)
		{
			m_strErrorMessage = _T("A service can not be declared with both ']' and ':' at the same time");
			bSuc = false;
			break;
		}
		if(nColon > 0)
		{
			CString strRight;
			m_lSvsID = SERVICE_ID_NOT_AVAILABLE;
			nIndex = strDec.Find(_T('{'));
			if(nIndex == -1)
			{
				m_strErrorMessage = _T("Char '{' missing");
				bSuc = false;
			}
			m_strBaseSvs = strDec.Mid(nColon + 1, nIndex - (nColon + 1));
			m_strBaseSvs.Trim(_T(' '));
			strTemp = strDec.Left(nColon);
			strRight = strDec.Mid(nIndex);
			strDec = strTemp + strRight;
		}
		else
		{
			m_strBaseSvs.Empty();
			strLeft = strDec.Left(nIndex + 1);
			strDec = strDec.Mid(nIndex + 1);
			strLeft.Remove(_T(' '));
			nIndex = strLeft.Find(strServiceIDToken);
			if(nIndex == -1)
			{
				m_strErrorMessage = _T("ServiceID not found");
				bSuc = false;
				break;
			}
			nIndex2 = strLeft.Find(_T("]"));
			if(nIndex2 == -1)
			{
				m_strErrorMessage = _T("ServiceID not enclosed with ']'");
				bSuc = false;
				break;
			}
			
			if(nIndex2 < (nIndex + (int)::_tcslen(strServiceIDToken)))
			{
				m_strErrorMessage = _T("ServiceID not defined properly");
				bSuc = false;
				break;
			}
			
			strTemp = strLeft.Mid(nIndex + (int)::_tcslen(strServiceIDToken));
			nIndex2 = strTemp.Find(_T("]"));
			strTemp = strTemp.Left(nIndex2);
			long lSvsID = ::_ttoi(strTemp);
			if(lSvsID < 0)
			{
				m_strErrorMessage = _T("Service ID can't be a negative value");
				bSuc = false;
				break;
			}
			m_lSvsID = (ULONG)lSvsID;
		}
		
		nIndex = strDec.Find(_T("{"));
		if(nIndex == -1)
		{
			m_strErrorMessage = _T("Char '{' missing");
			bSuc = false;
			break;
		}
		
		m_strName = strDec.Left(nIndex);
		strDec = strDec.Mid(nIndex + 1);
		
		m_strName.Trim(_T(' '));
		
		if(!CMethod::IsValid(m_strName))
		{
			m_strErrorMessage = _T("Invalid service name");
			bSuc = false;
			break;
		}
		
		nIndex = strDec.Find(_T('}'));
		if(nIndex == -1)
		{
			m_strErrorMessage = _T("Service decalaration not ended with '}'");
			bSuc = false;
			break;
		}
		strDec = strDec.Left(nIndex);
		
		strDec.Trim(_T(' '));
		nIndex = strDec.Find(_T(';'));
		while(nIndex != -1)
		{
			strLeft = strDec.Left(nIndex + 1);
			CMethod Method(m_aMethods.GetSize());
			Method.SetLang(m_lang);
			bSuc = Method.SetDeclaration(strLeft);
			if(bSuc)
			{
				Method.SetServiceName(GetName());
				if(AlreadyExist(Method.GetName()))
				{
					m_strErrorMessage = _T("Can not have two methods with the same name");
					bSuc = false;
				}
				else
				{
					m_aMethods.Add(Method);
				}
			}
			else
			{
				m_strErrorMessage = Method.GetErrorMessage();
				m_strErrorMessage += _T(" with ");
				m_strErrorMessage += strLeft;
				bSuc = false;
			}
			if(!bSuc)
				break;
			strDec = strDec.Mid(nIndex + 1);
			strDec.Trim(_T(' '));
			nIndex = strDec.Find(_T(';'));
		}
		
		if(bSuc && strDec.GetLength() > 0)
		{
			m_strErrorMessage = _T("Invalid uid declaration");
			m_strErrorMessage += _T(" with ");
			m_strErrorMessage += strLeft;
			bSuc = false;
			break;
		}
	}while(false);
	return bSuc;
}
