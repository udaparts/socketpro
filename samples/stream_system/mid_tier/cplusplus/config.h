
#pragma once

struct CConfig {
    CConfig();

    unsigned char m_main_threads;

    size_t m_nSlaveSessions;
    std::vector<SPA::ClientSide::CConnectionContext> m_vSlave;

	size_t m_nMasterSessions;
    SPA::ClientSide::CConnectionContext m_Master;
    std::string m_working_directory;
    std::string m_message_queue_password;

    unsigned int m_nStreamSQLPort;
    unsigned int m_nPort;
    bool m_bNoIpV6;

    std::vector<std::string> m_vCachedTable;

    void SetConfig();


private:
    //no copy constructor
    CConfig(const CConfig &config);
    //no assignment operator
    CConfig& operator=(const CConfig &config);
};

extern CConfig g_config;
