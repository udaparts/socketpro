#include "StdAfx.h"
#include "uidfile.h"

CUidFile::CUidFile(tagLanguage lang)
{
	m_lang = lang;
}

CUidFile::~CUidFile(void)
{
}

void CUidFile::SetLanguage(tagLanguage lang)
{
	m_lang = lang;
}

LPCTSTR CUidFile::GetErrorMessage()
{
	return m_strErrorMessage;
}

bool CUidFile::ExistingService(LPCTSTR strSvsName)
{
	int n;
	for(n=0; n<m_aService.GetSize(); n++)
	{
		if(m_aService[n].GetName().CompareNoCase(strSvsName) == 0)
			return true;
	}
	return false;
}

bool CUidFile::ExistingService(ULONG ulSvsID)
{
	int n;
	for(n=0; n<m_aService.GetSize(); n++)
	{
		if(m_aService[n].GetSvsID() == SERVICE_ID_NOT_AVAILABLE)
			continue;
		if(m_aService[n].GetSvsID() == ulSvsID)
			return true;
	}
	return false;
}

bool CUidFile::CreateDefFileVBNet()
{
	int n;
	ULONG ulWrite;
	CString strDef;
	CString strTemp;
	CString strComment;
	int nSlash = m_strOutputFile.ReverseFind(_T('\\'));
	if(nSlash != -1)
	{
		strTemp = m_strOutputFile.Mid(nSlash + 1);
	}
	else
	{
		strTemp = m_strOutputFile;
	}
	int nDot = strTemp.Find(_T('.'));
	if(nDot != -1)
	{
		strTemp = strTemp.Left(nDot);
	}
	
	strDef.Format(_T("Public Module %sConst\n"), strTemp);
	
	int nSvs = m_aService.GetSize();
	for(n=0; n<nSvs; n++)
	{
		if(n)
		{
			strDef += _T("\n");
		}
		CUIDService &Svs = m_aService[n];
		if(Svs.GetSvsID() != SERVICE_ID_NOT_AVAILABLE)
		{
			strComment.Format(_T("\t'defines for service %s\n"), Svs.GetName());
			strDef += strComment;
			strTemp.Format(_T("\tpublic const sid%s as UInteger = (SocketProAdapter.BaseServiceID.sidReserved + %d)\n\n"), Svs.GetName(), Svs.GetSvsID());
			strDef += strTemp;
		}

		int j;
		CString strPrev = GetInitialRequestID(Svs.GetName());
		int jAll = Svs.GetMethods().GetSize();
		for(j = 0; j<jAll; j++)
		{
			CMethod &mb = Svs.GetMethods()[j];
			if(j == 0)
			{
				strTemp.Format(_T("\tpublic Const id%s%s as UShort = (%s)\n"), mb.GetName(), Svs.GetName(), strPrev);
			}
			else
			{
				strTemp.Format(_T("\tpublic Const id%s%s as UShort = (%s + 1)\n"), mb.GetName(), Svs.GetName(), strPrev);
			}
			strDef += strTemp;
			strPrev.Format(_T("id%s%s"), mb.GetName(), Svs.GetName());
		}
	}
	strDef += _T("End Module");

	::DeleteFile(m_strDefFile);
	HANDLE hFile = ::CreateFile(m_strDefFile, GENERIC_ALL, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		m_strErrorMessage.Format(_T("Error in opening the file %s"), m_strOutputFile);
		return false;
	}

	::WriteFile(hFile, (LPVOID)LPCTSTR(strDef), strDef.GetLength(), &ulWrite, NULL);
	::CloseHandle(hFile);
	return true;
}

bool CUidFile::CreateDefFileCSharp()
{
	int n;
	ULONG ulWrite;
	CString strDef;
	CString strTemp;
	CString strComment;
	int nSlash = m_strOutputFile.ReverseFind(_T('\\'));
	if(nSlash != -1)
	{
		strTemp = m_strOutputFile.Mid(nSlash + 1);
	}
	else
	{
		strTemp = m_strOutputFile;
	}
	int nDot = strTemp.Find(_T('.'));
	if(nDot != -1)
	{
		strTemp = strTemp.Left(nDot);
	}
	
	strDef.Format(_T("public static class %sConst\n{\n"), strTemp);
	
	int nSvs = m_aService.GetSize();
	for(n=0; n<nSvs; n++)
	{
		if(n)
		{
			strDef += _T("\n");
		}
		CUIDService &Svs = m_aService[n];
		
		if(Svs.GetSvsID() != SERVICE_ID_NOT_AVAILABLE)
		{
			strComment.Format(_T("\t//defines for service %s\n"), Svs.GetName());
			strDef += strComment;
			strTemp.Format(_T("\tpublic const uint sid%s = (SocketProAdapter.BaseServiceID.sidReserved + %d);\n\n"), Svs.GetName(), Svs.GetSvsID());
			strDef += strTemp;
		}

		int j;
		CString strPrev = GetInitialRequestID(Svs.GetName());
		int jAll = Svs.GetMethods().GetSize();
		for(j = 0; j<jAll; j++)
		{
			CMethod &mb = Svs.GetMethods()[j];
			if(j == 0)
			{
				strTemp.Format(_T("\tpublic const ushort id%s%s = (%s);\n"), mb.GetName(), Svs.GetName(), strPrev);
			}
			else
			{
				strTemp.Format(_T("\tpublic const ushort id%s%s = (%s + 1);\n"), mb.GetName(), Svs.GetName(), strPrev);
			}
			strDef += strTemp;
			strPrev.Format(_T("id%s%s"), mb.GetName(), Svs.GetName());
		}
	}
	strDef += _T("}");

	::DeleteFile(m_strDefFile);
	HANDLE hFile = ::CreateFile(m_strDefFile, GENERIC_ALL, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		m_strErrorMessage.Format(_T("Error in opening the file %s"), m_strOutputFile);
		return false;
	}

	::WriteFile(hFile, (LPVOID)LPCTSTR(strDef), strDef.GetLength(), &ulWrite, NULL);
	::CloseHandle(hFile);
	return true;
}

bool CUidFile::HandleIncludeFiles(CString &strUID)
{
	int nFHead;
	int nFEnd;
	bool bSuc = true;
	int nInclude = -1;
	
	do
	{
		nInclude = strUID.Find(UID_INCLUDE);
		if(nInclude == -1)
			break;
		nFHead = strUID.Find(_T('"'), nInclude + 8);
		nFEnd = strUID.Find(_T('"'), nFHead + 1);
		if(nFHead > 0 && nFEnd > (nFHead + 4))
		{
			CString strTempL;
			CString strTempR;
			CString strFile;
			strTempL = strUID.Left(nInclude);
			strTempR = strUID.Mid(nFEnd + 1);
			strFile = strUID.Mid(nFHead + 1, (nFEnd- (nFHead + 1)));
			CUidFile UidFile(m_lang);
			if(!UidFile.SetInputFile(strFile))
			{
				m_strErrorMessage = UidFile.m_strErrorMessage + _T("(");
				m_strErrorMessage += UidFile.m_strFileName;
				m_strErrorMessage += _T(")");
				return false;
			}
			
			strUID = strTempL;
			strUID += UidFile.m_strCleanUID;
			strUID += strTempR;
		}
	}while(true);
	
	return bSuc;
}

