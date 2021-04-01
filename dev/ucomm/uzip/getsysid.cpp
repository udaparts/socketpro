
#include "stdafx.h"
#include "../include/membuffer.h"
#include "../core_shared/pinc/getsysid.h"
#include <algorithm>
#include <boost/algorithm/string/find.hpp>
#include "../include/jsonvalue.h"
#include "../core_shared/pinc/base64.h"
#include "../core_shared/pinc/sha1.h"
#include <fstream>

#if defined(__ANDROID__) || defined(ANDROID)

#elif defined(WINCE)
#include <Iphlpapi.h>
#else
#endif
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#ifdef WIN32_64
#include <psapi.h>
#else
#include <sys/stat.h>
extern const char *__progname;
#endif

#define MAX_REQ_SPEED 5120000
#define MAX_CLIENTS 1024000

namespace SPA {
    std::string UAppName;
#if defined(__ANDROID__) || defined(ANDROID)
    std::string g_strAndroidId;

    void SetAndroidId(JNIEnv *env) {
        jclass cls = env->FindClass("android/provider/Settings$Secure");
        jfieldID android_id = env->GetStaticFieldID(cls, "ANDROID_ID", "Ljava/lang/String;");
        jstring objAndroidId = (jstring) env->GetStaticObjectField(cls, android_id);
        const char *s = env->GetStringUTFChars(objAndroidId, nullptr);
        g_strAndroidId = s;
        env->ReleaseStringUTFChars(objAndroidId, s);
    }
#endif
    std::string GetManagedAppCmd(const std::string &appName);
    std::string GetHashId(const std::string &id, SPA::tagOperationSystem os);
    std::string GetJavaCmd(const std::string &all);
    std::string GetPythonCmd(const std::string &all);

    namespace ServerSide {
        bool SetRegistration(const char *str, URegistration &reg);

        const std::string EndDate("EndDate");
        const std::string CompanyName("CompanyName");
        const std::string DefaultCompanyNameValue("UDAParts");
        const std::string JavaScript("JavaScript");
        const std::string Platforms("Platforms");
        const std::string RequestQueue("Queue");
        const std::string ManyClients("MaxClients");
        const std::string MaxSpeed("MaxSpeed");
        const std::string Routing("Routing");
        const std::string Endian("Endian");
        const std::string Other("Other");
        const std::string MachineID("MachineId");
        const std::string Key("Key");
        const std::string Services("Services");

        URegistration::URegistration() :
        JavaScript(1),
        RequestQueue(1),
        ManyClients(MAX_CLIENTS),
        MaxSpeed(MAX_REQ_SPEED),
        Routing(1),
        Endian(1),
        Other(0) {
            //crash for segmentation failed error on linux with initialization style 
            //we are forced to comment the below call out
#ifdef WIN32_64
            CompanyName = DefaultCompanyNameValue;
#endif
        }

        time_t URegistration::GetEndDate() {
            const char *end;
            struct tm tmDate;
            ::memset(&tmDate, 0, sizeof (tm));
            if (EndDate.size() != 10)
                return std::mktime(&tmDate);
            int year = atoi(EndDate.c_str(), end);
            int month = atoi(EndDate.c_str() + 5, end);
            int day = atoi(EndDate.c_str() + 8, end);
            tmDate.tm_year = year - 1900;
            tmDate.tm_mon = month - 1;
            tmDate.tm_mday = day;
            return std::mktime(&tmDate);
        }

        std::string MakeString(const char *strAppName, unsigned char manyMachine, const char *secret, const URegistration &reg) {
            std::string s;
            if (manyMachine) {
                s = boost::str(boost::format("%1%%2%%3%%4%%5%%6%%7%%8%%9%") % reg.CompanyName
                        % reg.EndDate
                        % reg.Endian
                        % reg.JavaScript
                        % reg.ManyClients
                        % reg.MaxSpeed
                        % reg.Other
                        % reg.Routing
                        % reg.RequestQueue);
            } else {
                s = boost::str(boost::format("%1%%2%%3%%4%%5%%6%%7%%8%%9%%10%") % reg.CompanyName
                        % reg.EndDate
                        % reg.Endian
                        % reg.JavaScript
                        % reg.ManyClients
                        % reg.MaxSpeed
                        % reg.Other
                        % reg.Routing
                        % reg.RequestQueue
                        % reg.MachineID);
            }
            std::vector<std::string>::const_iterator it, end = reg.Platforms.end();
            for (it = reg.Platforms.begin(); it != end; ++it) {
                s += *it;
            }
            s += secret;
            if (manyMachine <= 1) {
                s += strAppName;
            }
            for (auto sit = reg.Services.cbegin(), send = reg.Services.cend(); sit != send; ++sit) {
                s += std::to_string(*sit);
            }
            return s;
        }

