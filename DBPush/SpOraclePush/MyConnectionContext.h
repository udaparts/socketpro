#pragma once

class CMyConnectionContext
{
public:
	CMyConnectionContext(void);
	~CMyConnectionContext(void);
	CMyConnectionContext(const CMyConnectionContext &cc);

public:
	CMyConnectionContext& operator=(const CMyConnectionContext &cc);

	CComBSTR		m_strHost;
	long			m_nPort;
	CComBSTR		m_strUID;
	CComBSTR		m_strPassword;
	short			m_EncrytionMethod;
	bool			m_bZip;
	CComBSTR		m_strLocation;
	bool			m_bStrict;
	long			m_VerifyCode;
};