bool CUidFile::GenerateFiles()
{
	int nDot = m_strFileName.ReverseFind(_T('.'));
	if(nDot != -1)
	{
		m_strOutputFile = m_strFileName.Left(nDot);
	}
	else
	{
		m_strOutputFile = m_strFileName;
	}

	m_strProjName = m_strOutputFile + "Server";

	m_strDefFile = m_strOutputFile + _T("_i");
	m_strOutputFile += _T(".");
	m_strDefFile += _T(".");

	switch(m_lang)
	{
	case lCSharp:
		m_strDefFile += _T("cs");
		m_strOutputFile += _T("cs");
		if(CreateDefFileCSharp())
		{
			CreateClientFileCSharp();
			nDot = m_strOutputFile.ReverseFind(_T('.'));
			m_strOutputFile.Insert(nDot, _T("Impl"));
			CreateServerPeerFileCSharp();
		}
		break;
	case lCpp:
		m_strOutputFile += _T("h");
		m_strDefFile += _T("h");
		if(CreateCommonDefFileH())
		{
			CreateClientHandlerFileH();
			nDot = m_strOutputFile.ReverseFind(_T('.'));
			m_strOutputFile.Insert(nDot, _T("Impl"));
			CreateServerPeerFileH();
			if(m_aService.GetSize() > 0)
			{
				CString strSvsName = m_aService[0].GetName();
				//CreateVC6Proj(_T("C:\\Program Files\\UDAParts\\SocketPro\\include\\sprowrap.cpp"));
				CreateVC6OtherFiles();
			}
		}
		break;
	case lVBNet:
		m_strOutputFile += _T("vb");
		m_strDefFile += _T("vb");
		if(CreateDefFileVBNet())
		{
			CreateClientFileVBNet();
			nDot = m_strOutputFile.ReverseFind(_T('.'));
			m_strOutputFile.Insert(nDot, _T("Impl"));
			CreateServerPeerFileVBNet();
		}
		break;
	case lCLI:
		m_strOutputFile += _T("h");
		m_strDefFile += _T("h");
		nDot = m_strDefFile.ReverseFind(_T('.'));
		m_strDefFile.Insert(nDot, _T("Cli"));
		if(CreateCommonDefFileH())
		{
			nDot = m_strOutputFile.ReverseFind(_T('.'));
			m_strOutputFile.Insert(nDot, _T("Cli"));
			CreateClientHandlerFileH();
			nDot = m_strOutputFile.ReverseFind(_T('.'));
			m_strOutputFile.Insert(nDot, _T("Impl"));
			CreateServerPeerFileH();
		}
		break;
	default:
		ATLASSERT(FALSE);
		break;
	}

	return true;
}

bool CUidFile::CreateServerPeerFileVBNet()
{
	int n;
	ULONG ulWrite;
	CString strDef;
	CString strTemp;
	strDef += _T("' **** including all of defines, service id(s) and request id(s) *****\n");
	strDef += _T("Imports System\nImports SocketProAdapter\nImports SocketProAdapter.ServerSide\n\n");
	int nSvs = m_aService.GetSize();
	for(n=0; n<nSvs; n++)
	{
		CString strSvsImp;
		CUIDService &Svs = m_aService[n];
		int j;
		int jAll = Svs.GetMethods().GetSize();
		strDef += _T("'server implementation for service ");
		strDef += Svs.GetName();
		strDef += _T("\n");
		
		strDef += _T("Public class ");
		strDef += Svs.GetName();
		if(Svs.GetSvsID() == SERVICE_ID_NOT_AVAILABLE)
		{
			CString str;
			str.Format(_T("Peer : Inherits %sPeer\n"), Svs.GetBaseSvs());
			strDef += str;
		}
		else
		{
			strDef += _T("Peer : Inherits CClientPeer\n");
		}
		
		if(Svs.GetSvsID() != SERVICE_ID_NOT_AVAILABLE)
		{
			strDef += _T("\tProtected Overrides Sub OnSwitchFrom(ByVal nServiceID As UInteger)\n\t\t'initialize the object here\n\tEnd Sub\n\n");
			strDef += _T("\tProtected Overrides Sub OnReleaseResource(ByVal closing As Boolean, ByVal nInfo As UInteger)\n\t\tIf closing Then\n\t\t\t'closing the socket with error code = nInfo\n\t\tElse\n\t\t\t'switch to a new service with the service id = nInfo\n\t\tEnd If\n\n\t\t'release all of your resources here as early as possible\n\tEnd Sub\n\n");
		}
		for(j=0; j<jAll; j++)
		{
			CMethod &method = Svs.GetMethods()[j];
			strDef += method.GetMethodForPeerVBNet();
			strDef += _T("\n");
		}
		
		strDef += _T("\nEnd Class\n\n");
	}

	strDef += _T("public class CMySocketProServer : Inherits CSocketProServer\n\n");
	strDef += _T("\tProtected Overrides Function OnSettingServer() As Boolean\n");
    strDef += _T("\t\t'amIntegrated and amMixed not supported yet\n");
    strDef += _T("\t\tConfig.AuthenticationMethod = tagAuthenticationMethod.amOwn\n\n");
	strDef += _T("\t\tReturn True 'true -- ok; and false -- no listening socket\n");
    strDef += _T("\tEnd Function\n\n");
	strDef += _T("\tProtected Overrides Sub OnAccept(ByVal hSocket as ULong, ByVal nError as Integer)\n\t\t'when a socket is initially established\n\tEnd Sub\n\n");
	strDef += _T("\tProtected Overrides Function OnIsPermitted(ByVal hSocket As ULong, ByVal userId as String, ByVal password as String, ByVal nSvsID As UInteger) As Boolean\n\t\t'give permission to all\n");
	strDef += _T("\t\treturn true\n\tEnd Function\n\n");
	strDef += _T("\tProtected Overrides Sub OnClose(ByVal hSocket as ULong, ByVal nError as Integer)\n\t\t'when a socket is closed\n\tEnd Sub\n\n");
	
	nSvs = m_aService.GetSize();
	for(n=0; n<nSvs; n++)
	{
		CUIDService &Svs = m_aService[n];
		CUIDService LastSvs = FindLastDerived(Svs.GetName());
		if(LastSvs.GetName() == Svs.GetName())
		{
			CString str;
			str.Format(_T("\t<ServiceAttr(%s.sid%s)> "), m_strDefClass, Svs.GetName());
			strDef += str;
			strTemp.Format(_T("Private m_%s As CSocketProService(Of %sPeer) = new CSocketProService(Of %sPeer)()\n"), Svs.GetName(), Svs.GetName(), Svs.GetName());
			strDef += strTemp;
		}
	}
	strDef += _T("\t'One SocketPro server supports any number of services. You can list them here!\n\n");
	
	strDef += _T("\tShared Sub Main(ByVal args As String())\n");
	strDef += _T("\t\tDim MySocketProServer As New CMySocketProServer()\n");
	strDef += _T("\t\tIf Not MySocketProServer.Run(20901) Then\n");
	strDef += _T("\t\t\tConsole.WriteLine(\"Error code = \" & CSocketProServer.LastSocketError.ToString())\n");
	strDef += _T("\t\tEnd If\n");
	strDef += _T("\t\tConsole.WriteLine(\"Input a line to close the application ......\")\n");
	strDef += _T("\t\tDim str As String = Console.ReadLine()\n");
	strDef += _T("\t\tMySocketProServer.StopSocketProServer() 'or MySocketProServer.Dispose()\n");
	strDef += _T("\tEnd Sub\n\n");

	strDef += _T("End Class\n\n");

	::DeleteFile(m_strOutputFile);
	HANDLE hFile = ::CreateFile(m_strOutputFile, GENERIC_ALL, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		m_strErrorMessage.Format(_T("Error in opening the file %s"), m_strOutputFile);
		return false;
	}

	::WriteFile(hFile, (LPVOID)LPCTSTR(strDef), strDef.GetLength(), &ulWrite, NULL);
	::CloseHandle(hFile);
	return true;
}