        std::string CreateKey(const char *appName, unsigned char manyMachine, const char *secret, SPA::tagOperationSystem os) {
            size_t pos;
            std::string name(appName);
#ifdef WIN32_64
            pos = name.find_last_of('\\');
            if (pos == std::string::npos) {
                pos = name.find_last_of('/');
            }
#else
            pos = name.find_last_of('/');
#endif
            if (pos != std::string::npos)
                name = name.substr(pos + 1);
            switch (os) {
                case SPA::tagOperationSystem::osWin:
                case SPA::tagOperationSystem::osWinCE:
                    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                    break;
                default:
                    break;
            }
            name += std::string("_sp.json");
            std::ifstream in(name.c_str()); // input
            if (!in)
                return "0";
            std::stringstream buffer;
            buffer << in.rdbuf();
            std::string RegString(buffer.str());
            URegistration reg;
            if (!SetRegistration(&(RegString.at(0)), reg))
                return "1";
            return GetHashId(MakeString(name.c_str(), manyMachine, secret, reg), os);
        }

#if defined(__ANDROID__) || defined(ANDROID) || defined(WINCE)

        bool SetRegistration(char *str, URegistration &reg) {
            return false;
        }

        /*
        bool CreateEmptyRegistrationFile() {
            return false;
        }
         */

        bool IsRegisterred(const char *secret, URegistration &reg) {
            return false;
        }
#else

        bool CreateEmptyRegistrationFile() {
            JSON::JArray<char> aOs;
            tagOperationSystem os = GetOS();
            switch (os) {
                case tagOperationSystem::osWin:
                    aOs = {"win", "apple"};
                    break;
                case tagOperationSystem::osApple:
                    aOs.push_back("apple");
                    break;
                case tagOperationSystem::osUnix:
                    aOs = {"unix", "apple"};
                    break;
                    break;
                default:
                    break;
            }
            aOs.push_back("android");
            aOs.push_back("wince");
            JSON::JArray<char> aServices;
            time_t t;
            char strDate[128] = {0};
            time(&t);
#if defined(__ANDROID__) || defined(ANDROID) || defined(WINCE)
            tm *p = ::gmtime(&t);
            ::sprintf(strDate, "%.4d/%.2d/%.2d", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday);
#elif defined(WIN32_64)
            struct tm newtime;
            ::gmtime_s(&newtime, &t);
            tm *p = &newtime;
            ::sprintf_s(strDate, sizeof (strDate), "%.4d/%.2d/%.2d", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday);
#else
            tm *p = ::gmtime(&t);
            ::sprintf(strDate, "%.4d/%.2d/%.2d", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday);
#endif
            std::string id = GetSysId();
            SPA::JSON::JObject<char> jobj;
            jobj[EndDate] = strDate;
            jobj[CompanyName] = DefaultCompanyNameValue;
            jobj[JavaScript] = 1;
            jobj[CompanyName] = DefaultCompanyNameValue;
            jobj[Platforms] = std::move(aOs);
            jobj[Services] = std::move(aServices);
            jobj[RequestQueue] = 1;
            jobj[ManyClients] = MAX_CLIENTS;
            jobj[MaxSpeed] = MAX_REQ_SPEED;
            jobj[Routing] = 1;
            jobj[Endian] = 1;
            jobj[Other] = 0;
            jobj[MachineID] = std::move(id);
            jobj[Key] = "";
            JSON::JValue<char> jv(std::move(jobj));

            std::string appName = GetAppName();
            std::string RegFileName = appName + "_sp.json";
            std::ofstream outfile(RegFileName.c_str());
            if (outfile) {
                outfile << jv.Stringify();
                outfile.close();
                return true;
            }
            return false;
        }

