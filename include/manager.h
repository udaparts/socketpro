#ifndef _SPA_CLIENTSIDE_POOL_MANAGER_H_
#define _SPA_CLIENTSIDE_POOL_MANAGER_H_

#include "poolconfig.h"

namespace SPA {
    namespace ClientSide {
        struct SpManager;

        class CSpConfig {
        private:
            CSpConfig();

        public:
            static CSpConfig SpConfig;
            void* GetPool(const std::string &poolKey, unsigned int *pSvsId = nullptr);
            CAsyncServiceHandler* SeekHandler(const std::string &poolKey);
            CAsyncServiceHandler* SeekHandlerByQueue(const std::string &poolKey);
            const std::string& GetErrMsg();
            std::string GetConfig();

        private:
            bool Parse(bool midTier, const char *jsonConfig);
            void Normalize();
            void CheckHostErrors();
            void CheckPoolErrors();
            bool ExistHost(const std::string &key);

        private:
            typedef std::map<std::string, CPoolConfig*> CMapPPool;
            SPA::CUCriticalSection m_cs;
            int m_bQP;
            std::string CertStore;
            CMapHost Hosts;
            CPoolConfig::CMapPool Pools;
            std::string m_errMsg;
            CMapPPool m_pp;
            bool m_bMidTier;
            friend struct SpManager;
        };

        struct SpManager {
            static CSpConfig& SetConfig(bool midTier = false, const char *jsonConfig = nullptr);
            static void* GetPool(const std::string& poolkey, unsigned int *pSvsId = nullptr);
            static CAsyncServiceHandler* SeekHandler(const std::string& poolKey);
            static CAsyncServiceHandler* SeekHandlerByQueue(const std::string& poolKey);
        };

    } //namespace ClientSide
} //namespace PA

#endif
