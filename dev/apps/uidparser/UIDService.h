#ifndef ___UID_SERVICE_H___
#define ___UID_SERVICE_H___

#include "method.h"

#define strServiceIDToken				_T("[ServiceID=")
#define SERVICE_ID_NOT_AVAILABLE		(0xFFFFFFFF)

class CUIDService
{
public:
	CUIDService(long lSvsID = 0);
	CUIDService(const CUIDService& UIDService);
	virtual ~CUIDService();

public:
	CUIDService& operator=(const CUIDService& UIDService);
	
public:
	const CString& GetName(){return m_strName;}
	tagLanguage GetLanguage(){return m_lang;}
	const CString& GetErrorMessage(){return m_strErrorMessage;}
	const CString& GetDeclaration(){return m_strDeclaration;}
	ULONG GetSvsID();
	const CString& GetBaseSvs(){return m_strBaseSvs;}
	
	void SetLanguage(tagLanguage lang){m_lang = lang;}
	CSimpleArray<CMethod>& GetMethods(){return m_aMethods;}
	bool AlreadyExist(const CString &strMethodName);
	int GetCountOfSlowMethods();
	
public:
	CString GetPeerSwitchVBNet(bool bSlow);
	CString GetPeerSwitchCpp(bool bSlow);
	CString GetPeerSwitchCSharp(bool bSlow);
	bool SetDeclaration(LPCTSTR strDecalaration);

private:
	tagLanguage				m_lang;
	CString					m_strName;
	ULONG					m_lSvsID;
	CString					m_strDeclaration;
	CSimpleArray<CMethod>	m_aMethods;
	CString					m_strErrorMessage;
	CString					m_strBaseSvs;
};

#endif