        bool SetRegistration(const char *str, URegistration &reg) {
            std::shared_ptr<JSON::JValue<char>> root(JSON::Parse<char>(str));
            if (!root)
                return false;
            JSON::JValue<char> *jv = root->Child(EndDate);
            if (jv && jv->GetType() == JSON::enumType::String) {
                reg.EndDate = jv->AsString();
            }

            jv = root->Child(CompanyName);
            if (jv && jv->GetType() == JSON::enumType::String) {
                reg.CompanyName = jv->AsString();
            }

            jv = root->Child(MachineID);
            if (jv && jv->GetType() == JSON::enumType::String) {
                reg.MachineID = jv->AsString();
            }

            jv = root->Child(Key);
            if (jv && jv->GetType() == JSON::enumType::String) {
                reg.Key = jv->AsString();
            }

            jv = root->Child(Platforms);
            if (jv && jv->GetType() == JSON::enumType::Array) {
                auto &arr = jv->AsArray();
                for (auto it = arr.begin(), end = arr.end(); it != end; ++it) {
                    if (it->GetType() != JSON::enumType::String) {
                        continue;
                    }
                    reg.Platforms.push_back(it->AsString());
                }
            }

            jv = root->Child(Services);
            if (jv && jv->GetType() == JSON::enumType::Array) {
                auto &arr = jv->AsArray();
                for (auto it = arr.begin(), end = arr.end(); it != end; ++it) {
                    if (it->GetType() != JSON::enumType::Uint64) {
                        continue;
                    }
                    reg.Services.push_back((unsigned int) it->AsUint64());
                }
            }

            jv = root->Child(JavaScript);
            if (jv && jv->GetType() == JSON::enumType::Uint64) {
                reg.JavaScript = (unsigned int) jv->AsUint64();
            } else {
                reg.JavaScript = 1;
            }

            jv = root->Child(RequestQueue);
            if (jv && jv->GetType() == JSON::enumType::Uint64) {
                reg.RequestQueue = (unsigned int) jv->AsUint64();
            } else {
                reg.RequestQueue = 1;
            }

            jv = root->Child(ManyClients);
            if (jv && jv->GetType() == JSON::enumType::Uint64) {
                reg.ManyClients = (unsigned int) jv->AsUint64();
            } else {
                reg.ManyClients = MAX_CLIENTS;
            }

            jv = root->Child(MaxSpeed);
            if (jv && jv->GetType() == JSON::enumType::Uint64) {
                reg.MaxSpeed = (unsigned int) jv->AsUint64();
            } else {
                reg.MaxSpeed = MAX_REQ_SPEED;
            }

            jv = root->Child(Routing);
            if (jv && jv->GetType() == JSON::enumType::Uint64) {
                reg.Routing = (unsigned int) jv->AsUint64();
            } else {
                reg.Routing = 1;
            }

            jv = root->Child(Endian);
            if (jv && jv->GetType() == JSON::enumType::Uint64) {
                reg.Endian = (unsigned int) jv->AsUint64();
            } else {
                reg.Endian = 1;
            }

            jv = root->Child(Other);
            if (jv && jv->GetType() == JSON::enumType::Uint64) {
                reg.Other = (unsigned int) jv->AsUint64();
            } else {
                reg.Other = 0;
            }
            return true;
        }

        bool IsRegisterred(const char *secret, URegistration &reg) {
            reg.Platforms.clear();
            reg.Services.clear();
            std::string appName = GetAppName();
            std::string RegFileName = appName + "_sp.json";
            std::ifstream RegFile(RegFileName.c_str());
            if (!RegFile.is_open()) {
                CreateEmptyRegistrationFile();
                RegFile.open(RegFileName.c_str());
            }
            std::stringstream buffer;
            buffer << RegFile.rdbuf();
            std::string RegString(buffer.str());
            RegFile.close();
            CScopeUQueue su;
            su->Push((const unsigned char*) RegString.c_str(), (unsigned int) RegString.size());
            su->SetNull();
            if (!SetRegistration((char *) su->GetBuffer(), reg))
                return false;
            std::time_t t;
            ::time(&t); //now
            time_t tEnd = reg.GetEndDate();
            if (tEnd + 24 * 3600 < t) //two days allowance
                return false;
            t = GetLastWriteTime(RegFileName.c_str());
            if (t > tEnd + 12 * 3600)
                return false;
            if (GetHashId(MakeString(RegFileName.c_str(), (unsigned char) 2, secret, reg), SPA::GetOS()) == reg.Key)
                return true;
            if (GetHashId(MakeString(RegFileName.c_str(), (unsigned char) 1, secret, reg), SPA::GetOS()) == reg.Key)
                return true;
            std::string sysId = GetSysId();
            if (sysId != reg.MachineID)
                return false;
            return (GetHashId(MakeString(RegFileName.c_str(), (unsigned char) 0, secret, reg), SPA::GetOS()) == reg.Key);
        }
#endif
    } //namespace ServerSide