bool CUidFile::CreateServerPeerFileCSharp()
{
	int n;
	ULONG ulWrite;
	CString strDef;
	CString strTemp;
	strDef += _T("/* **** including all of defines, service id(s) and request id(s) ***** */\n");
	strDef += _T("using System;\nusing SocketProAdapter;\nusing SocketProAdapter.ServerSide;\n\n");
	int nSvs = m_aService.GetSize();
	for(n=0; n<nSvs; n++)
	{
		CString strSvsImp;
		CUIDService &Svs = m_aService[n];
		int j;
		int jAll = Svs.GetMethods().GetSize();
		strDef += _T("//server implementation for service ");
		strDef += Svs.GetName();
		strDef += _T("\n");
		
		strDef += _T("public class ");
		strDef += Svs.GetName();
		if(Svs.GetSvsID() == SERVICE_ID_NOT_AVAILABLE)
		{
			CString str;
			str.Format(_T("Peer : %sPeer\n{\n"), Svs.GetBaseSvs());
			strDef += str;
		}
		else
		{
			strDef += _T("Peer : CClientPeer\n{\n");
		}
		
		if(Svs.GetSvsID() != SERVICE_ID_NOT_AVAILABLE)
		{
			strDef += _T("\tprotected override void OnSwitchFrom(uint nServiceID)\n\t{\n\t\t//initialize the object here\n\t}\n\n");
			strDef += _T("\tprotected override void OnReleaseResource(bool closing, uint nInfo)\n\t{\n\t\tif(closing)\n\t\t{\n\t\t\t//closing the socket with error code = nInfo\n\t\t}\n\t\telse\n\t\t{\n\t\t\t//switch to a new service with the service id = nInfo\n\t\t}\n\n\t\t//release all of your resources here as early as possible\n\t}\n\n");
		}
		for(j=0; j<jAll; j++)
		{
			CMethod &method = Svs.GetMethods()[j];
			strDef += method.GetMethodForPeerCSharp();
			strDef += _T("\n");
		}

		strDef += _T("}\n\n");
	}

	strDef += _T("public class CMySocketProServer : CSocketProServer\n{\n\n");
	strDef += _T("\tprotected override bool OnSettingServer()\n\t{\n\t\t//amIntegrated and amMixed not supported yet\n");
	strDef += _T("\t\tConfig.AuthenticationMethod = tagAuthenticationMethod.amOwn;\n\n");
	strDef += _T("\t\treturn true; //true -- ok; false -- no listening server\n\t}\n\n");
	strDef += _T("\tprotected override void OnAccept(ulong hSocket, int nError)\n\t{\n\t\t//when a socket is initially established\n\t}\n\n");
	strDef += _T("\tprotected override bool OnIsPermitted(ulong hSocket, string userId, string password, uint nSvsID)\n\t{\n\t\t//give permission to all\n");
	strDef += _T("\t\treturn true;\n\t}\n\n");
	strDef += _T("\tprotected override void OnClose(ulong hSocket, int nError)\n\t{\n\t\t//when a socket is closed\n\t}\n\n");
	
	nSvs = m_aService.GetSize();
	for(n=0; n<nSvs; n++)
	{
		CUIDService &Svs = m_aService[n];
		CUIDService LastSvs = FindLastDerived(Svs.GetName());
		if(LastSvs.GetName() == Svs.GetName())
		{
			CString str;
			str.Format(_T("\t[ServiceAttr(%s.sid%s)]\n"), m_strDefClass, Svs.GetName());
			strDef += str;
			strTemp.Format(_T("\tprivate CSocketProService<%sPeer> m_%s = new CSocketProService<%sPeer>();\n"), Svs.GetName(), Svs.GetName(), Svs.GetName());
			strDef += strTemp;
		}
	}
	strDef += _T("\t//One SocketPro server supports any number of services. You can list them here!\n\n");

	strDef += _T("\tstatic void Main(string[] args)\n");
	strDef += _T("\t{\n");
	strDef += _T("\t\tCMySocketProServer MySocketProServer = new CMySocketProServer();\n");
	strDef += _T("\t\tif (!MySocketProServer.Run(20901))\n");
	strDef += _T("\t\t\tConsole.WriteLine(\"Error code = \" + CSocketProServer.LastSocketError.ToString());\n");
	strDef += _T("\t\tConsole.WriteLine(\"Input a line to close the application ......\");\n");
	strDef += _T("\t\tstring str = Console.ReadLine();\n");
	strDef += _T("\t\tMySocketProServer.StopSocketProServer(); //or MySocketProServer.Dispose();\n");
	strDef += _T("\t}\n\n");
	strDef += _T("}\n\n");

	::DeleteFile(m_strOutputFile);
	HANDLE hFile = ::CreateFile(m_strOutputFile, GENERIC_ALL, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		m_strErrorMessage.Format(_T("Error in opening the file %s"), m_strOutputFile);
		return false;
	}

	::WriteFile(hFile, (LPVOID)LPCTSTR(strDef), strDef.GetLength(), &ulWrite, NULL);
	::CloseHandle(hFile);
	return true;
}

CUIDService CUidFile::FindLastDerived(LPCTSTR strSvsName)
{
	CUIDService SvsRtn;
	int n;
	int nSize = m_aService.GetSize();
	for(n=0; n<nSize; n++)
	{
		CUIDService &svs = m_aService[n];
		if(svs.GetName() == strSvsName || svs.GetBaseSvs() == SvsRtn.GetName())
		{
			SvsRtn = svs;
		}
	}
	return SvsRtn;
}

CUIDService CUidFile::FindRootService(LPCTSTR strSvsName)
{
	CUIDService SvsRtn;
	int n;
	int nSize = m_aService.GetSize();
	for(n=nSize-1; n>=0; n--)
	{
		CUIDService &Svs = m_aService[n];
		if(Svs.GetName() == strSvsName)
		{
			SvsRtn = Svs;
			strSvsName = Svs.GetBaseSvs();
		}
	}
	return SvsRtn;
}

CUIDService CUidFile::FindService(LPCTSTR strBaseSvsName)
{
	CUIDService SvsRtn;
	int n;
	int nSize = m_aService.GetSize();
	for(n=0; n<nSize; n++)
	{
		CUIDService &Svs = m_aService[n];
		if(Svs.GetName() == strBaseSvsName)
		{
			if(Svs.GetMethods().GetSize() != 0)
			{
				SvsRtn = Svs;
				break;
			}
			else
			{
				strBaseSvsName = Svs.GetBaseSvs();
			}
		}
	}
	return SvsRtn;
}

CSimpleArray<CMethod> CUidFile::GetAllSlowMethods(LPCTSTR strSvsName)
{
	CSimpleArray<CMethod> aMethod;
	int n;
	int nSize = m_aService.GetSize();
	for(n=nSize-1; n>=0; n--)
	{
		CUIDService &Svs = m_aService[n];
		if(Svs.GetName() == strSvsName)
		{
			int j;
			CSimpleArray<CMethod> aM = Svs.GetMethods();
			int jAll = aM.GetSize();
			for(j=0; j<jAll; j++)
			{
				CMethod &method = aM[j];
				if(method.IsSlow())
				{
					aMethod.Add(method);
				}
			}
			strSvsName = Svs.GetBaseSvs();
		}
	}
	return aMethod;
}

