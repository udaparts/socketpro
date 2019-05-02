#pragma once

#include "../../include/membuffer.h"

namespace UHTTP {

    class CUJsLoader {

        struct SetLoader {
            SetLoader();
        };
        static SetLoader sl;

    public:
        CUJsLoader(const char *UserAgent, const char *BrowserInfo, const char *host, bool bCrossDomain, bool secure);
        const char *GetSPACode(unsigned int &len) const;

    private:
        CUJsLoader(const CUJsLoader &loader);
        CUJsLoader& operator=(const CUJsLoader &loader);
        void ProcessWBInfo(const char *BrowserInfo);

        static void LoadJavaScripts();
        static void LoadFile(const char *jsFileName, SPA::CUQueue &qBuffer);

    private:
        bool m_bCrossDomain;
        bool m_bJson;
        bool m_bWs;
        bool m_bSwfobject;
        double m_dFlashVersion;
        SPA::CScopeUQueue m_jsCode;
        static SPA::CUQueue m_qAjax;
        static SPA::CUQueue m_qWs;
        static SPA::CUQueue m_qScript;
        static SPA::CUQueue m_qJson;
        static SPA::CUQueue m_qSwfobject;
        static SPA::CUQueue m_qFlashWs;
    };
};