    std::string GetHashId(const std::string &id, SPA::tagOperationSystem os) {
        char str[64] = {0};
        unsigned char bytes[32] = {0};
        unsigned int Salt = 0x4567FB21;
        SPA::CScopeUQueue su;
        su->Push((const unsigned char*) id.c_str(), (unsigned int) id.length());
        unsigned char byteOs = (unsigned char) os;
        su->Push(&byteOs, sizeof (byteOs));
        su << Salt;
        SPA::CSHA1 sha1;
        sha1.Update(su->GetBuffer(), su->GetSize());
        sha1.Final();
        bool ok = sha1.GetHash(bytes);
        unsigned int res = SPA::CBase64::encode(bytes, 20, str);
        return str;
    }
#ifdef WINCE

    std::string ReadFromPipe(const char *cmd) {
        // ????

        return "";
    }
#elif defined(WIN32_64) 

    std::string PrintProcessNameAndID(DWORD processID) {
        char szProcessName[MAX_PATH] = {0};
        HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

        // Get the process name.
        if (nullptr != hProcess) {
            HMODULE hMod;
            DWORD cbNeeded;
            if (::EnumProcessModules(hProcess, &hMod, sizeof (hMod), &cbNeeded)) {
                ::GetModuleBaseNameA(hProcess, hMod, szProcessName, sizeof (szProcessName) / sizeof (char));
            }
            CloseHandle(hProcess);
        }
        return szProcessName;
    }

    std::string ReadFromPipe(const char *cmd) {
        const unsigned int BUFFER_SIZE = 2048;
        char Buffer[BUFFER_SIZE + 1] = {0};
        std::string s;
        DWORD aProcesses[1024], cbNeeded, cProcesses;
        if (!EnumProcesses(aProcesses, sizeof (aProcesses), &cbNeeded)) {
            return s;
        }
        cProcesses = cbNeeded / sizeof (DWORD);
        for (DWORD i = 0; i < cProcesses; ++i) {
            if (i)
                s += ";";
            if (aProcesses[i] != 0) {
                s += PrintProcessNameAndID(aProcesses[i]);
            }
        }
        return s;
    }
#else

    std::string ReadFromPipe(const char *cmd) {
        const unsigned int BUFFER_SIZE = 2048;
        char Buffer[BUFFER_SIZE + 1] = {0};
        std::string s;
        FILE *pipe = ::popen(cmd, "r");
        if (pipe == nullptr)
            return s;
        while (::fgets(Buffer, BUFFER_SIZE, pipe)) {
            s += Buffer;
            ::memset(Buffer, 0, sizeof (Buffer));
        }
        ::pclose(pipe);
        return s;
    }
#endif

    bool lower_test(char l, char r) {
#ifdef WIN32_64
        return (::tolower(l) == ::tolower(r));
#else
        return (l == r);
#endif
    }

    std::string SeekValue(const char *source, const char *start, const char *header, const char *footer) {
        size_t len = ::strlen(header);
        std::string s;
        const char *str = ::strstr(source, start);
        if (!str)
            return s;
        str = ::strstr(str, header);
        if (!str)
            return s;
        str += len;
        footer = ::strstr(str, footer);
        if (!footer)
            return s;
        --footer;
        s.assign(str, footer);
        return s;
    }

    std::time_t Get2013() {
        std::tm tm;
        memset(&tm, 0, sizeof (tm));
        tm.tm_year = 113;
        tm.tm_yday = 0;
        tm.tm_mday = 1;
        return std::mktime(&tm);
    }

#ifndef WIN32_64

    std::string ExtractSecondParameter(const char *appName) {

        pid_t pid = getpid();
        std::string cmd = "ps -fp ";
        cmd += std::to_string(pid);
        /*
            //ps -C mono -o command=
            std::string cmd = "ps -C ";
            cmd += appName;
            cmd += " -o command=";
         */
        std::string exeName = appName;
        exeName += " ";
        std::string s = ReadFromPipe(cmd.c_str());
        size_t head = s.find(exeName);
        std::string str = s.substr(head + exeName.size());
        boost::trim_if(str, boost::is_any_of(" ./\r\n\t"));
        size_t end = str.find_first_of(" \r\n\t");
        s = str.substr(0, end);
        //std::replace(s.begin(), s.end(), '.', '_');
        return s;
    }
#endif

#ifdef WINCE