CString CUidFile::GetBaseClassName(bool bClient, LPCTSTR strMyClassName)
{
	CString str;
	int n;
	int nSize = m_aService.GetSize();
	for(n=0; n<nSize; n++)
	{
		CUIDService &Svs = m_aService[n];
		if(Svs.GetName() == strMyClassName)
		{
			str = Svs.GetBaseSvs();
			break;
		}
	}
	if(bClient)
	{
		if(str.GetLength() == 0)
		{
			str = _T("CAsyncServiceHandler");
		}
	}
	else
	{
		if(str.GetLength() == 0)
		{
			str = _T("CClientPeer");
		}
	}
	return str;
}

CString CUidFile::GetInitialRequestID(LPCTSTR strMyClassName)
{
	CString str;
	int n;
	int nSize = m_aService.GetSize();
	for(n=0; n<nSize; n++)
	{
		CUIDService &Svs = m_aService[n];
		if(Svs.GetName() == strMyClassName)
		{
			str = Svs.GetBaseSvs();
			if(str.GetLength() == 0)
			{
				switch(m_lang)
				{
				case lCpp:
					str = _T("SPA::idReservedTwo + 1");
					break;
				case lCSharp:
					str = _T("(ushort)SocketProAdapter.tagBaseRequestID.idReservedTwo + 1");
					break;
				case lVBNet:
					str = _T("SocketProAdapter.tagBaseRequestID.idReservedTwo + 1");
					break;
				default:
					ATLASSERT(FALSE);
					break;
				}
			}
			else
			{
				CUIDService BaseSvs = FindService(str);
				int nSize = BaseSvs.GetMethods().GetSize();
				switch(m_lang)
				{
				case lCpp:
					if(nSize == 0)
					{
						str = _T("SPA::idReservedTwo + 50");
					}
					else
					{
						CMethod &method = BaseSvs.GetMethods()[nSize - 1];
						str.Format(_T("id%s%s + 50"), method.GetName(), BaseSvs.GetName());
					}
					break;
				case lCSharp:
					if(nSize == 0)
					{
						str = _T("((ushort)SocketProAdapter.tagBaseRequestID.idReservedTwo + 50");
					}
					else
					{
						CMethod &method = BaseSvs.GetMethods()[nSize - 1];
						str.Format(_T("id%s%s + 50"), method.GetName(), BaseSvs.GetName());
					}
					break;
				case lVBNet:
					if(nSize == 0)
					{
						str = _T("(SocketProAdapter.tagBaseRequestID.idReservedTwo + 50");
					}
					else
					{
						CMethod &method = BaseSvs.GetMethods()[nSize - 1];
						str.Format(_T("id%s%s + 50"), method.GetName(), BaseSvs.GetName());
					}
					break;
				default:
					ATLASSERT(FALSE);
					break;
				}
			}
		}
	}
	return str;
}

bool CUidFile::CreateServerPeerFileH()
{
	int n;
	ULONG ulWrite;
	CString strInc;
	CString strNum;
	CString strDef(_T("#ifndef "));
	CString strTemp = m_strOutputFile;
	int nSlash = strTemp.ReverseFind(_T('\\'));
	if(nSlash != -1)
	{
		strTemp = strTemp.Mid(nSlash + 1);
	}
	strTemp.Replace(_T('.'), _T('_'));
	strTemp.MakeUpper();
	strTemp += _T("__");
	strTemp = _T("___SOCKETPRO_SERVICES_IMPL_") + strTemp;

	strDef += strTemp;
	strDef += _T("\n");
	
	strDef += _T("#define ");
	strDef += strTemp;
	strDef += _T("\n\n");

	strDef += _T("#include \"aserverw.h\"\nusing namespace SPA;\nusing namespace SPA::ServerSide;\n\n");
	
	strDef += _T("/* **** including all of defines, service id(s) and request id(s) ***** */\n");

	nSlash = m_strDefFile.ReverseFind(_T('\\'));
	strInc = m_strDefFile.Mid(nSlash + 1);
	strInc = _T("#include \"") + strInc;
	strInc += _T("\"");

	strDef += strInc;
	strDef += _T("\n\n");

	int nSvs = m_aService.GetSize();
	for(n=0; n<nSvs; n++)
	{
		CString strSvsImp;
		CUIDService &Svs = m_aService[n];
		strDef += _T("//server implementation for service ");
		strDef += Svs.GetName();
		strDef += _T("\n");
		
		strDef += _T("class ");
		strDef += Svs.GetName();
		if(Svs.GetSvsID() != SERVICE_ID_NOT_AVAILABLE)
		{
			strDef += _T("Peer : public CClientPeer\n");
		}
		else
		{
			strDef += _T("Peer : public ");
			strDef += Svs.GetBaseSvs();
			strDef += _T("Peer\n");
		}
		strDef += _T("{\nprotected:\n");
		if(Svs.GetSvsID() != SERVICE_ID_NOT_AVAILABLE)
		{
			strDef += _T("\tvirtual void OnSwitchFrom(unsigned int serviceId)\n\t{\n\t\t//initialize the object here\n\t}\n\n");
			strDef += _T("\tvirtual void OnReleaseResource(bool closing, unsigned int info)\n\t{\n\t\tif(closing)\n\t\t{\n\t\t\t//closing the socket with error code = info\n\t\t}\n\t\telse\n\t\t{\n\t\t\t//switch to a new service with the service id = info\n\t\t}\n\n\t\t//release all of your resources here as early as possible\n\t}\n\n");
		}
		int j;
		int jAll = Svs.GetMethods().GetSize();
		for(j=0; j<jAll; j++)
		{
			CMethod &method = Svs.GetMethods()[j];
			strDef += method.GetMethodForPeerCpp();
			strDef += _T("\n");
		}

		int nCountOfSlows = Svs.GetCountOfSlowMethods();

		if(nCountOfSlows < Svs.GetMethods().GetSize() || Svs.GetSvsID() != SERVICE_ID_NOT_AVAILABLE)
		{
			strDef += _T("\tvirtual void OnFastRequestArrive(unsigned short reqId, unsigned int len)\n\t{\n");
			if(Svs.GetSvsID() == SERVICE_ID_NOT_AVAILABLE)
			{
				CString str;
				str.Format(_T("\t\t%sPeer::OnFastRequestArrive(reqId, len);\n"), Svs.GetBaseSvs());
				strDef += str;
			}
			strDef += Svs.GetPeerSwitchCpp(false);
			strDef += _T("\t}\n\n");
		}
		
		if(nCountOfSlows != 0 || Svs.GetSvsID() != SERVICE_ID_NOT_AVAILABLE)
		{
			strDef += _T("\tvirtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len)\n\t{\n");
			if(Svs.GetSvsID() == SERVICE_ID_NOT_AVAILABLE)
			{
				CString str;
				str.Format(_T("\t\t%sPeer::OnSlowRequestArrive(reqId, len);\n"), Svs.GetBaseSvs());
				strDef += str;
			}
			strDef += Svs.GetPeerSwitchCpp(true);
			strDef += _T("\t\treturn 0;\n");
			strDef += _T("\t}\n\n");
		}

		strDef += _T("};\n\n");
	}
	
	strDef += _T("class CMySocketProServer : public CSocketProServer\n{\nprotected:\n");
	strDef += _T("\tvirtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6)\n\t{\n\t\t//amIntegrated and amMixed not supported yet\n\t\tCSocketProServer::Config::SetAuthenticationMethod(amOwn);\n\n");
	strDef += _T("\t\t//add service(s) into SocketPro server\n\t\tAddService();\n\t\treturn true; //true -- ok; false -- no listening server\n\t}\n\n");
	strDef += _T("\tvirtual void OnAccept(USocket_Server_Handle h, int errCode)\n\t{\n\t\t//when a socket is initially established\n\t}\n\n");
	strDef += _T("\tvirtual bool OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId)\n\t{\n\t\t//give permission to all\n");
	strDef += _T("\t\treturn true;\n\t}\n\n");
	strDef += _T("\tvirtual void OnClose(USocket_Server_Handle h, int errCode)\n\t{\n\t\t//when a socket is closed\n\t}\n\n");

	strDef += _T("private:\n");
	nSvs = m_aService.GetSize();
	for(n=0; n<nSvs; n++)
	{
		CUIDService &Svs = m_aService[n];
		CUIDService LastSvs = FindLastDerived(Svs.GetName());
		if(LastSvs.GetName() == Svs.GetName())
		{
			strTemp.Format(_T("\tCSocketProService<%sPeer> m_%s;\n"), Svs.GetName(), Svs.GetName());
			strDef += strTemp;
		}
	}
	strDef += _T("\t//One SocketPro server supports any number of services. You can list them here!\n");

	int nD = 0;
	strDef += _T("\nprivate:\n\tvoid AddService()\n\t{\n\t\tbool ok;\n");
	for(n=0; n<nSvs; n++)
	{
		int j;
		int jAll;
		CUIDService &Svs = m_aService[n];
		CUIDService LastSvs = FindLastDerived(Svs.GetName());
		if(LastSvs.GetName() != Svs.GetName())
			continue;
		nD++;
		if(nD)
		{
			strDef += _T("\n");
		}
		strDef += _T("\t\t//No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree\n");
		
		CUIDService RootSvs = FindRootService(Svs.GetName());
		strTemp.Format(_T("\t\tok = m_%s.AddMe(sid%s, taNone);\n"), Svs.GetName(), RootSvs.GetName());
		strDef += strTemp;
		strDef += _T("\t\t//If ok is false, very possibly you have two services with the same service id!\n\n");
		CSimpleArray<CMethod> aMethod = GetAllSlowMethods(Svs.GetName());
		jAll = aMethod.GetSize();
		for(j=0; j<jAll; j++)
		{
			CMethod &method = aMethod[j];
			if(method.IsSlow())
			{
				strTemp.Format(_T("\t\tok = m_%s.AddSlowRequest(id%s%s);\n"), Svs.GetName(), method.GetName(), method.GetServiceName());
				strDef += strTemp;
			}
		}
	}
	
	strDef += _T("\n\t\t//Add all of other services into SocketPro server here!\n");

	strDef += _T("\t}\n");

	strDef +=_T("};\n\n");

	strDef += _T("\n#endif");
	
	::DeleteFile(m_strOutputFile);
	HANDLE hFile = ::CreateFile(m_strOutputFile, GENERIC_ALL, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		m_strErrorMessage.Format(_T("Error in opening the file %s"), m_strOutputFile);
		return false;
	}

	::WriteFile(hFile, (LPVOID)LPCTSTR(strDef), strDef.GetLength(), &ulWrite, NULL);
	::CloseHandle(hFile);
	return true;
}

