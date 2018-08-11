#ifndef MARIADB_SP_STREAMING_DB_CONFIG
#define MARIADB_SP_STREAMING_DB_CONFIG

#include <unordered_map>

class CConfig {
public:
    static void Update(std::unordered_map<std::string, std::string> &mapConfig);
};

#endif
