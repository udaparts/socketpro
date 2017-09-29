
#include "stdafx.h"
#include "config.h"

CConfig g_config;

CConfig::CConfig()
: m_main_threads(1),
m_nSlaveSessions(0),
m_nMasterSessions(2),
m_nStreamSQLPort(20902),
m_nPort(20901),
m_bNoIpV6(false) {
}

void CConfig::SetConfig() {
    m_main_threads = 4;

    //master
    m_Master.Host = "localhost";
    m_Master.UserId = L"root";
    m_Master.Password = L"Smash123";
    m_Master.Port = m_nStreamSQLPort;
    m_nMasterSessions = 2;

    CConnectionContext cc = m_Master;

    m_vSlave.push_back(cc);
    m_vSlave.push_back(cc);

    m_vSlave.push_back(m_Master);
    m_nSlaveSessions = 6;

#ifdef WIN32_64
    m_working_directory = "c:\\sp_test";
#else
    m_working_directory = "/home/yye/sp_test/";
#endif

    if (!m_nMasterSessions)
        m_nMasterSessions = 2;
    if (!m_nSlaveSessions && m_vSlave.size())
        m_nSlaveSessions = m_vSlave.size();
}