bool CUidFile::CreateClientFileVBNet()
{
	int n;
	ULONG ulWrite;
	CString strTemp;
	CString strComment;
	CString strDefClass;
	int nSlash = m_strOutputFile.ReverseFind(_T('\\'));
	if(nSlash != -1)
	{
		strTemp = m_strOutputFile.Mid(nSlash + 1);
	}
	else
	{
		strTemp = m_strOutputFile;
	}
	int nDot = strTemp.Find(_T('.'));
	if(nDot != -1)
	{
		strTemp = strTemp.Left(nDot);
	}
	strDefClass.Format(_T("%sConst"), strTemp);
	m_strDefClass = strDefClass;

	CString strDef(_T("Imports System\nImports SocketProAdapter\nImports SocketProAdapter.ClientSide\n\n"));
	
	int nSvs = m_aService.GetSize();
	for(n=0; n<nSvs; n++)
	{
		if(n)
		{
			strDef += _T("\n");
		}
		CUIDService &Svs = m_aService[n];
		if(Svs.GetSvsID() == SERVICE_ID_NOT_AVAILABLE)
		{
			strTemp.Format(_T("public class %s : Inherits %s\n"), Svs.GetName(), Svs.GetBaseSvs());
		}
		else
		{
			strTemp.Format(_T("public class %s : Inherits CAsyncServiceHandler\n"), Svs.GetName());
		}
		strDef += strTemp;
		int j;
		int jAll = Svs.GetMethods().GetSize();
		if(Svs.GetSvsID() != SERVICE_ID_NOT_AVAILABLE)
		{
			strTemp = _T("\tPublic Sub New()\n\t\tMyBase.New({?}.sid{b})\n\tEnd Sub\n\n");
			strTemp.Replace(_T("{?}"), strDefClass);
			strTemp.Replace(_T("{b}"), Svs.GetName());
			strDef += strTemp;
		}

		for(j = 0; j<jAll; j++)
		{
			CMethod &mb = Svs.GetMethods()[j];
			mb.SetDefClass(m_strDefClass);
			strDef += mb.GetSynFunVBNetNew();
			if(j != (jAll-1))
				strDef += _T("\n");
		}

		strDef += _T("End Class\n");
	}
	
	::DeleteFile(m_strOutputFile);
	HANDLE hFile = ::CreateFile(m_strOutputFile, GENERIC_ALL, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		m_strErrorMessage.Format(_T("Error in opening the file %s"), m_strOutputFile);
		return false;
	}

	::WriteFile(hFile, (LPVOID)LPCTSTR(strDef), strDef.GetLength(), &ulWrite, NULL);
	::CloseHandle(hFile);
	return true;
}

bool CUidFile::CreateClientFileCSharp()
{
	int n;
	ULONG ulWrite;
	CString strTemp;
	CString strComment;
	CString strDefClass;
	int nSlash = m_strOutputFile.ReverseFind(_T('\\'));
	if(nSlash != -1)
	{
		strTemp = m_strOutputFile.Mid(nSlash + 1);
	}
	else
	{
		strTemp = m_strOutputFile;
	}
	int nDot = strTemp.Find(_T('.'));
	if(nDot != -1)
	{
		strTemp = strTemp.Left(nDot);
	}
	strDefClass.Format(_T("%sConst"), strTemp);
	m_strDefClass = strDefClass;

	CString strDef(_T("\n\nusing System;\nusing SocketProAdapter;\nusing SocketProAdapter.ClientSide;\n\n"));
	
	int nSvs = m_aService.GetSize();
	for(n=0; n<nSvs; n++)
	{
		if(n)
		{
			strDef += _T("\n");
		}
		CUIDService &Svs = m_aService[n];
		if(Svs.GetBaseSvs().GetLength() == 0)
			strTemp.Format(_T("public class %s : CAsyncServiceHandler\n{\n"), Svs.GetName());
		else
			strTemp.Format(_T("public class %s : %s\n{\n"), Svs.GetName(), Svs.GetBaseSvs());
		strDef += strTemp;
		
		if(Svs.GetBaseSvs().GetLength() == 0)
		{
			CString strC = _T("\tpublic {?}() : base({d}.sid{?})\n\t{\n\t}\n\n");
			
			strC.Replace(_T("{?}"), Svs.GetName());
			strC.Replace(_T("{d}"), strDefClass);

			strDef += strC;
		}
		
		int j;
		int jAll = Svs.GetMethods().GetSize();

		for(j = 0; j<jAll; j++)
		{
			if(j)
				strDef += _T("\n");
			CMethod &mb = Svs.GetMethods()[j];
			mb.SetDefClass(m_strDefClass);
			strDef += mb.GetSynFunCSharpNew();
		}
		strDef += _T("}\n");
	}
	

	::DeleteFile(m_strOutputFile);
	HANDLE hFile = ::CreateFile(m_strOutputFile, GENERIC_ALL, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		m_strErrorMessage.Format(_T("Error in opening the file %s"), m_strOutputFile);
		return false;
	}

	::WriteFile(hFile, (LPVOID)LPCTSTR(strDef), strDef.GetLength(), &ulWrite, NULL);
	::CloseHandle(hFile);
	return true;
}

