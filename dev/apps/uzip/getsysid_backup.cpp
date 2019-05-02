
#include "stdafx.h"
#include "../../pinc/getsysid.h"
#include <algorithm>
#include <boost/algorithm/string/find.hpp>
#include "../../pinc/prettywriter.h"
#include "../../pinc/base64.h"
#include "../../pinc/sha1.h"
#include "../../include/membuffer.h"
#include <fstream>
#include <sstream>
#include "../../pinc/uvariant2rj.h"
#include <boost/format.hpp>
#include <boost/filesystem/operations.hpp>

#ifndef WIN32_64
extern const char *__progname;
#endif

namespace SPA {
    std::string GetHashId(const std::string &id);
    namespace ServerSide {
        bool SetRegistration(char *str, URegistration &reg);

        const std::string EndDate("EndDate");
        const std::string CompanyName("CompanyName");
        const std::string DefaultCompanyNameValue("UDAParts");
        const std::string JavaScript("JavaScript");
        const std::string Platforms("Platforms");
        const std::string RequestQueue("Queue");
        const std::string ManyClients("MaxClients");
        const std::string Routing("Routing");
        const std::string Endian("Endian");
        const std::string Other("Other");
        const std::string MachineID("MachineId");
        const std::string Key("Key");

        typedef rapidjson::PrettyWriter<CUQueue> UJsonPrettyWriter;

        URegistration::URegistration() :
        JavaScript(0),
        RequestQueue(0),
        ManyClients(7500),
        Routing(0),
        Endian(0),
        Other(0) {
            //crash for segmentation failed error on linux with initialization style 
            //we are forced to comment the below call out
#ifdef WIN32_64
            CompanyName = DefaultCompanyNameValue;
#endif
        }

        time_t URegistration::GetEndDate() {
            struct tm tmDate;
            ::memset(&tmDate, 0, sizeof (tm));
            if (EndDate.size() != 10)
                return mktime(&tmDate);
            int year = atoi(EndDate.c_str());
            int month = atoi(EndDate.c_str() + 5);
            int day = atoi(EndDate.c_str() + 8);
            tmDate.tm_year = year - 1900;
            tmDate.tm_mon = month - 1;
            tmDate.tm_mday = day;
            return mktime(&tmDate);
        }

        std::string MakeString(const char *strAppName, bool manyMachine, const char *secret, const URegistration &reg) {
            std::string s;
            if (manyMachine)
                s = boost::str(boost::format("%1%%2%%3%%4%%5%%6%%7%%8%") % reg.CompanyName
                    % reg.EndDate
                    % reg.Endian
                    % reg.JavaScript
                    % reg.ManyClients
                    % reg.Other
                    % reg.Routing
                    % reg.RequestQueue);
            else
                s = boost::str(boost::format("%1%%2%%3%%4%%5%%6%%7%%8%%9%") % reg.CompanyName
                    % reg.EndDate
                    % reg.Endian
                    % reg.JavaScript
                    % reg.ManyClients
                    % reg.Other
                    % reg.Routing
                    % reg.RequestQueue
                    % reg.MachineID);

            std::vector<std::string>::const_iterator it, end = reg.Platforms.end();
            for (it = reg.Platforms.begin(); it != end; ++it) {
                s += *it;
            }
            s += secret;
            s += strAppName;

            return s;
        }

        std::string CreateKey(const char *appName, bool manyMachine, const char *secret) {
            size_t pos;
            std::string name(appName);
#ifdef WIN32_64
            pos = name.find_last_of('\\');
            if (pos == std::string::npos)
                pos = name.find_last_of('/');
#else
            pos = name.find_last_of('/');
#endif
            if (pos != std::string::npos)
                name = name.substr(pos + 1);

#ifdef WIN32_64
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
#endif
            name += std::string("_sp.json");
            std::ifstream in(name); // input
            if (!in)
                return "0";
            std::stringstream buffer;
            buffer << in.rdbuf();
            std::string RegString(buffer.str());
            URegistration reg;
            if (!SetRegistration(&(RegString.at(0)), reg))
                return "1";
            return GetHashId(MakeString(name.c_str(), manyMachine, secret, reg));
        }