    std::string GetSysId() {
        DWORD ret;
        DWORD size;
        char str[64] = {0};
        SPA::CScopeUQueue su;
        PMIB_IFTABLE pi = (PMIB_IFTABLE) su->GetBuffer();
        size = su->GetMaxSize();
        ret = GetIfTable(pi, &size, 0);
        if (ret == ERROR_INSUFFICIENT_BUFFER) {
            su->ReallocBuffer(size + 256);
            size = su->GetMaxSize();
            pi = (MIB_IFTABLE *) su->GetBuffer();
            ret = GetIfTable(pi, &size, 0);
        }
        if (ret == NO_ERROR) {
            DWORD i, count = pi->dwNumEntries;
            for (i = 0; i < count; ++i) {
                PMIB_IFROW mi = pi->table + i;
                if (mi->dwType == IF_TYPE_ETHERNET_CSMACD || mi->dwType == IF_TYPE_IEEE80211) {
                    ::sprintf(str, "%02x%02x%02x%02x%02x%02x", mi->bPhysAddr[0],
                            mi->bPhysAddr[1],
                            mi->bPhysAddr[2],
                            mi->bPhysAddr[3],
                            mi->bPhysAddr[4],
                            mi->bPhysAddr[5]);
                    break;
                }

            }
        }
        return str;
    }

    std::string GetAppName() {
        if (UAppName.size()) {
            return UAppName;
        }
        std::wstring name;
        std::wstring wAppName;
        wchar_t strBuffer[4096 + 1] = {0};
        ::GetModuleFileNameW(nullptr, strBuffer, sizeof (strBuffer) / sizeof (wchar_t));
        name = strBuffer;
        std::wstring::size_type pos = name.rfind('\\');
        if (pos != std::wstring::npos)
            wAppName = name.substr(pos + 1);
        else
            wAppName = name;
        std::transform(wAppName.begin(), wAppName.end(), wAppName.begin(), ::tolower);
        pos = wAppName.rfind('.');
        if (pos != std::wstring::npos)
            name = wAppName.substr(0, pos);
        else
            name = wAppName;
        SPA::CScopeUQueue su;
        SPA::Utilities::ToUTF8(name.c_str(), name.size(), *su);
        UAppName = (const char*) su->GetBuffer();
        return UAppName;
    }

#elif defined(WIN32_64)   

    std::string GetAppName() {
        const char *cmd;
        if (UAppName.size()) {
            return UAppName;
        }
        unsigned int count = 0;
        std::string AppName;
        std::string s;
        int n;
        cmd = "tasklist";
        char strBuffer[4096 + 1] = {0};
        ::GetModuleFileNameA(nullptr, strBuffer, sizeof (strBuffer));
        int len = (int) ::strlen(strBuffer);
        for (n = len - 1; n >= 0; --n) {
            char c = strBuffer[n];
            if (c == '\\' || c == ':') {
                break;
            } else {
                AppName.insert(AppName.begin(), c);
            }
        }
        std::transform(AppName.begin(), AppName.end(), AppName.begin(), ::tolower);
        n = (int) AppName.rfind('.');
        if (n > 0)
            AppName = AppName.substr(0, n);
        if (AppName == "java") {
            std::string javaCmd = GetManagedAppCmd(AppName);
            std::string jcmd = GetJavaCmd(javaCmd);
            if (jcmd.size() != 0) {
                AppName = jcmd;
                std::transform(AppName.begin(), AppName.end(), AppName.begin(), ::tolower);
                return AppName;
            }
        } else if (AppName.find("python") == 0) {
            std::string pythonCmd = GetManagedAppCmd(AppName);
            std::string pcmd = GetPythonCmd(pythonCmd);
            if (pcmd.size() != 0) {
                AppName = pcmd;
                std::transform(AppName.begin(), AppName.end(), AppName.begin(), ::tolower);
                return AppName;
            }
        }
        s = ReadFromPipe(cmd);
        std::string::iterator fpos = std::search(s.begin(), s.end(), AppName.begin(), AppName.end(), lower_test);
        while (fpos != s.end()) {
            ++count;
            fpos += AppName.length();
            fpos = std::search(fpos, s.end(), AppName.begin(), AppName.end(), lower_test);
        }

        if (count > 1) {
            --count;
            char str[64];
            ::sprintf_s(str, sizeof (str), "%d", count);
            AppName += str;
        }
        UAppName = AppName;
        return AppName;
    }