bool CUidFile::CreateClientHandlerFileH()
{
	int n;
	ULONG ulWrite;
	CString strInc;
	CString strNum;
	CString strDef(_T("#ifndef "));
	CString strTemp = m_strOutputFile;
	int nSlash = strTemp.ReverseFind(_T('\\'));
	if(nSlash != -1)
	{
		strTemp = strTemp.Mid(nSlash + 1);
	}
	strTemp.Replace(_T('.'), _T('_'));
	strTemp.MakeUpper();
	strTemp += _T("__");
	strTemp = _T("___SOCKETPRO_CLIENT_HANDLER_") + strTemp;

	strDef += strTemp;
	strDef += _T("\n");
	
	strDef += _T("#define ");
	strDef += strTemp;
	strDef += _T("\n\n");

	strDef += _T("#include \"aclientw.h\"\nusing namespace SPA;\nusing namespace SPA::ClientSide;\n\n");

	strDef += _T("/* **** including all of defines, service id(s) and request id(s) ***** */\n");

	nSlash = m_strDefFile.ReverseFind(_T('\\'));
	strInc = m_strDefFile.Mid(nSlash + 1);
	strInc = _T("#include \"") + strInc;
	strInc += _T("\"");

	strDef += strInc;
	strDef += _T("\n\n");
	
	int nSvs = m_aService.GetSize();
	for(n=0; n<nSvs; n++)
	{
		CUIDService &Svs = m_aService[n];
		int j;
		int jAll = Svs.GetMethods().GetSize();
		strDef += _T("//client handler for service ");
		strDef += Svs.GetName();
		strDef += _T("\n");
		
		strDef += _T("class ");
		strDef += Svs.GetName();
		CString strBaseName;

		if(Svs.GetSvsID() == SERVICE_ID_NOT_AVAILABLE)
		{
			strDef += _T(" : public ");
			strDef += Svs.GetBaseSvs();
			strDef += _T("\n");
			strBaseName = Svs.GetBaseSvs();
		}
		else
		{
			strDef += _T(" : public CAsyncServiceHandler\n");
			strBaseName = _T("CAsyncServiceHandler");
		}
		strDef += _T("{\n");
		
		if(Svs.GetSvsID() != SERVICE_ID_NOT_AVAILABLE)
		{
			strDef += _T("public:\n");
			CString strTemp(_T("\t{?}(CClientSocket *pClientSocket)\n\t: {B}(sid{?}, pClientSocket)\n\t{\n\t}\n\n"));
			strTemp.Replace(_T("{?}"), Svs.GetName());
			strTemp.Replace(_T("{B}"), strBaseName);

			/*strDef += _T("\tvirtual unsigned int GetSvsID()\n");
			strDef += _T("\t{\n");
			strDef += _T("\t\treturn sid");
			strDef += Svs.GetName();
			strDef += _T(";\n\t}\n\n");*/
			strDef += strTemp;

		}
		if(jAll > 0 || Svs.GetSvsID() != SERVICE_ID_NOT_AVAILABLE)
		{
			strDef += _T("public:\n");
		}

		/*for(j = 0; j<jAll; j++)
		{
			CMethod &mb = Svs.GetMethods()[j];
			CString strM = mb.GetClientAsynInputMethodCpp();
			strDef += _T("\t");
			strDef += strM;
			strDef += _T("\n\n");
		}*/
		//if(Svs.GetSvsID() != SERVICE_ID_NOT_AVAILABLE || Svs.GetMethods().GetSize() > 0)
		//{
		//	//for processing returning results
		//	strDef += _T("\t//We can process returning results inside the function.\n");
		//	strDef += _T("\tvirtual void OnResultReturned(unsigned short reqId, CUQueue &UQueue)\n\t{\n");
		//	if(Svs.GetSvsID() == SERVICE_ID_NOT_AVAILABLE)
		//	{
		//		CString str;
		//		str.Format(_T("\t\t%s::OnResultReturned(reqId, UQueue);\n"), Svs.GetBaseSvs());
		//		strDef += str;
		//	}
		//	//strDef += _T("\t\tif(m_err.m_hr != 0) return; //exception transfered from SocketPro server\n");
		//	strDef += _T("\t\tswitch(reqId)\n\t\t{\n");
		//	for(j = 0; j<jAll; j++)
		//	{
		//		CMethod &mb = Svs.GetMethods()[j];
		//		if(mb.GetReturn() == dtVoid && mb.GetOutputCount() == 0)
		//			continue;
		//		strDef += mb.GetRtnProcessing();
		//	}
		//	strDef += _T("\t\tdefault:\n\t\t\tbreak;\n\t\t}\n\t}\n");
		//	strDef += _T("\npublic:\n");
		//}
		
		for(j = 0; j<jAll; j++)
		{
			if(j)
			{
				strDef += _T("\n");
			}
			CMethod &mb = Svs.GetMethods()[j];
			mb.SetDefClass(m_strDefClass);
			strDef += mb.GetSynFunCppNew();
		}

		strDef += _T("};\n");
	}
	strDef += _T("#endif\n");
	
	::DeleteFile(m_strOutputFile);
	HANDLE hFile = ::CreateFile(m_strOutputFile, GENERIC_ALL, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		m_strErrorMessage.Format(_T("Error in opening the file %s"), m_strOutputFile);
		return false;
	}

	::WriteFile(hFile, (LPVOID)LPCTSTR(strDef), strDef.GetLength(), &ulWrite, NULL);
	::CloseHandle(hFile);
	return true;
}

bool CUidFile::CreateVC6OtherFiles()
{
	DWORD ulWrite;
	HANDLE hFile;
	CString strFile;
	CString strDef;
/*
	strFile = _T("StdAfx.h");
	strDef = _T("\n#pragma once\n");
	strDef += _T("#include <iostream>\n");
	strDef += _T("using namespace std;\n\n");

	hFile = ::CreateFile(strFile, GENERIC_ALL, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		m_strErrorMessage.Format(_T("Error in opening the file %s"), strFile);
		return false;
	}
	::WriteFile(hFile, (LPVOID)LPCTSTR(strDef), strDef.GetLength(), &ulWrite, NULL);
	::CloseHandle(hFile);*/

	strFile = m_strProjName + _T(".cpp");
	strDef = _T("\n#include \"stdafx.h\"\n");
	strDef += _T("#include \"");
	strDef += m_strOutputFile;
	strDef += _T("\"\n\n");
	strDef += _T("int main(int argc, char* argv[])\n");
	strDef += _T("{\n");

	strDef += _T("\tCMySocketProServer	MySocketProServer;\n");
	strDef += _T("\tif(!MySocketProServer.Run(20901))\n");
	strDef += _T("\t{\n");
	strDef += _T("\t\tint errCode = MySocketProServer.GetErrorCode();\n");
	strDef += _T("\t\tstd::cout<<\"Error happens with code = \"<< errCode <<std::endl;\n");
	strDef += _T("\t}\n");

	strDef += _T("\tstd::cout << \"Press any key to stop the server ......\"<<std::endl;\n");
	strDef += _T("\t::getchar();\n");
	strDef += _T("\treturn 0;\n");
	strDef += _T("}\n");

	hFile = ::CreateFile(strFile, GENERIC_ALL, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		m_strErrorMessage.Format(_T("Error in opening the file %s"), strFile);
		return false;
	}

	::WriteFile(hFile, (LPVOID)LPCTSTR(strDef), strDef.GetLength(), &ulWrite, NULL);
	::CloseHandle(hFile);

	return true;
}

