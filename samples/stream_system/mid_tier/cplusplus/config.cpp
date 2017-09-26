
#include "stdafx.h"
#include "config.h"

CConfig g_config;

CConfig::CConfig()
: m_main_threads(1),
m_nSlaveThreads(4),
m_nStreamSQLPort(20902),
m_nPort(20901),
m_bNoIpV6(false) {
}

void CConfig::GetConfig(CConfig &config) {

}

