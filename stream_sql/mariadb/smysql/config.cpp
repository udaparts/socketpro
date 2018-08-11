
#include "config.h"
#include "../../../include/mysql/server_impl/mysqlimpl.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include "streamingserver.h"

#define STREAM_DB_CONFIG_FILE	"sp_streaming_db_config.txt"

void CConfig::Update(std::unordered_map<std::string, std::string> &mapConfig) {
    std::ifstream input(STREAM_DB_CONFIG_FILE);
    if (input.good()) {
        std::string line;
        while (std::getline(input, line)) {
            auto pos = line.find('=');
            if (pos == std::string::npos) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Bad entry (%s) found in file %s", line.c_str(), STREAM_DB_CONFIG_FILE);
                continue;
            }
            std::string key = line.substr(0, pos);
            SPA::ServerSide::CMysqlImpl::Trim(key);
            std::string value = line.substr(pos + 1);
            SPA::ServerSide::CMysqlImpl::Trim(value);
            if (!key.size()) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Empty key found in file %s", STREAM_DB_CONFIG_FILE);
                continue;
            }
            if (!value.size()) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Empty value found in file %s for key (%s)", STREAM_DB_CONFIG_FILE, key.c_str());
                continue;
            }
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            auto it = mapConfig.find(key);
            if (it == mapConfig.end()) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Key (%s) not supported in file %s", key.c_str(), STREAM_DB_CONFIG_FILE);
                continue;
            }
            it->second = value;
        }
    } else {
        CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "File %s not found", STREAM_DB_CONFIG_FILE);
    }
}
