
#include "stdafx.h"
#include "config.h"

CConfig g_config;

CConfig::CConfig()
: m_nMasterSessions(2),
m_slave_threads(1),
m_sessions_per_host(2),
m_main_threads(1),
m_nPort(20911),
m_bNoIpV6(false) {
}

void CConfig::GetConfig() {
    //load the following settings from a configuration file
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

#if defined(_UMYSQL_SOCKETPRO_H_)
    m_nMasterSessions = 2; //two sessions enough
#else
    m_nMasterSessions = 1; //one session enough
#endif

    //slave
#if defined(_UMYSQL_SOCKETPRO_H_)
    m_slave_default_db = L"sakila";
#else
    m_slave_default_db = L"sakila.db";
#endif
    m_slave_queue_name = "db_sakila";
    SPA::ClientSide::CConnectionContext cc = m_ccMaster;
    cc.Host = "104.154.160.127";
    m_vccSlave.push_back(cc);
    //treat master as last salve
    m_vccSlave.push_back(m_ccMaster);

    m_slave_threads = 2;
    m_sessions_per_host = 3;

    //middle tier
    //test certificate and private key files are located at the directory ../socketpro/bin
#ifdef WIN32_64
    m_store_or_pfx = "intermediate.pfx";
#else
    m_cert = "intermediate.cert.pem";
    m_key = "intermediate.key.pem";
#endif
    m_password_or_subject = "mypassword";

#if defined(_UMYSQL_SOCKETPRO_H_)
    //cached tables on front applications
    m_vFrontCachedTable.push_back(L"sakila.actor");
    m_vFrontCachedTable.push_back(L"sakila.language");
    m_vFrontCachedTable.push_back(L"sakila.country");
    m_vFrontCachedTable.push_back(L"mysqldb.employee");
#else
    //cached tables on front applications
    m_vFrontCachedTable.push_back(L"actor");
    m_vFrontCachedTable.push_back(L"language");
    m_vFrontCachedTable.push_back(L"country");
#endif
}