    std::string GetSysId() {
        HRESULT hr;
        ULONG res = 0;
        USES_CONVERSION;
        std::string str;
        bool b = CoInitializeEx(0, COINIT_MULTITHREADED) == S_OK;

        do {
            hr = CoInitializeSecurity(nullptr,
                    -1, // COM authentication
                    nullptr, // Authentication services
                    nullptr, // Reserved
                    RPC_C_AUTHN_LEVEL_DEFAULT, // Default authentication 
                    RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
                    nullptr, // Authentication info
                    EOAC_NONE, // Additional capabilities 
                    nullptr // Reserved
                    );
            CComPtr<IWbemLocator> pIWbemLocator;
            hr = pIWbemLocator.CoCreateInstance(CLSID_WbemLocator);
            if (FAILED(hr))
                break;

            CComPtr<IWbemServices> pIWbemServices;
            hr = pIWbemLocator->ConnectServer(CComBSTR(L"ROOT\\CIMV2"), // Object path of WMI namespace
                    nullptr, // User name. nullptr = current user
                    nullptr, // User password. nullptr = current
                    nullptr, // Locale. nullptr indicates current
                    0, // Security flags.
                    nullptr, // Authority (e.g. Kerberos)
                    nullptr, // Context object 
                    &pIWbemServices // pointer to IWbemServices proxy
                    );
            if (FAILED(hr))
                break;
            hr = CoSetProxyBlanket(pIWbemServices, // Indicates the proxy to set
                    RPC_C_AUTHN_WINNT, // RPC_C_AUTHN_xxx
                    RPC_C_AUTHZ_NONE, // RPC_C_AUTHZ_xxx
                    nullptr, // Server principal name 
                    RPC_C_AUTHN_LEVEL_CALL, // RPC_C_AUTHN_LEVEL_xxx 
                    RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
                    nullptr, // client identity
                    EOAC_NONE // proxy capabilities 
                    );
            if (FAILED(hr))
                break;

            CComPtr<IEnumWbemClassObject> pIEnumWbemClassObject;
            hr = pIWbemServices->ExecQuery(CComBSTR("WQL"),
                    CComBSTR("SELECT * FROM Win32_BIOS"),
                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                    nullptr,
                    &pIEnumWbemClassObject);
            if (!FAILED(hr)) {
                CComPtr<IWbemClassObject> pIWbemClassObject;
                hr = pIEnumWbemClassObject->Next(WBEM_INFINITE, 1, &pIWbemClassObject, &res);
                if (!FAILED(hr) && pIWbemClassObject != nullptr) {
                    CComVariant vt;
                    hr = pIWbemClassObject->Get(L"SerialNumber", 0, &vt, nullptr, nullptr);
                    if (!FAILED(hr) && vt.vt == VT_BSTR)
                        str += W2A(vt.bstrVal);
                }
            }
            if (str.length() != 0)
                break;
            pIEnumWbemClassObject.Release();
            hr = pIWbemServices->ExecQuery(CComBSTR("WQL"),
                    CComBSTR("SELECT * FROM Win32_BaseBoard"),
                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                    nullptr,
                    &pIEnumWbemClassObject);
            if (!FAILED(hr)) {
                CComPtr<IWbemClassObject> pIWbemClassObject;
                hr = pIEnumWbemClassObject->Next(WBEM_INFINITE, 1, &pIWbemClassObject, &res);
                if (!FAILED(hr) && pIWbemClassObject != nullptr) {
                    CComVariant vt;
                    hr = pIWbemClassObject->Get(L"SerialNumber", 0, &vt, nullptr, nullptr);
                    if (!FAILED(hr) && vt.vt == VT_BSTR)
                        str += W2A(vt.bstrVal);
                }
            }
            if (str.length() != 0)
                break;
            pIEnumWbemClassObject.Release();
            hr = pIWbemServices->ExecQuery(CComBSTR("WQL"),
                    CComBSTR("SELECT * FROM Win32_ComputerSystemProduct"),
                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                    nullptr,
                    &pIEnumWbemClassObject);
            if (!FAILED(hr)) {
                CComPtr<IWbemClassObject> pIWbemClassObject;
                hr = pIEnumWbemClassObject->Next(WBEM_INFINITE, 1, &pIWbemClassObject, &res);
                if (!FAILED(hr) && pIWbemClassObject != nullptr) {
                    CComVariant vt;
                    hr = pIWbemClassObject->Get(L"UUID", 0, &vt, nullptr, nullptr);
                    if (!FAILED(hr) && vt.vt == VT_BSTR)
                        str += W2A(vt.bstrVal);
                }
            }
        } while (false);

        if (b)
            CoUninitialize();
        return GetHashId(str, SPA::GetOS());
    }

