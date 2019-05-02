
#include "stdafx.h"
#include "jsloader.h"
#include <fstream>

namespace UHTTP {

    SPA::CUQueue CUJsLoader::m_qAjax;
    SPA::CUQueue CUJsLoader::m_qWs;
    SPA::CUQueue CUJsLoader::m_qScript;
    SPA::CUQueue CUJsLoader::m_qJson;
    SPA::CUQueue CUJsLoader::m_qSwfobject;
    SPA::CUQueue CUJsLoader::m_qFlashWs;

    CUJsLoader::SetLoader CUJsLoader::sl;

    CUJsLoader::SetLoader::SetLoader() {
        LoadJavaScripts();
    }

    CUJsLoader::CUJsLoader(const char *UserAgent, const char *BrowserInfo, const char *host, bool bCrossDomain, bool secure)
    : m_bCrossDomain(bCrossDomain) {
        ProcessWBInfo(BrowserInfo);
        if (!m_bJson)
            m_jsCode->Push(m_qJson.GetBuffer(), m_qJson.GetSize());
        if (m_bWs && UserAgent) {
            if (::strstr(UserAgent, "Safari")) {
                const char *version = ::strstr(UserAgent, "Version/");
                if (version) {
                    double d = ::atof(version + 8);
                    if (d < 5.9)
                        m_bWs = false;
                }
            } else if (::strstr(UserAgent, "Opera/")) {
                const char *version = ::strstr(UserAgent, "Version/");
                if (version) {
                    double d = ::atof(version + 8);
                    if (d < 12.4)
                        m_bWs = false;
                }
            }
        }
        if (m_bWs) //WebSocket communicator
            m_jsCode->Push(m_qWs.GetBuffer(), m_qWs.GetSize());
        else if (m_dFlashVersion > 10) {
            m_jsCode->Push("window.WEB_SOCKET_SWF_LOCATION='WebSocketMain.swf';");
            if (!m_bSwfobject)
                m_jsCode->Push(m_qSwfobject.GetBuffer(), m_qSwfobject.GetSize());
            m_jsCode->Push(m_qFlashWs.GetBuffer(), m_qFlashWs.GetSize());
            m_jsCode->Push(m_qWs.GetBuffer(), m_qWs.GetSize());
        } else if (m_bCrossDomain) //JavaScript communicator
            m_jsCode->Push(m_qScript.GetBuffer(), m_qScript.GetSize());
        else //AJAX communicator
            m_jsCode->Push(m_qAjax.GetBuffer(), m_qAjax.GetSize());

        m_jsCode->Push("('");
        if (secure) {
            if (m_bWs)
                m_jsCode->Push("wss://");
            else
                m_jsCode->Push("https://");
        } else {
            if (m_bWs || m_dFlashVersion >= 10)
                m_jsCode->Push("ws://");
            else
                m_jsCode->Push("http://");
        }
        m_jsCode->Push(host);
        m_jsCode->Push("/');");

        m_jsCode->Push("var UHTTP=SPA.ClientSide;");
        if (m_bCrossDomain)
            m_jsCode->Push("UHTTP.crossDomain=true;");
        else
            m_jsCode->Push("UHTTP.crossDomain=false;");
        if (m_bWs) //WebSocket communicator
            m_jsCode->Push("UHTTP.transport=0;");
        else if (m_dFlashVersion >= 10)
            m_jsCode->Push("UHTTP.transport=1;");
        else if (m_bCrossDomain) //JavaScript communicator
            m_jsCode->Push("UHTTP.transport=3;");
        else //AJAX communicator
            m_jsCode->Push("UHTTP.transport=2;");
        m_jsCode->Push("SPA.onLoad();");
        m_jsCode->SetNull();
    }

    void CUJsLoader::ProcessWBInfo(const char *BrowserInfo) {
        const char *json = ::strstr(BrowserInfo, "json=");
        if (json)
            m_bJson = ::atoi(json + 5) ? true : false;
        else
            m_bJson = false;
        const char *ws = ::strstr(BrowserInfo, "ws=");
        if (ws)
            m_bWs = ::atoi(ws + 3) ? true : false;
        else
            m_bWs = false;
        const char *swfobject = ::strstr(BrowserInfo, "swfobject=");
        if (swfobject)
            m_bSwfobject = ::atoi(swfobject + 10) ? true : false;
        else
            m_bSwfobject = false;

        const char *fv = ::strstr(BrowserInfo, "fv=");
        if (fv)
            m_dFlashVersion = ::atof(fv + 3);
        else
            m_dFlashVersion = 0.0;
    }

    void CUJsLoader::LoadFile(const char *jsFileName, SPA::CUQueue &qBuffer) {
        unsigned int length;
        qBuffer.SetSize(0);
        std::ifstream file(jsFileName, std::ios::binary);
        if (file.is_open()) {
            file.seekg(0, std::ios::end);
            length = (unsigned int) file.tellg();
            file.seekg(0, std::ios::beg);
            if (qBuffer.GetMaxSize() + sizeof (wchar_t) < length) {
                qBuffer.ReallocBuffer(length + sizeof (wchar_t));
            }
            file.read((char*) qBuffer.GetBuffer(), length);
            qBuffer.SetSize(length);
        }
        qBuffer.SetNull();
    }

    void CUJsLoader::LoadJavaScripts() {
        LoadFile("uwebsocket.js", m_qWs);
        LoadFile("ujson.js", m_qJson);
        LoadFile("ujscript.js", m_qScript);
        LoadFile("uajax.js", m_qAjax);
        LoadFile("swfobject.js", m_qSwfobject);
        LoadFile("web_socket.js", m_qFlashWs);
    }

    const char* CUJsLoader::GetSPACode(unsigned int &len) const {
        len = m_jsCode->GetSize();
        return (const char*) m_jsCode->GetBuffer();
    }
}