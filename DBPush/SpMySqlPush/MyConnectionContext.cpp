#include "StdAfx.h"
#include "MyConnectionContext.h"

CMyConnectionContext	g_cc;

CMyConnectionContext::CMyConnectionContext(void) : m_nPort(0), m_EncrytionMethod(0), m_bZip(false), m_bStrict(false), m_VerifyCode(0)
{

}

CMyConnectionContext::~CMyConnectionContext(void)
{

}

CMyConnectionContext& CMyConnectionContext::operator=(const CMyConnectionContext &cc)
{
	m_bZip = cc.m_bZip;
	m_strHost = cc.m_strHost;
	m_nPort = cc.m_nPort;
	m_strPassword = cc.m_strPassword;
	m_strUID = cc.m_strUID;
	m_EncrytionMethod = cc.m_EncrytionMethod;
	m_bStrict = cc.m_bStrict;
	m_strLocation = cc.m_strLocation;
	m_VerifyCode = cc.m_VerifyCode;
	return *this;
}
