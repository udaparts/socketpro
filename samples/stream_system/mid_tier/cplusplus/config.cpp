
#include "stdafx.h"
#include "config.h"

CConfig g_config;

CConfig::CConfig()
	:m_nMasterSessions(2),
	m_nSlaveSessions(0),
	m_main_threads(1),
	m_nPort(20901),
	m_bNoIpV6(false) {
}

void CConfig::SetConfig() {
	m_main_threads = 4;

	//master
	m_ccMaster.Host = "localhost";
	m_ccMaster.UserId = L"root";
	m_ccMaster.Password = L"Smash123";
	m_ccMaster.Port = 20902;
	m_nMasterSessions = 2; //two sessions enough

	//slave
	CConnectionContext cc = m_ccMaster;
	m_vccSlave.push_back(cc);
	m_vccSlave.push_back(cc);
	//treat master as last salve
	m_vccSlave.push_back(m_ccMaster);
	m_nSlaveSessions = 12;


#ifdef WIN32_64
	m_working_directory = "c:\\sp_test";
#else
	m_working_directory = "/home/yye/sp_test/";
#endif
}