        bool CreateEmptyRegistrationFile() {
            time_t t;
            char strDate[128];
            std::string strOs;
            std::string id = GetSysId();
            SPA::CScopeUQueue su;

            SPA::UJsonDocument docRes;
            SPA::UJsonValue aOs;
            tagOperationSystem os = GetOS();
            switch (os) {
                case osWin:
                    strOs = "win";
                    break;
                case osWinCE:
                    strOs = "wince";
                    break;
                case osApple:
                    strOs = "apple";
                    break;
                case osUnix:
                    strOs = "unix";
                    break;
                case osAndroid:
                    strOs = "android";
                    break;
                default:
                    break;
            }
            time(&t);

#ifdef WIN32_64
            struct tm newtime;
            ::gmtime_s(&newtime, &t);
            tm *p = &newtime;
            ::sprintf_s(strDate, sizeof (strDate), "%.4d/%.2d/%.2d", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday);
#else
            tm *p = ::gmtime(&t);
            ::sprintf(strDate, "%.4d/%.2d/%.2d", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday);
#endif
            aOs.SetArray();
            aOs.PushBack(strOs.c_str(), docRes.GetAllocator());
            docRes.SetObject();
            docRes.AddMember(EndDate.c_str(), strDate, docRes.GetAllocator());
            docRes.AddMember(CompanyName.c_str(), DefaultCompanyNameValue.c_str(), docRes.GetAllocator());
            docRes.AddMember(JavaScript.c_str(), 0, docRes.GetAllocator());
            docRes.AddMember(Platforms.c_str(), aOs, docRes.GetAllocator());
            docRes.AddMember(RequestQueue.c_str(), 0, docRes.GetAllocator());
            docRes.AddMember(ManyClients.c_str(), 7500, docRes.GetAllocator());
            docRes.AddMember(Routing.c_str(), 0, docRes.GetAllocator());
            docRes.AddMember(Endian.c_str(), 0, docRes.GetAllocator());
            docRes.AddMember(Other.c_str(), 0, docRes.GetAllocator());
            docRes.AddMember(MachineID.c_str(), id.c_str(), docRes.GetAllocator());
            docRes.AddMember(Key.c_str(), "", docRes.GetAllocator());

            su->SetSize(0);
            UJsonPrettyWriter writer(*su);
            docRes.Accept(writer);
            su->SetNull();
            std::string appName = GetAppName();
            std::string RegFileName = appName + "_sp.json";
            std::ofstream outfile(RegFileName);
            if (outfile) {
                outfile << (const char*) su->GetBuffer();
                outfile.close();
                return true;
            }
            return false;
        }

        bool SetRegistration(char *str, URegistration &reg) {
            SPA::UJsonDocument doc;
            const SPA::UJsonDocument::ValueType &json = doc.ParseInsitu < 0 > (str);
            if (doc.HasParseError())
                return false;

            if (json[EndDate.c_str()].IsString())
                reg.EndDate = json[EndDate.c_str()].GetString();

            if (json[CompanyName.c_str()].IsString())
                reg.CompanyName = json[CompanyName.c_str()].GetString();

            if (json[MachineID.c_str()].IsString())
                reg.MachineID = json[MachineID.c_str()].GetString();

            if (json[Key.c_str()].IsString())
                reg.Key = json[Key.c_str()].GetString();

            const SPA::UJsonValue &os = json[Platforms.c_str()];
            if (os.IsArray()) {
                SPA::UJsonValue::ConstValueIterator it, end = os.End();
                for (it = os.Begin(); it != end; ++it) {
                    if (!it->IsString())
                        continue;
                    reg.Platforms.push_back(it->GetString());
                }
            }

            if (json[JavaScript.c_str()].IsUint())
                reg.JavaScript = json[JavaScript.c_str()].GetUint() ? 1 : 0;
            else
                reg.JavaScript = 0;

            if (json[RequestQueue.c_str()].IsUint())
                reg.RequestQueue = json[RequestQueue.c_str()].GetUint();
            else
                reg.RequestQueue = 0;

            if (json[ManyClients.c_str()].IsUint())
                reg.ManyClients = json[ManyClients.c_str()].GetUint();
            else
                reg.ManyClients = 0;

            if (json[Routing.c_str()].IsUint())
                reg.Routing = json[Routing.c_str()].GetUint() ? 1 : 0;
            else
                reg.Routing = 0;

            if (json[Endian.c_str()].IsUint())
                reg.Endian = json[Endian.c_str()].GetUint() ? 1 : 0;
            else
                reg.Endian = 0;

            if (json[Other.c_str()].IsUint())
                reg.Other = json[Other.c_str()].GetUint() ? 1 : 0;
            else
                reg.Other = 0;
            return true;
        }

