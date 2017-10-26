
#include "stdafx.h"
#include "config.h"

CConfig g_config;

CConfig::CConfig()
	: m_nMasterSessions(2),
	m_nSlaveSessions(0),
	m_main_threads(1),
	m_nPort(20911),
	m_bNoIpV6(false) {
}

void CConfig::SetConfig() {
	m_main_threads = 4;

	//master
#if defined(_UMYSQL_SOCKETPRO_H_)
	m_ccMaster.Port = 20902;
	m_master_default_db = L"sakila";
#else
	m_ccMaster.Port = 20901;
	m_master_default_db = L"sakila.db";
#endif
	m_ccMaster.Host = "localhost";
	m_ccMaster.UserId = L"root";
	m_ccMaster.Password = L"Smash123";

	m_nMasterSessions = 2; //two sessions enough


	//slave
#if defined(_UMYSQL_SOCKETPRO_H_)
	m_slave_default_db = L"sakila";
#else
	m_slave_default_db = L"sakila.db";
#endif
	SPA::ClientSide::CConnectionContext cc = m_ccMaster;
	cc.Host = "104.154.160.127";
	m_vccSlave.push_back(cc);
	//treat master as last salve
	m_vccSlave.push_back(m_ccMaster);
	m_nSlaveSessions = 4;

	//middle tier
	//test certificate and private key files are located at the directory ../socketpro/bin
#ifdef WIN32_64
	m_store_or_pfx = "intermediate.pfx";
#else
	m_cert = "intermediate.cert.pem";
	m_key = "intermediate.key.pem";
#endif
	m_password_or_subject = "mypassword";

	//cached tables on front applications
	m_vFrontCachedTable.push_back(L"sakila.actor");
	m_vFrontCachedTable.push_back(L"sakila.language");
	m_vFrontCachedTable.push_back(L"sakila.country");
	m_vFrontCachedTable.push_back(L"mysqldb.employee");
}
