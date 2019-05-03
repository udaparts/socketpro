
#pragma once

#include "../../include/commutil.h"
#include <string>

#ifdef WIN32_64
#pragma comment(lib, "wbemuuid.lib")
#include <comdef.h>
#ifndef WINCE
#include <wbemidl.h>
#endif
#endif
#include <vector>
#if defined(__ANDROID__) || defined(ANDROID)
#include <jni.h>
#endif

namespace SPA {
    namespace ServerSide {

        class URegistration {
        public:
            URegistration();

        public:
            time_t GetEndDate();

        public:
            unsigned int JavaScript;
            unsigned int RequestQueue;
            unsigned int ManyClients;
            unsigned int MaxSpeed;
            unsigned int Routing;
            unsigned int Endian;
            unsigned int Other;
            std::string CompanyName;
            std::string MachineID;
            std::string Key;
            std::string EndDate;
            std::vector<std::string> Platforms;

        private:
            URegistration(const URegistration &reg);
            URegistration& operator=(const URegistration &reg);
        };
        bool IsRegisterred(const char *secret, URegistration &reg);
        std::string CreateKey(const char *appName, bool manyMachine, const char *secret, SPA::tagOperationSystem os);
    }; //ServerSide

    std::string GetSysId();
    std::string SeekValue(const char *source, const char *start, const char *header, const char *footer);
    std::string ReadFromPipe(const char *cmd);
    std::string GetAppName();
    extern std::string UAppName;
    std::time_t Get2013();
    time_t GetLastWriteTime(const char *filePath);

#if defined(__ANDROID__) || defined(ANDROID)
    void SetAndroidId(JNIEnv *env);
#endif

}; //namespace SPA