bool CUidFile::CreateVC6Proj(LPCTSTR strSpWrapCppFile)
{
	DWORD ulWrite;
	CString strDef = _T("# Microsoft Developer Studio Project File - Name=\"");
	strDef += m_strProjName;
	strDef += _T("\" - Package Owner=<4>\n");
	strDef += _T("# Microsoft Developer Studio Generated Build File, Format Version 6.00\n");
	strDef += _T("# ** DO NOT EDIT **\n\n");
	strDef += _T("# TARGTYPE \"Win32 (x86) Console Application\" 0x0103\n\n");
	strDef += _T("CFG=");
	strDef += m_strProjName;
	strDef += _T(" - Win32 Debug\n");
	strDef += _T("!MESSAGE This is not a valid makefile. To build this project using NMAKE,\n");
	strDef += _T("!MESSAGE use the Export Makefile command and run\n");
	strDef += _T("!MESSAGE \n");
	strDef += _T("!MESSAGE NMAKE /f \"");
	strDef += m_strProjName;
	strDef += _T(".mak\".\n");
	strDef += _T("!MESSAGE \n");
	strDef += _T("!MESSAGE You can specify a configuration when running NMAKE\n");
	strDef += _T("!MESSAGE by defining the macro CFG on the command line. For example:\n");
	strDef += _T("!MESSAGE \n");
	strDef += _T("!MESSAGE NMAKE /f \"");
	strDef += m_strProjName;
	strDef += _T(".mak\" CFG=\"");
	strDef += m_strProjName;
	strDef += _T(" - Win32 Debug\"\n");
	strDef += _T("!MESSAGE \n");
	strDef += _T("!MESSAGE Possible choices for configuration are:\n");
	strDef += _T("!MESSAGE \n");
	strDef += _T("!MESSAGE \"");
	strDef += m_strProjName;
	strDef += _T(" - Win32 Release\" (based on \"Win32 (x86) Console Application\")\n");
	strDef += _T("!MESSAGE \"");
	strDef += m_strProjName;
	strDef += _T(" - Win32 Debug\" (based on \"Win32 (x86) Console Application\")\n");
	strDef += _T("!MESSAGE \n\n");
	
	strDef += _T("# Begin Project\n");
	strDef += _T("# PROP AllowPerConfigDependencies 0\n");
	strDef += _T("# PROP Scc_ProjName \"\"\n");
	strDef += _T("# PROP Scc_LocalPath \"\"\n");
	strDef += _T("CPP=cl.exe\n");
	strDef += _T("RSC=rc.exe\n\n");
	strDef += _T("!IF  \"$(CFG)\" == \"");
	strDef += m_strProjName;
	strDef += _T(" - Win32 Release\"\n\n");
	strDef += _T("# PROP BASE Use_MFC 0\n");
	strDef += _T("# PROP BASE Use_Debug_Libraries 0\n");
	strDef += _T("# PROP BASE Output_Dir \"Release\"\n");
	strDef += _T("# PROP BASE Intermediate_Dir \"Release\"\n");
	strDef += _T("# PROP BASE Target_Dir \"\"\n");
	strDef += _T("# PROP Use_MFC 0\n");
	strDef += _T("# PROP Use_Debug_Libraries 0\n");
	strDef += _T("# PROP Output_Dir \"Release\"\n");
	strDef += _T("# PROP Intermediate_Dir \"Release\"\n");
	strDef += _T("# PROP Ignore_Export_Lib 0\n");
	strDef += _T("# PROP Target_Dir \"\"\n");
	strDef += _T("# ADD BASE CPP /nologo /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_CONSOLE\" /D \"_MBCS\" /Yu\"stdafx.h\" /FD /c\n");
	strDef += _T("# ADD CPP /nologo /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_CONSOLE\" /D \"_MBCS\" /Yu\"stdafx.h\" /FD /c\n");
	strDef += _T("# ADD BASE RSC /l 0x409 /d \"NDEBUG\"\n");
	strDef += _T("# ADD RSC /l 0x409 /d \"NDEBUG\"\n");
	strDef += _T("BSC32=bscmake.exe\n");
	strDef += _T("# ADD BASE BSC32 /nologo\n");
	strDef += _T("# ADD BSC32 /nologo\n");
	strDef += _T("LINK32=link.exe\n");
	strDef += _T("# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386\n");
	strDef += _T("# ADD LINK32 /nologo /subsystem:console /machine:I386\n\n");
	strDef += _T("!ELSEIF  \"$(CFG)\" == \"");
	strDef += m_strProjName;
	strDef += _T(" - Win32 Debug\"\n\n");
	strDef += _T("# PROP BASE Use_MFC 0\n");
	strDef += _T("# PROP BASE Use_Debug_Libraries 1\n");
	strDef += _T("# PROP BASE Output_Dir \"Debug\"\n");
	strDef += _T("# PROP BASE Intermediate_Dir \"Debug\"\n");
	strDef += _T("# PROP BASE Target_Dir \"\"\n");
	strDef += _T("# PROP Use_MFC 0\n");
	strDef += _T("# PROP Use_Debug_Libraries 1\n");
	strDef += _T("# PROP Output_Dir \"Debug\"\n");
	strDef += _T("# PROP Intermediate_Dir \"Debug\"\n");
	strDef += _T("# PROP Ignore_Export_Lib 0\n");
	strDef += _T("# PROP Target_Dir \"\"\n");
	

	strDef += _T("# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_CONSOLE\" /D \"_MBCS\" /Yu\"stdafx.h\" /FD /GZ /c\n");
	strDef += _T("# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_CONSOLE\" /D \"_MBCS\" /Yu\"stdafx.h\" /FD /GZ /c\n");
	strDef += _T("# ADD BASE RSC /l 0x409 /d \"_DEBUG\"\n");
	strDef += _T("# ADD RSC /l 0x409 /d \"_DEBUG\"\n");
	strDef += _T("BSC32=bscmake.exe\n");
	strDef += _T("# ADD BASE BSC32 /nologo\n");
	strDef += _T("# ADD BSC32 /nologo\n");
	strDef += _T("LINK32=link.exe\n");
	strDef += _T("# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept\n");
	strDef += _T("# ADD LINK32 /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept\n\n");

	strDef += _T("!ENDIF \n\n");
	strDef += _T("# Begin Target\n\n");
	strDef += _T("# Name \"");
	strDef += m_strProjName;
	strDef += _T(" - Win32 Release\"\n");
	strDef += _T("# Name \"");
	strDef += m_strProjName;
	strDef += _T(" - Win32 Debug\"\n");
	strDef += _T("# Begin Group \"Source Files\"\n\n");
	strDef += _T("# PROP Default_Filter \"cpp;c;cxx;rc;def;r;odl;idl;hpj;bat\"\n");
	strDef += _T("# Begin Source File\n\n");
	strDef += _T("SOURCE=\"");
	strDef += strSpWrapCppFile;
	strDef += _T("\"\n");
	strDef += _T("# End Source File\n");
	strDef += _T("# Begin Source File\n\n");
	strDef += _T("SOURCE=.\\");
	strDef += m_strProjName;
	strDef += _T(".cpp\n");
	strDef += _T("# End Source File\n");
	strDef += _T("# End Group\n");
	strDef += _T("# Begin Group \"Header Files\"\n\n");
	strDef += _T("# PROP Default_Filter \"h;hpp;hxx;hm;inl\"\n");
	strDef += _T("# Begin Source File\n\n");
	strDef += _T("SOURCE=.\\StdAfx.h\n");
	strDef += _T("# End Source File\n");
	strDef += _T("# Begin Source File\n\n");
	strDef += _T("SOURCE=.\\");
	strDef += m_strOutputFile;
	strDef += _T("\n");
	strDef += _T("# End Source File\n");
	strDef += _T("# End Group\n");
	strDef += _T("# End Target\n");
	strDef += _T("# End Project\n\n");

	CString strFile = m_strProjName + _T(".dsp");

	HANDLE hFile = ::CreateFile(strFile, GENERIC_ALL, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		m_strErrorMessage.Format(_T("Error in opening the file %s"), strFile);
		return false;
	}

	::WriteFile(hFile, (LPVOID)LPCTSTR(strDef), strDef.GetLength(), &ulWrite, NULL);
	::CloseHandle(hFile);
	return true;
}

