
#pragma once

struct CConfig {
    CConfig();

    //master
    std::wstring m_master_default_db;
    size_t m_nMasterSessions;
    SPA::ClientSide::CConnectionContext m_ccMaster;
    std::string m_master_queue_name;

    //slave
    std::wstring m_slave_default_db;
    size_t m_nSlaveSessions;
    std::vector<SPA::ClientSide::CConnectionContext> m_vccSlave;
    std::string m_slave_queue_name;

    //middle tier server
    unsigned char m_main_threads;
    unsigned int m_nPort;
    bool m_bNoIpV6;

    //secure communication between front and middle tier
#ifdef WIN32_64
    std::string m_store_or_pfx;
#else
    std::string m_cert; //in PEM
    std::string m_key; //in PEM
#endif
    std::string m_password_or_subject;

    std::vector<std::wstring> m_vFrontCachedTable;

    void GetConfig();

private:
    //no copy constructor
    CConfig(const CConfig &config);
    //no assignment operator
    CConfig& operator=(const CConfig &config);
};

extern CConfig g_config;