        bool IsRegisterred(const char *secret, URegistration &reg) {
            std::string appName = GetAppName();
            std::string RegFileName = appName + "_sp.json";
            std::ifstream RegFile(RegFileName);
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
            t = boost::filesystem::last_write_time(RegFileName);
            if (t > tEnd + 12 * 3600)
                return false;
            if (GetHashId(MakeString(RegFileName.c_str(), true, secret, reg)) == reg.Key)
                return true;
            std::string sysId = GetSysId();
            if (sysId != reg.MachineID)
                return false;
            return (GetHashId(MakeString(RegFileName.c_str(), false, secret, reg)) == reg.Key);
        }
    } //namespace ServerSide

    std::string GetHashId(const std::string &id) {
        char str[64] = {0};
        unsigned char bytes[32] = {0};
        unsigned int Salt = 0x4567FB21;
        SPA::CScopeUQueue su;
        su->Push((const unsigned char*) id.c_str(), (unsigned int) id.length());
        su << Salt;
        SPA::CSHA1 sha1;
        sha1.Update(su->GetBuffer(), su->GetSize());
        sha1.Final();
        bool ok = sha1.GetHash(bytes);
        unsigned int res = SPA::CBase64::encode(bytes, 20, str);
        return str;
    }

    std::string ReadFromPipe(const char *cmd) {
        const unsigned int BUFFER_SIZE = 2048;
        char Buffer[BUFFER_SIZE + 1] = {0};
        std::string s;
#ifdef WIN32_64  
        FILE *pipe = ::_popen(cmd, "r");
#else
        FILE *pipe = ::popen(cmd, "r");
#endif
        if (pipe == NULL)
            return s;
        while (::fgets(Buffer, BUFFER_SIZE, pipe)) {
            s += Buffer;
            ::memset(Buffer, 0, sizeof (Buffer));
        }
#ifdef WIN32_64  
        ::_pclose(pipe);
#else
        ::pclose(pipe);
#endif
        return s;
    }

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

    std::string GetAppName() {
        const char *cmd;
        unsigned int count = 0;
        std::string AppName;
        std::string s;
#ifdef WIN32_64
        int n;
        cmd = "tasklist";
        char strBuffer[4096 + 1] = {0};
        ::GetModuleFileNameA(NULL, strBuffer, sizeof (strBuffer));
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
#else
        cmd = "ps -e";
        AppName = __progname; //linux
#endif

        s = ReadFromPipe(cmd);
        std::string::iterator fpos = std::search(s.begin(), s.end(), AppName.begin(), AppName.end(), lower_test);
        while (fpos != s.end()) {
            ++count;
            fpos += AppName.length();
            fpos = std::search(fpos, s.end(), AppName.begin(), AppName.end(), lower_test);
        }
        --count;

        if (count) {
            char str[64];
#ifdef WIN32_64 
            ::sprintf_s(str, sizeof (str), "%d", count);
#else
            ::sprintf(str, "%d", count);
#endif
            AppName += str;
        }
        return AppName;
    }

#ifdef WIN32_64   

