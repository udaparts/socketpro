#ifndef ___UDAPARTS_INTERFACE_DEFINITION_FILE_HHH_
#define ___UDAPARTS_INTERFACE_DEFINITION_FILE_HHH_

#include "uidservice.h"

#define	 UID_INCLUDE	_T("#include")

class CUidFile
{
public:
	CUidFile(tagLanguage lang);
	virtual ~CUidFile(void);

public:
	void SetLanguage(tagLanguage lang);
	bool SetInputFile(LPCTSTR strInputFile);
	bool GenerateFiles();
	LPCTSTR GetErrorMessage();
	
private:
	//C++
	bool CreateCommonDefFileH();
	bool CreateClientHandlerFileH();
	bool CreateServerPeerFileH();
	bool CreateVC6Proj(LPCTSTR strSpWrapCppFile);
	bool CreateVC6OtherFiles();
	
	//C#
	bool CreateDefFileCSharp();
	bool CreateClientFileCSharp();
	bool CreateServerPeerFileCSharp();

	//vb.net
	bool CreateDefFileVBNet();
	bool CreateClientFileVBNet();
	bool CreateServerPeerFileVBNet();

private:
	bool ExistingService(ULONG ulSvsID);
	bool ExistingService(LPCTSTR strSvsName);
	CString GetBaseClassName(bool bClient, LPCTSTR strMyClassName);
	CString GetInitialRequestID(LPCTSTR strMyClassName);
	CUIDService FindRootService(LPCTSTR strSvsName);
	CUIDService FindService(LPCTSTR strSvsName);
	CUIDService FindLastDerived(LPCTSTR strSvsName);
	CSimpleArray<CMethod> GetAllSlowMethods(LPCTSTR strSvsName);
	bool HandleIncludeFiles(CString &strUID);

private:
	tagLanguage		m_lang;
	CString			m_strNameSpace;
	CString			m_strFileName;
	CString			m_strOutputFile;
	CString			m_strDefFile;
	CString			m_strDefClass;
	CString			m_strErrorMessage;
	CSimpleArray<CUIDService> m_aService;
	CString			m_strCleanUID;
	CString			m_strProjName;
};


#endif
