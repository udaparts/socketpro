#pragma once

#include <map>
#include <vector>
#include "../core_shared/pinc/fastujson.h"

namespace UHTTP {

    typedef std::map<int, std::string>::const_iterator CEcPointer;

    enum tagHttpResponseFeedPlan {
        hrfpAll = 0,
        hrfpHeaderOnly,
        hrfpLengthUnknown,
        hrfpChuncked,
        hrfpPartial,
    };

    enum tagHttpResponseStatus {
        hrsInitial = 0,
        hrsHeadersOnly,
        hrsContent,
        hrsDownloadingFile,
        hrsCompleted,
    };

    enum tagHttpRequestType {
        hrtCustomer = 0,
        hrtDownloadAdapter = 1,
        hrtDownloadLoader = 2,
        hrtJsRequest = 3,
        hrtPush = 4,
    };

    enum tagSpRequest {
        srUnknown = 0,
        srRequest = 1,
        srSwitchTo,
        srEnter,
        srExit,
        srSpeak,
        srSendUserMessage,
        srPing,
        srDoBatch,
        srClose,
    };

    enum tagSpError {
        seOk = 0,
        seBadJson,
        seBadNumberOfArgs,
        seBadArgs,
        seWrongArgType,
        seUnexpectedRequest,
        seEmptyRequest,
        seNotSupport,
        seAuthenticationFailed,
    };

    struct U_MODULE_HIDDEN UHttpRequest {
        const char *ReqName;
        const char *Id;
        SPA::INT64 CallIndex;
        double Version;
        const SPA::UJsonValue *Args;
        tagSpRequest SpRequest;
        tagSpError ErrCode;

        inline const char* GetUID() const {
            if (SpRequest != srSwitchTo)
                return nullptr;
            if (Args->Size() > 0) {
                const SPA::UJsonValue &uid = GetArg(0);
                if (uid.IsString())
                    return uid.GetString();
            }
            return nullptr;
        }

        std::wstring GetUserIdW() const {
            const char *userId = GetUID();
            if (userId == nullptr)
                return L"";
            SPA::CScopeUQueue su;
            SPA::Utilities::ToWide(userId, ::strlen(userId), *su);
            return (const wchar_t*)su->GetBuffer();
        }

        unsigned int GetReqCount() const {
            if (SpRequest != srDoBatch)
                return 1;
            return GetArg(1).Size();
        }

        bool IsServerBatching() const {
            if (SpRequest != srDoBatch)
                return false;
            return GetArg(0).GetBool();
        }

        const SPA::UJsonValue & GetChildRequest(unsigned int index) const {
            assert(SpRequest == srDoBatch);
            return GetArg(1)[index];
        }

        inline const char* GetPwd() const {
            if (SpRequest != srSwitchTo)
                return nullptr;
            if (Args->Size() > 1) {
                const SPA::UJsonValue &pwd = GetArg(1);
                if (pwd.IsString())
                    return pwd.GetString();
            }
            return nullptr;
        }

        inline void CleanPwd() {
            char *str = (char*) GetPwd();
            if (!str)
                return;
            size_t len = ::strlen(str);
            ::memset(str, 0, len);
        }

        std::wstring GetPwdW() const {
            const char *pwd = GetPwd();
            if (pwd == nullptr)
                return L"";
            SPA::CScopeUQueue su;
            SPA::Utilities::ToWide(pwd, ::strlen(pwd), *su);
            std::wstring str = (const wchar_t*)su->GetBuffer();
            
            //remove password stored in memory
            unsigned char *p = (unsigned char*)su->GetBuffer();
            ::memset(p, 0, sizeof(wchar_t)*str.size());
            return str;
        }

        inline bool IsBatching() const {
            return (SpRequest == srDoBatch);
        }

        inline unsigned int GetArgCount() const {
            return Args->Size();
        }

        inline const SPA::UJsonValue & GetArg(unsigned int index) const {
            return (*Args)[index];
        }
    };

    struct HttpResponse {

        HttpResponse() {
            ::memset(this, 0, sizeof (HttpResponse));
        }
        tagHttpResponseStatus Status;
        bool GZip;
        bool Chunked;
    };

}