    time_t GetLastWriteTime(const char *filePath) {
        time_t tt(-1);
        HANDLE hHandle = ::CreateFileA(filePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hHandle == INVALID_HANDLE_VALUE)
            return tt;
        FILETIME ft;
        BOOL ok = ::GetFileTime(hHandle, nullptr, nullptr, &ft);
        ::CloseHandle(hHandle);
        if (ok) {
            ULARGE_INTEGER ull;
            ull.LowPart = ft.dwLowDateTime;
            ull.HighPart = ft.dwHighDateTime;
            return static_cast<time_t> (ull.QuadPart / 10000000ULL - 11644473600ULL);
        }
        return tt;
    }

#else

    std::string GetAppName() {
        const char *cmd;
        if (UAppName.size()) {
            return UAppName;
        }
        unsigned int count = 0;
        std::string AppName;
        std::string s;
        AppName = __progname; //linux
        cmd = "ps -e";
        if (AppName == "java") {
            std::string javaCmd = GetManagedAppCmd(AppName);
            std::string jcmd = GetJavaCmd(javaCmd);
            if (jcmd.size() != 0) {
                AppName = jcmd;
                return AppName;
            }
        } else if (AppName.find("python") == 0) {
            std::string pythonCmd = GetManagedAppCmd(AppName);
            std::string pcmd = GetPythonCmd(pythonCmd);
            if (pcmd.size() != 0) {
                AppName = pcmd;
                return AppName;
            }
        } else if (AppName == "mono" || AppName == "cli") {
            AppName = ExtractSecondParameter(AppName.c_str());
            cmd = "ps aux";
        }
        s = ReadFromPipe(cmd);
        std::string::iterator fpos = std::search(s.begin(), s.end(), AppName.begin(), AppName.end(), lower_test);
        while (fpos != s.end()) {
            ++count;
            fpos += AppName.length();
            fpos = std::search(fpos, s.end(), AppName.begin(), AppName.end(), lower_test);
        }

        if (count > 1) {
            --count;
            char str[64];
            ::sprintf(str, "%d", count);
            AppName += str;
        }
        UAppName = AppName;
        return AppName;
    }

    std::string GetSysId() {
        std::string id;
        std::string s = ReadFromPipe("cat /proc/cpuinfo");

        //id = SeekValue(s.c_str(), "bogomips", ": ", "\n");
        id += SeekValue(s.c_str(), "model name", ": ", "\n");
#if defined(__ANDROID__) || defined(ANDROID)
        id += g_strAndroidId;
#else
        //cpu MHz not stable and it keeps on change every startup
        //id += SeekValue(s.c_str(), "cpu MHz", ": ", "\n");

        s = ReadFromPipe("ip link");
        id += SeekValue(s.c_str(), "eth0", "link/ether ", "brd ");
#endif
        //std::cout << id << std::endl;
        return GetHashId(id, SPA::GetOS());
    }

    time_t GetLastWriteTime(const char *filePath) {
        struct stat fileStat;
        if (::stat(filePath, &fileStat) == 0) {
            return fileStat.st_mtime;
        }
        return std::time_t(-1);
    }

#endif


#if defined(__ANDROID__) || defined(ANDROID) || defined(WINCE)

    std::string GetManagedAppCmd(const std::string &appName) {
        return "";
    }

#elif defined(WIN32_64)