    std::string GetSysId() {
        HRESULT hr;
        ULONG res = 0;
        USES_CONVERSION;
        std::string str;
        bool b = CoInitializeEx(0, COINIT_MULTITHREADED) == S_OK;

        do {
            hr = CoInitializeSecurity(NULL,
                    -1, // COM authentication
                    NULL, // Authentication services
                    NULL, // Reserved
                    RPC_C_AUTHN_LEVEL_DEFAULT, // Default authentication 
                    RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
                    NULL, // Authentication info
                    EOAC_NONE, // Additional capabilities 
                    NULL // Reserved
                    );
            if (FAILED(hr))
                break;

            CComPtr<IWbemLocator> pIWbemLocator;
            hr = pIWbemLocator.CoCreateInstance(CLSID_WbemLocator);
            if (FAILED(hr))
                break;

            CComPtr<IWbemServices> pIWbemServices;
            hr = pIWbemLocator->ConnectServer(CComBSTR(L"ROOT\\CIMV2"), // Object path of WMI namespace
                    NULL, // User name. NULL = current user
                    NULL, // User password. NULL = current
                    0, // Locale. NULL indicates current
                    NULL, // Security flags.
                    0, // Authority (e.g. Kerberos)
                    0, // Context object 
                    &pIWbemServices // pointer to IWbemServices proxy
                    );
            if (FAILED(hr))
                break;
            hr = CoSetProxyBlanket(pIWbemServices, // Indicates the proxy to set
                    RPC_C_AUTHN_WINNT, // RPC_C_AUTHN_xxx
                    RPC_C_AUTHZ_NONE, // RPC_C_AUTHZ_xxx
                    NULL, // Server principal name 
                    RPC_C_AUTHN_LEVEL_CALL, // RPC_C_AUTHN_LEVEL_xxx 
                    RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
                    NULL, // client identity
                    EOAC_NONE // proxy capabilities 
                    );
            if (FAILED(hr))
                break;

            CComPtr<IEnumWbemClassObject> pIEnumWbemClassObject;
            hr = pIWbemServices->ExecQuery(CComBSTR("WQL"),
                    CComBSTR("SELECT * FROM Win32_BIOS"),
                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                    NULL,
                    &pIEnumWbemClassObject);
            if (!FAILED(hr)) {
                CComPtr<IWbemClassObject> pIWbemClassObject;
                hr = pIEnumWbemClassObject->Next(WBEM_INFINITE, 1, &pIWbemClassObject, &res);
                if (!FAILED(hr) && pIWbemClassObject != NULL) {
                    CComVariant vt;
                    hr = pIWbemClassObject->Get(L"SerialNumber", 0, &vt, 0, 0);
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
                    NULL,
                    &pIEnumWbemClassObject);
            if (!FAILED(hr)) {
                CComPtr<IWbemClassObject> pIWbemClassObject;
                hr = pIEnumWbemClassObject->Next(WBEM_INFINITE, 1, &pIWbemClassObject, &res);
                if (!FAILED(hr) && pIWbemClassObject != NULL) {
                    CComVariant vt;
                    hr = pIWbemClassObject->Get(L"SerialNumber", 0, &vt, 0, 0);
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
                    NULL,
                    &pIEnumWbemClassObject);
            if (!FAILED(hr)) {
                CComPtr<IWbemClassObject> pIWbemClassObject;
                hr = pIEnumWbemClassObject->Next(WBEM_INFINITE, 1, &pIWbemClassObject, &res);
                if (!FAILED(hr) && pIWbemClassObject != NULL) {
                    CComVariant vt;
                    hr = pIWbemClassObject->Get(L"UUID", 0, &vt, 0, 0);
                    if (!FAILED(hr) && vt.vt == VT_BSTR)
                        str += W2A(vt.bstrVal);
                }
            }
        } while (false);

        if (b)
            CoUninitialize();
        return GetHashId(str);
    }

#else

    std::string GetSysId() {
        std::string id;
        std::string s = ReadFromPipe("cat /proc/cpuinfo");
        //		id = SeekValue(s.c_str(), "bogomips", ": ", "\n");
        id += SeekValue(s.c_str(), "model name", ": ", "\n");
        id += SeekValue(s.c_str(), "cpu MHz", ": ", "\n");
        s = ReadFromPipe("ifconfig | grep HWaddr");
        id += SeekValue(s.c_str(), "eth0", "HWaddr ", "\n");
        id += SeekValue(s.c_str(), "eth1", "HWaddr ", "\n");
        return GetHashId(id);
    }

#endif

}; //namespace SPA