bool CUidFile::CreateCommonDefFileH()
{
	int n;
	ULONG ulWrite;
	CString strNum;
	CString strDef(_T("#ifndef "));
	CString strTemp = m_strDefFile;
	int nSlash = strTemp.ReverseFind(_T('\\'));
	if(nSlash != -1)
	{
		strTemp = strTemp.Mid(nSlash + 1);
	}
	strTemp.Replace(_T('.'), _T('_'));
	strTemp.MakeUpper();
	strTemp += _T("__");
	strTemp = _T("___SOCKETPRO_DEFINES_") + strTemp;

	strDef += strTemp;
	
	strDef += _T("\n#define ");
	strDef += strTemp; 
	strDef += _T("\n\n");

	int nSvs = m_aService.GetSize();
	for(n=0; n<nSvs; n++)
	{
		CUIDService &Svs = m_aService[n];
		if(Svs.GetSvsID() != SERVICE_ID_NOT_AVAILABLE)
		{
			strDef += _T("//defines for service ");
			strDef += Svs.GetName();
			strDef += _T("\n#define ");
			strDef += _T("sid");
			strDef += Svs.GetName();
			strDef += _T("\t(SPA::sidReserved + ");
			strNum.Format(_T("%d"), Svs.GetSvsID());
			strDef += strNum;
			strDef += _T(")\n\n");
		}
		int j;
		CString strPrev = GetInitialRequestID(Svs.GetName());
		int jAll = Svs.GetMethods().GetSize();
		for(j = 0; j<jAll; j++)
		{
			CMethod &mb = Svs.GetMethods()[j];
			strDef += _T("#define ");
			strDef += _T("id");
			strDef += mb.GetName();
			strDef += Svs.GetName();
			strDef += _T("\t(");
			strDef += strPrev;
			if(j == 0)
			{
				strDef += _T(")\n");
			}
			else
			{
				strDef += _T(" + 1)\n");
			}
			strPrev.Format(_T("id%s%s"), mb.GetName(), Svs.GetName());
		}
		strDef += _T("\n\n");
	}

	strDef += _T("\n#endif");
	
	::DeleteFile(m_strDefFile);
	HANDLE hFile = ::CreateFile(m_strDefFile, GENERIC_ALL, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		m_strErrorMessage.Format(_T("Error in opening the file %s"), m_strDefFile);
		return false;
	}

	::WriteFile(hFile, (LPVOID)LPCTSTR(strDef), strDef.GetLength(), &ulWrite, NULL);
	::CloseHandle(hFile);
	return true;
}

bool CUidFile::SetInputFile(LPCTSTR strInputFile)
{
	CString strTemp;
	CString strLeft;
	int nIndex2;
	CUIDService	UIDService;
	UIDService.SetLanguage(m_lang);
	m_aService.RemoveAll();
	bool bSuc = true;
	HANDLE hFile = ::CreateFile(strInputFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		m_strErrorMessage.Format(_T("Can't open the file %s"), strInputFile);
		return false;
	}

	ULONG lSize = ::GetFileSize(hFile, NULL);
	TCHAR *strDescription = new TCHAR[lSize + 1];
	::ReadFile(hFile, strDescription, lSize, &lSize, NULL);
	strDescription[lSize] = 0;

	CString strAll = strDescription;
	CString strService;

	strAll.Replace(cTab, cBlank);
	strAll.Replace(cLine, cBlank);
	
	//remove //
	int nIndex = strAll.Find(_T("//"));
	while(nIndex != -1)
	{
		nIndex2 = strAll.Find(cReturn, nIndex + 2);
		if(nIndex2 == -1)
		{
			strAll = strAll.Left(nIndex);
		}
		else
		{
			strTemp = strAll.Left(nIndex);
			strAll = strAll.Mid(nIndex2);
			strAll = strTemp + strAll;
		}
		nIndex = strAll.Find(_T("//")); 
	}
	
	strAll.Replace(cReturn, cBlank);

	//remove all /* and */
	do
	{
		nIndex = strAll.Find(_T("/*"));
		nIndex2 = strAll.Find(_T("*/"), nIndex + 2);
		if(nIndex == -1 && nIndex2 == -1)
		{
			break;
		}
		else if(nIndex2 > nIndex + 2)
		{
			strLeft = strAll.Left(nIndex);
			strAll = strAll.Mid(nIndex2 + 2); 
			strAll = strLeft + _T(" ") + strAll;
		}
		else
		{
			m_strErrorMessage = _T("Invalid service decalaration");
			delete []strDescription;
			::CloseHandle(hFile);
			return false;
			break;
		}
	}while(true);

	m_strCleanUID = strAll;

	if(!HandleIncludeFiles(strAll))
	{
		return false;
	}

	int nNext = strAll.Find(_T('}'));
	nNext += 1;
	while(nNext > 0)
	{
		strService = strAll.Left(nNext);
		strAll = strAll.Mid(nNext);

		if(UIDService.SetDeclaration(strService))
		{
			if(ExistingService(UIDService.GetName()))
			{
				m_strErrorMessage = _T("Two services can't have the same name");
				bSuc = false;
				break;
			}
			else if(ExistingService(UIDService.GetSvsID()))
			{
				m_strErrorMessage = _T("Two services can't have the same service id");
				bSuc = false;
				break;
			}
			else
			{
				m_aService.Add(UIDService);
			}
		}
		else
		{
			m_strErrorMessage = UIDService.GetErrorMessage();
			bSuc = false;
			break;
		}
		nNext = strAll.Find(_T('}'));
		nNext += 1;
	}

	delete []strDescription;
	::CloseHandle(hFile);
	if(bSuc)
	{
		m_strFileName = strInputFile;
	}
	return bSuc;
}