    std::string GetManagedAppCmd(const std::string &appName) {
        DWORD pid = ::GetCurrentProcessId();
        HRESULT hr;
        ULONG res = 0;
        USES_CONVERSION;
        std::string str;
        bool b = CoInitializeEx(0, COINIT_MULTITHREADED) == S_OK;
        do {
            hr = CoInitializeSecurity(nullptr,
                    -1, // COM authentication
                    nullptr, // Authentication services
                    nullptr, // Reserved
                    RPC_C_AUTHN_LEVEL_DEFAULT, // Default authentication 
                    RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
                    nullptr, // Authentication info
                    EOAC_NONE, // Additional capabilities 
                    nullptr // Reserved
                    );
            CComPtr<IWbemLocator> pIWbemLocator;
            hr = pIWbemLocator.CoCreateInstance(CLSID_WbemLocator);
            if (FAILED(hr))
                break;
            CComPtr<IWbemServices> pIWbemServices;
            hr = pIWbemLocator->ConnectServer(CComBSTR(L"ROOT\\CIMV2"), // Object path of WMI namespace
                    nullptr, // User name. nullptr = current user
                    nullptr, // User password. nullptr = current
                    nullptr, // Locale. nullptr indicates current
                    0, // Security flags.
                    nullptr, // Authority (e.g. Kerberos)
                    nullptr, // Context object 
                    &pIWbemServices // pointer to IWbemServices proxy
                    );
            if (FAILED(hr))
                break;
            hr = CoSetProxyBlanket(pIWbemServices, // Indicates the proxy to set
                    RPC_C_AUTHN_WINNT, // RPC_C_AUTHN_xxx
                    RPC_C_AUTHZ_NONE, // RPC_C_AUTHZ_xxx
                    nullptr, // Server principal name 
                    RPC_C_AUTHN_LEVEL_CALL, // RPC_C_AUTHN_LEVEL_xxx 
                    RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
                    nullptr, // client identity
                    EOAC_NONE // proxy capabilities 
                    );
            if (FAILED(hr))
                break;
            std::string cmd = "SELECT * FROM Win32_Process where ProcessId = ";
            cmd += std::to_string((SPA::INT64)pid);

            CComPtr<IEnumWbemClassObject> pIEnumWbemClassObject;
            hr = pIWbemServices->ExecQuery(CComBSTR("WQL"),
                    CComBSTR(cmd.c_str()),
                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                    nullptr,
                    &pIEnumWbemClassObject);
            if (!FAILED(hr)) {
                CComPtr<IWbemClassObject> pIWbemClassObject;
                hr = pIEnumWbemClassObject->Next(WBEM_INFINITE, 1, &pIWbemClassObject, &res);
                if (!FAILED(hr) && pIWbemClassObject != nullptr) {
                    CComVariant vt;
                    hr = pIWbemClassObject->Get(L"CommandLine", 0, &vt, nullptr, nullptr);
                    if (!FAILED(hr) && vt.vt == VT_BSTR)
                        str += W2A(vt.bstrVal);
                }
            }
        } while (false);
        if (b)
            CoUninitialize();
        return str;
    }
#else

    std::string GetManagedAppCmd(const std::string &appName) {
        pid_t pid = getpid();
        std::string cmd = "ps -fp ";
        cmd += std::to_string(pid);
        std::string exeName = " " + appName + " "; //" java ";
        std::string s = ReadFromPipe(cmd.c_str());
        size_t head = s.find(exeName);
        return s.substr(head + exeName.size());
    }
#endif

    std::string GetJavaCmd(const std::string &all) {
        size_t move = 12;
        size_t pos = all.find(" -classpath ");
        if (pos == std::string::npos) {
            move = 4;
            pos = all.find_first_of(" -cp ");
        }
        if (pos == std::string::npos)
            return "";
        std::string str = all.substr(pos + move);
        pos = str.find(" -");
        if (pos != std::string::npos)
            str = str.substr(0, pos);
        boost::algorithm::trim(str);
        pos = str.rfind(' ');
        str = str.substr(pos + 1);
        boost::algorithm::trim(str);
        return str;
    }

    std::string GetPythonCmd(const std::string &all) {
        size_t pos = all.rfind(".py");
        std::string str = all.substr(0, pos);
        pos = str.rfind(' ');
        str = str.substr(pos + 1);
#ifdef WIN32_64
        pos = str.rfind('\\');
        if (pos != std::string::npos)
            str = str.substr(pos + 1);
#endif
        pos = str.rfind('/');
        if (pos != std::string::npos)
            str = str.substr(pos + 1);
        return str;
    }

}; //namespace SPA