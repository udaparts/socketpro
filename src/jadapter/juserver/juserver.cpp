
#include "../../../include/server_functions.h"
#include "SPA_ServerSide_ServerCoreLoader.h"
#include <string>
#include <unordered_map>
#include <algorithm>
#ifndef NDEBUG
#include <iostream>
#endif

#ifdef WINCE
#elif defined(WIN32_64)
#else
#include <unistd.h>
#endif

SPA::CUCriticalSection g_co;
JavaVM *g_vm = nullptr;
jclass g_clsCSocketPeer = nullptr;
jclass g_clsCHttpPeerBase = nullptr;
jclass g_clsCHttpHeaderValue = nullptr;
jclass g_clsServer = nullptr;

jmethodID g_midOnCloseGlobal = nullptr;
jmethodID g_midOnSSLHandShakeCompleted = nullptr;
jmethodID g_midOnAccept = nullptr;
jmethodID g_midOnIdle = nullptr;
jmethodID g_midOnIsPermitted = nullptr;

jmethodID g_midOnClose = nullptr;
jmethodID g_midOnResultsSent = nullptr;
jmethodID g_midOnRequestArrive = nullptr;
jmethodID g_midOnFastRequestArrive = nullptr;
jmethodID g_midSLOW_PROCESS = nullptr;
jmethodID g_midOnRequestProcessed = nullptr;
jmethodID g_midOnBaseRequestCame = nullptr;
jmethodID g_midOnSwitchTo = nullptr;
jmethodID g_midOnChatRequestComing = nullptr;
jmethodID g_midOnChatRequestCame = nullptr;

jmethodID g_midOnHttpAuthentication = nullptr;

jmethodID g_constructorHttpHeaderValue = nullptr;
jfieldID g_fidHeader = nullptr;
jfieldID g_fidValue = nullptr;

void CleanException(JNIEnv *env) {
    jthrowable ex = env->ExceptionOccurred();
    if (ex) {
        env->ExceptionClear();
    }
}

void CALLBACK OnThreadEvent(SPA::ServerSide::tagThreadEvent te);

#ifdef WIN32_64
typedef DWORD UTHREAD_ID;
#else
typedef pthread_t UTHREAD_ID;
#endif

std::vector<UTHREAD_ID> g_vDThread;

void SetCurrentThreadAsDaemon() {
    jint res;
#ifdef WIN32_64
    UTHREAD_ID tid = ::GetCurrentThreadId();
#else
    UTHREAD_ID tid = pthread_self();
#endif
    SPA::CAutoLock al(g_co);
    if (std::find(g_vDThread.begin(), g_vDThread.end(), tid) == g_vDThread.end()) {
        JNIEnv *env = nullptr;
        JavaVMAttachArgs args;
        args.version = JNI_VERSION_1_6; // choose your JNI version
        if (::IsMainThread())
            args.name = (char*) "userver_main_thread"; // you might want to give the java thread a name
        else
            args.name = (char*) "userver_worker_thread"; // you might want to give the java thread a name
        args.group = nullptr; // you might want to assign the java thread to a ThreadGroup
        res = g_vm->AttachCurrentThreadAsDaemon((void **) &env, &args);
        assert(res == JNI_OK && env != nullptr);
        if (res == 0)
            g_vDThread.push_back(tid);
    }
}

void RemoveCurrentThreadAsDaemon() {
#ifdef WIN32_64
    UTHREAD_ID tid = ::GetCurrentThreadId();
#else
    UTHREAD_ID tid = pthread_self();
#endif
    bool removed = true;
    SPA::CAutoLock al(g_co);
    while (removed) {
        removed = false;
        for (auto it = g_vDThread.begin(), end = g_vDThread.end(); it != end; ++it) {
            if (*it == tid) {
                jint res = g_vm->DetachCurrentThread();
                assert(JNI_OK == res);
                removed = true;
                g_vDThread.erase(it);
                break;
            }
        }
    }
}

void CALLBACK OnCloseGlobal(USocket_Server_Handle Handler, int errCode);
void CALLBACK OnSSLHandShakeCompleted(USocket_Server_Handle Handler, int errCode);
void CALLBACK OnAccept(USocket_Server_Handle Handler, int errCode);
void CALLBACK OnIdle(SPA::INT64 milliseconds);
bool CALLBACK OnIsPermitted(USocket_Server_Handle Handler, unsigned int serviceId);

void CALLBACK OnClose(USocket_Server_Handle Handler, int errCode);
void CALLBACK OnResultsSent(USocket_Server_Handle Handler);
void CALLBACK OnRequestArrive(USocket_Server_Handle Handler, unsigned short requestId, unsigned int len);
void CALLBACK OnFastRequestArrive(USocket_Server_Handle Handler, unsigned short requestId, unsigned int len);
int CALLBACK SLOW_PROCESS(unsigned short requestId, unsigned int len, USocket_Server_Handle Handler);
void CALLBACK OnRequestProcessed(USocket_Server_Handle Handler, unsigned short requestId);
void CALLBACK OnBaseRequestCame(USocket_Server_Handle Handler, unsigned short requestId);
void CALLBACK OnSwitchTo(USocket_Server_Handle Handler, unsigned int oldServiceId, unsigned int newServiceId);
void CALLBACK OnChatRequestComing(USocket_Server_Handle handler, SPA::tagChatRequestID chatRequestID, unsigned int len);
void CALLBACK OnChatRequestCame(USocket_Server_Handle handler, SPA::tagChatRequestID chatRequestId);
bool CALLBACK OnHttpAuthentication(USocket_Server_Handle handler, const wchar_t *userId, const wchar_t *password);

void SetCaches(JNIEnv *env) {
    jclass cls = env->FindClass("SPA/ServerSide/CSocketProServer");
    g_clsServer = (jclass) env->NewGlobalRef(cls);
    g_midOnCloseGlobal = env->GetStaticMethodID(cls, "OnCloseInternal", "(JI)V");
    g_midOnSSLHandShakeCompleted = env->GetStaticMethodID(cls, "OnSSLShakeCompletedInternal", "(JI)V");
    g_midOnAccept = env->GetStaticMethodID(cls, "OnAcceptInternal", "(JI)V");
    g_midOnIdle = env->GetStaticMethodID(cls, "OnIdleInternal", "(J)V");
    g_midOnIsPermitted = env->GetStaticMethodID(cls, "IsPermittedInternal", "(JI)Z");

    cls = env->FindClass("SPA/ServerSide/CSocketPeer");
    g_clsCSocketPeer = (jclass) env->NewGlobalRef(cls);
    g_midOnClose = env->GetStaticMethodID(cls, "OnClosed", "(JI)V");
    g_midOnResultsSent = env->GetStaticMethodID(cls, "OnResultsSent", "(J)V");
    g_midOnRequestArrive = env->GetStaticMethodID(cls, "OnRArrive", "(JSI)V");
    g_midOnFastRequestArrive = env->GetStaticMethodID(cls, "OnFast", "(JSI)V");
    g_midSLOW_PROCESS = env->GetStaticMethodID(cls, "OnSlow", "(JSI)I");
    g_midOnRequestProcessed = env->GetStaticMethodID(cls, "OnSlowRequestProcessed", "(JS)V");
    g_midOnBaseRequestCame = env->GetStaticMethodID(cls, "OnBRCame", "(JS)V");
    g_midOnSwitchTo = env->GetStaticMethodID(cls, "OnSwitch", "(JII)V");
    g_midOnChatRequestComing = env->GetStaticMethodID(cls, "OnChatComing", "(JSI)V");
    g_midOnChatRequestCame = env->GetStaticMethodID(cls, "OnCRCame", "(JS)V");

    cls = env->FindClass("SPA/ServerSide/CHttpPeerBase");
    g_clsCHttpPeerBase = (jclass) env->NewGlobalRef(cls);
    g_midOnHttpAuthentication = env->GetStaticMethodID(cls, "DoAuthentication", "(JLjava/lang/String;Ljava/lang/String;)Z");

    cls = env->FindClass("SPA/ServerSide/CHttpHeaderValue");
    g_clsCHttpHeaderValue = (jclass) env->NewGlobalRef(cls);
    g_constructorHttpHeaderValue = env->GetMethodID(cls, "<init>", "()V");
    g_fidHeader = env->GetFieldID(cls, "Header", "Ljava/lang/String;");
    g_fidValue = env->GetFieldID(cls, "Value", "Ljava/lang/String;");
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetPeer(JNIEnv *, jclass, jlong h, jobject peer) {

}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_RemovePeer(JNIEnv *, jclass, jlong h) {

}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_UseUTF16(JNIEnv *env, jclass) {
#ifndef WIN32_64
    UseUTF16();
#endif
    SetCaches(env);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_InitSocketProServer(JNIEnv *env, jclass, jint param, jobject server) {
    if (!g_vm) {
        jint es = env->GetJavaVM(&g_vm);
        assert(JNI_OK == es);
        es = env->EnsureLocalCapacity(32);
        assert(JNI_OK == es);
    }
    bool ok = InitSocketProServer((int) param);
    SetThreadEvent(OnThreadEvent);
    SetOnAccept(OnAccept);
    SetOnClose(OnCloseGlobal);
    SetOnIdle(OnIdle);
    SetOnIsPermitted(OnIsPermitted);
    SetOnSSLHandShakeCompleted(OnSSLHandShakeCompleted);
    return ok;
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_UninitSocketProServer(JNIEnv *env, jclass) {
    UninitSocketProServer();
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_StartSocketProServer(JNIEnv *, jclass, jint port, jint maxBacklog, jboolean v6) {
    return StartSocketProServer((unsigned int) port, (unsigned int) maxBacklog, v6 ? true : false);
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_StopSocketProServer(JNIEnv *env, jclass) {
    StopSocketProServer();
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_IsCanceled(JNIEnv *, jclass, jlong h) {
    return IsCanceled((USocket_Server_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_IsRunning(JNIEnv *, jclass) {
    return IsRunning();
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetAuthenticationMethod(JNIEnv *, jclass, jint am) {
    SetAuthenticationMethod((SPA::ServerSide::tagAuthenticationMethod)am);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetAuthenticationMethod(JNIEnv *, jclass) {
    return (jint) GetAuthenticationMethod();
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetSharedAM(JNIEnv *, jclass, jboolean b) {
    SetSharedAM(b ? true : false);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetSharedAM(JNIEnv *, jclass) {
    return GetSharedAM();
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_PostQuitPump(JNIEnv *, jclass) {
    PostQuitPump();
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_IsMainThread(JNIEnv *, jclass) {
    return IsMainThread();
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_AddSvsContext(JNIEnv *, jclass, jint svsId, jint apartment) {
    CSvsContext sc;
    sc.m_ta = (SPA::tagThreadApartment)apartment;
    sc.m_OnBaseRequestCame = OnBaseRequestCame;
    sc.m_OnChatRequestCame = OnChatRequestCame;
    sc.m_OnChatRequestComing = OnChatRequestComing;
    sc.m_OnClose = OnClose;
    sc.m_OnFastRequestArrive = OnFastRequestArrive;
    sc.m_OnHttpAuthentication = OnHttpAuthentication;
    sc.m_OnRequestArrive = OnRequestArrive;
    sc.m_OnRequestProcessed = OnRequestProcessed;
    sc.m_OnResultsSent = OnResultsSent;
    sc.m_OnSwitchTo = OnSwitchTo;
    sc.m_SlowProcess = SLOW_PROCESS;
    return AddSvsContext((unsigned int) svsId, sc);
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_RemoveASvsContext(JNIEnv *, jclass, jint svsId) {
    RemoveASvsContext((unsigned int) svsId);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_AddSlowRequest(JNIEnv *, jclass, jint svsId, jshort reqId) {
    return AddSlowRequest((unsigned int) svsId, (unsigned int) reqId);
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_RemoveSlowRequest(JNIEnv *, jclass, jint svsId, jshort reqId) {
    RemoveSlowRequest((unsigned int) svsId, (unsigned int) reqId);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetCountOfServices(JNIEnv *, jclass) {
    return (jint) GetCountOfServices();
}

JNIEXPORT jintArray JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetServices(JNIEnv *env, jclass) {
    unsigned int count = GetCountOfServices() + 10;
    std::vector<unsigned int> v(0, count);
    unsigned int res = GetServices(v.data(), (unsigned int) v.size());
    jintArray arr = env->NewIntArray((jsize) res);
    env->SetIntArrayRegion(arr, 0, (jsize) res, (const jint*) v.data());
    return arr;
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetCountOfSlowRequests(JNIEnv *, jclass, jint svsId) {
    return (jint) GetCountOfSlowRequests((unsigned int) svsId);
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_RemoveAllSlowRequests(JNIEnv *, jclass, jint svsId) {
    RemoveAllSlowRequests((unsigned int) svsId);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetAllSlowRequestIds(JNIEnv *env, jclass, jint svsId, jshortArray reqIds, jint len) {
    if (!reqIds || !len)
        return 0;
    jshort *p = env->GetShortArrayElements(reqIds, nullptr);
    unsigned int res = GetAllSlowRequestIds((unsigned int) svsId, (unsigned short*) p, (unsigned int) len);
    env->ReleaseShortArrayElements(reqIds, p, 0);
    return (jint) res;
}

JNIEXPORT jlong JNICALL Java_SPA_ServerSide_ServerCoreLoader_AddADll(JNIEnv *env, jclass, jbyteArray file, jint param) {
    if (!file)
        return 0;
    jsize len = env->GetArrayLength(file);
    jbyte *p = env->GetByteArrayElements(file, nullptr);
    std::string path((const char*) p, (const char*) p + len);
    HINSTANCE h = AddADll(path.c_str(), (int) param);
    env->ReleaseByteArrayElements(file, p, JNI_ABORT);
    return (jlong) h;
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_RemoveADllByHandle(JNIEnv *, jclass, jlong hinstance) {
    return RemoveADllByHandle((HINSTANCE) hinstance);
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetPrivateKeyFile(JNIEnv *env, jclass, jbyteArray keyFile) {
    if (!keyFile) {
        SetPrivateKeyFile("");
        return;
    }
    jsize len = env->GetArrayLength(keyFile);
    jbyte *p = env->GetByteArrayElements(keyFile, nullptr);
    std::string path((const char*) p, (const char*) p + len);
    SetPrivateKeyFile(path.c_str());
    env->ReleaseByteArrayElements(keyFile, p, JNI_ABORT);
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetCertFile(JNIEnv *env, jclass, jbyteArray certFile) {
    if (!certFile) {
        SetCertFile("");
        return;
    }
    jsize len = env->GetArrayLength(certFile);
    jbyte *p = env->GetByteArrayElements(certFile, nullptr);
    std::string path((const char*) p, (const char*) p + len);
    SetCertFile(path.c_str());
    env->ReleaseByteArrayElements(certFile, p, JNI_ABORT);
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetPKFPassword(JNIEnv *env, jclass, jbyteArray pwd) {
    if (!pwd) {
        SetPKFPassword("");
        return;
    }
    jsize len = env->GetArrayLength(pwd);
    jbyte *p = env->GetByteArrayElements(pwd, nullptr);
    std::string str((const char*) p, (const char*) p + len);
    SetPKFPassword(str.c_str());
    env->ReleaseByteArrayElements(pwd, p, JNI_ABORT);
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetDHParmsFile(JNIEnv *env, jclass, jbyteArray file) {
    if (!file) {
        SetDHParmsFile("");
        return;
    }
    jsize len = env->GetArrayLength(file);
    jbyte *p = env->GetByteArrayElements(file, nullptr);
    std::string path((const char*) p, (const char*) p + len);
    SetDHParmsFile(path.c_str());
    env->ReleaseByteArrayElements(file, p, JNI_ABORT);
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetDefaultEncryptionMethod(JNIEnv *, jclass, jint em) {
    SetDefaultEncryptionMethod((SPA::tagEncryptionMethod)em);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetDefaultEncryptionMethod(JNIEnv *, jclass) {
    return (jint) GetDefaultEncryptionMethod();
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetPfxFile(JNIEnv *env, jclass, jbyteArray file) {
    if (!file) {
        SetPfxFile("");
        return;
    }
    jsize len = env->GetArrayLength(file);
    jbyte *p = env->GetByteArrayElements(file, nullptr);
    std::string path((const char*) p, (const char*) p + len);
    SetPfxFile(path.c_str());
    env->ReleaseByteArrayElements(file, p, JNI_ABORT);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetServerErrorCode(JNIEnv *, jclass) {
    return (jint) GetServerErrorCode();
}

JNIEXPORT jstring JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetServerErrorMessage(JNIEnv *env, jclass) {
    char errMsg[4096] = {0};
    unsigned int res = GetServerErrorMessage(errMsg, sizeof (errMsg));
    return env->NewStringUTF(errMsg);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_IsServerRunning(JNIEnv *, jclass) {
    return IsServerRunning();
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_IsServerSSLEnabled(JNIEnv *, jclass) {
    return IsServerSSLEnabled();
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_Close(JNIEnv *, jclass, jlong h) {
    return Close((USocket_Server_Handle) h);
}

JNIEXPORT jshort JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetCurrentRequestID(JNIEnv *, jclass, jlong h) {
    return (jshort) GetCurrentRequestID((USocket_Server_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetCurrentRequestLen(JNIEnv *, jclass, jlong h) {
    return (jint) GetCurrentRequestLen((USocket_Server_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetRcvBytesInQueue(JNIEnv *, jclass, jlong h) {
    return (jint) GetRcvBytesInQueue((USocket_Server_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetSndBytesInQueue(JNIEnv *, jclass, jlong h) {
    return (jint) GetSndBytesInQueue((USocket_Server_Handle) h);
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_PostClose(JNIEnv *, jclass, jlong h, jint errCode) {
    PostClose((USocket_Server_Handle) h, (int) errCode);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_QueryRequestsInQueue(JNIEnv *, jclass, jlong h) {
    return (jint) QueryRequestsInQueue((USocket_Server_Handle) h);
}

JNIEXPORT jbyteArray JNICALL Java_SPA_ServerSide_ServerCoreLoader_RetrieveBuffer(JNIEnv *env, jclass, jlong h, jint len, jboolean peek) {
    if (len < 0)
        len = 0;
    jbyteArray bytes = env->NewByteArray(len);
    if (len) {
        const unsigned char *arr = GetRequestBuffer((USocket_Server_Handle) h);
        env->SetByteArrayRegion(bytes, 0, len, (const jbyte*) arr);
    }
    return bytes;
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_IsOpened(JNIEnv *, jclass, jlong h) {
    return IsOpened((USocket_Server_Handle) h);
}

JNIEXPORT jlong JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetBytesReceived(JNIEnv *, jclass, jlong h) {
    return (jlong) GetBytesReceived((USocket_Server_Handle) h);
}

JNIEXPORT jlong JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetBytesSent(JNIEnv *, jclass, jlong h) {
    return (jlong) GetBytesSent((USocket_Server_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_SendReturnData(JNIEnv *env, jclass, jlong h, jshort reqId, jint len, jbyteArray buffer) {
    jbyte *p = nullptr;
    if (buffer && len)
        p = env->GetByteArrayElements(buffer, nullptr);
    jint res = (jint) SendReturnData((USocket_Server_Handle) h, (unsigned short) reqId, (unsigned int) len, (const unsigned char*) p);
    if (p)
        env->ReleaseByteArrayElements(buffer, p, JNI_ABORT);
    return res;
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_SendReturnDataIndex(JNIEnv *env, jclass, jlong h, jlong index, jshort reqId, jint len, jbyteArray buffer) {
    jbyte *p = nullptr;
    if (buffer && len)
        p = env->GetByteArrayElements(buffer, nullptr);
    jint res = (jint) SendReturnDataIndex((USocket_Server_Handle) h, (SPA::UINT64) index, (unsigned short) reqId, (unsigned int) len, (const unsigned char*) p);
    if (p)
        env->ReleaseByteArrayElements(buffer, p, JNI_ABORT);
    return res;
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetLastCallInfo(JNIEnv *env, jclass, jbyteArray buffer, jint len) {
    jbyte *p = nullptr;
    std::string s;
    if (buffer && len) {
        p = env->GetByteArrayElements(buffer, nullptr);
        s.assign((const char*) p, (size_t) len);
    }
    SetLastCallInfo(s.c_str());
    if (p)
        env->ReleaseByteArrayElements(buffer, p, JNI_ABORT);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetSvsID(JNIEnv *, jclass, jlong h) {
    return (jint) GetSvsID((USocket_Server_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetServerSocketErrorCode(JNIEnv *, jclass, jlong h) {
    return (jint) GetServerSocketErrorCode((USocket_Server_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetServerSocketErrorMessage(JNIEnv *env, jclass, jlong h, jbyteArray msg, jint len) {
    if (!msg && !len)
        return 0;
    jbyte *p = env->GetByteArrayElements(msg, nullptr);
    jint res = GetServerSocketErrorMessage((USocket_Server_Handle) h, (char*) p, (unsigned int) len);
    env->ReleaseByteArrayElements(msg, p, 0);
    return res;
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_IsBatching(JNIEnv *, jclass, jlong h) {
    return IsBatching((USocket_Server_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetBytesBatched(JNIEnv *, jclass, jlong h) {
    return (jint) GetBytesBatched((USocket_Server_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_StartBatching(JNIEnv *, jclass, jlong h) {
    return StartBatching((USocket_Server_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_CommitBatching(JNIEnv *, jclass, jlong h) {
    return CommitBatching((USocket_Server_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_AbortBatching(JNIEnv *, jclass, jlong h) {
    return AbortBatching((USocket_Server_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetUserID(JNIEnv *env, jclass, jlong h, jstring userId) {
    if (!userId)
        return SetUserID((USocket_Server_Handle) h, L"");
    const jchar *uid = env->GetStringChars(userId, nullptr);
    std::vector<jchar> v(uid, uid + env->GetStringLength(userId));
    jchar c = 0;
    v.push_back(c); //null-terminated
    bool ok = SetUserID((USocket_Server_Handle) h, (const wchar_t *) v.data());
    env->ReleaseStringChars(userId, uid);
    return ok;
}

JNIEXPORT jstring JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetUID(JNIEnv *env, jclass, jlong h) {
    jchar str[256] = {0};
    unsigned int res = GetUID((USocket_Server_Handle) h, (wchar_t*) str, 256);
    return env->NewString(str, (jsize) res);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetPassword(JNIEnv *env, jclass, jlong h, jstring pwd) {
    if (!pwd)
        return SetPassword((USocket_Server_Handle) h, L"");
    const jchar *p = env->GetStringChars(pwd, nullptr);
    std::vector<jchar> v(p, p + env->GetStringLength(pwd));
    jchar c = 0;
    v.push_back(c); //null-terminated
    bool ok = SetPassword((USocket_Server_Handle) h, (const wchar_t *) v.data());
    env->ReleaseStringChars(pwd, p);
    return ok;
}

JNIEXPORT jstring JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetPassword(JNIEnv *env, jclass, jlong h) {
    jchar str[256] = {0};
    unsigned int res = GetPassword((USocket_Server_Handle) h, (wchar_t*) str, 256);
    return env->NewString(str, (jsize) res);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_Enter(JNIEnv *env, jclass, jlong h, jintArray groups, jint count) {
    jint *p = nullptr;
    if (count && groups)
        p = env->GetIntArrayElements(groups, nullptr);
    bool ok = Enter((USocket_Server_Handle) h, (const unsigned int*) p, (unsigned int) count);
    if (p)
        env->ReleaseIntArrayElements(groups, p, JNI_ABORT);
    return ok;
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_Exit(JNIEnv *, jclass, jlong h) {
    Exit((USocket_Server_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_Speak(JNIEnv *env, jclass, jlong h, jbyteArray msg, jint size, jintArray groups, jint count) {
    jbyte *b = nullptr;
    if (msg && size)
        b = env->GetByteArrayElements(msg, nullptr);
    jint *p = nullptr;
    if (count && groups)
        p = env->GetIntArrayElements(groups, nullptr);
    bool ok = Speak((USocket_Server_Handle) h, (const unsigned char*) b, (unsigned int) size, (const unsigned int*) p, (unsigned int) count);
    if (p)
        env->ReleaseIntArrayElements(groups, p, JNI_ABORT);
    if (b)
        env->ReleaseByteArrayElements(msg, b, JNI_ABORT);
    return ok;
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_SpeakPush(JNIEnv *env, jclass, jbyteArray msg, jint size, jintArray groups, jint count) {
    jbyte *b = nullptr;
    if (msg && size)
        b = env->GetByteArrayElements(msg, nullptr);
    jint *p = nullptr;
    if (count && groups)
        p = env->GetIntArrayElements(groups, nullptr);
    bool ok = SpeakPush((const unsigned char*) b, (unsigned int) size, (const unsigned int*) p, (unsigned int) count);
    if (p)
        env->ReleaseIntArrayElements(groups, p, JNI_ABORT);
    if (b)
        env->ReleaseByteArrayElements(msg, b, JNI_ABORT);
    return ok;
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_SpeakEx(JNIEnv *env, jclass, jlong h, jbyteArray msg, jint size, jintArray groups, jint count) {
    jbyte *b = nullptr;
    if (msg && size)
        b = env->GetByteArrayElements(msg, nullptr);
    jint *p = nullptr;
    if (count && groups)
        p = env->GetIntArrayElements(groups, nullptr);
    bool ok = SpeakEx((USocket_Server_Handle) h, (const unsigned char*) b, (unsigned int) size, (const unsigned int*) p, (unsigned int) count);
    if (p)
        env->ReleaseIntArrayElements(groups, p, JNI_ABORT);
    if (b)
        env->ReleaseByteArrayElements(msg, b, JNI_ABORT);
    return ok;
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_SpeakExPush(JNIEnv *env, jclass, jbyteArray msg, jint size, jintArray groups, jint count) {
    jbyte *b = nullptr;
    if (msg && size)
        b = env->GetByteArrayElements(msg, nullptr);
    jint *p = nullptr;
    if (count && groups)
        p = env->GetIntArrayElements(groups, nullptr);
    bool ok = SpeakExPush((const unsigned char*) b, (unsigned int) size, (const unsigned int*) p, (unsigned int) count);
    if (p)
        env->ReleaseIntArrayElements(groups, p, JNI_ABORT);
    if (b)
        env->ReleaseByteArrayElements(msg, b, JNI_ABORT);
    return ok;
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_SendUserMessageEx(JNIEnv *env, jclass, jlong h, jstring userId, jbyteArray msg, jint size) {
    std::vector<jchar> v;
    const jchar *uid = nullptr;
    if (userId) {
        uid = env->GetStringChars(userId, nullptr);
        v.assign(uid, uid + env->GetStringLength(userId));
    }
    jchar c = 0;
    v.push_back(c); //null-terminated
    jbyte *b = nullptr;
    if (msg && size)
        b = env->GetByteArrayElements(msg, nullptr);
    bool ok = SendUserMessageEx((USocket_Server_Handle) h, (const wchar_t*) v.data(), (const unsigned char*) b, (unsigned int) size);
    if (b)
        env->ReleaseByteArrayElements(msg, b, JNI_ABORT);
    if (uid)
        env->ReleaseStringChars(userId, uid);
    return ok;
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_SendUserMessageExPush(JNIEnv *env, jclass, jstring userId, jbyteArray msg, jint size) {
    std::vector<jchar> v;
    const jchar *uid = nullptr;
    if (userId) {
        uid = env->GetStringChars(userId, nullptr);
        v.assign(uid, uid + env->GetStringLength(userId));
    }
    jchar c = 0;
    v.push_back(c); //null-terminated
    jbyte *b = nullptr;
    if (msg && size)
        b = env->GetByteArrayElements(msg, nullptr);
    bool ok = SendUserMessageExPush((const wchar_t*) v.data(), (const unsigned char*) b, (unsigned int) size);
    if (b)
        env->ReleaseByteArrayElements(msg, b, JNI_ABORT);
    if (uid)
        env->ReleaseStringChars(userId, uid);
    return ok;
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_SendUserMessage(JNIEnv *env, jclass, jlong h, jstring userId, jbyteArray msg, jint size) {
    std::vector<jchar> v;
    const jchar *uid = nullptr;
    if (userId) {
        uid = env->GetStringChars(userId, nullptr);
        v.assign(uid, uid + env->GetStringLength(userId));
    }
    jchar c = 0;
    v.push_back(c); //null-terminated
    jbyte *b = nullptr;
    if (msg && size)
        b = env->GetByteArrayElements(msg, nullptr);
    bool ok = SendUserMessage((USocket_Server_Handle) h, (const wchar_t*) v.data(), (const unsigned char*) b, (unsigned int) size);
    if (b)
        env->ReleaseByteArrayElements(msg, b, JNI_ABORT);
    if (uid)
        env->ReleaseStringChars(userId, uid);
    return ok;
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_SendUserMessagePush(JNIEnv *env, jclass, jstring userId, jbyteArray msg, jint size) {
    std::vector<jchar> v;
    const jchar *uid = nullptr;
    if (userId) {
        uid = env->GetStringChars(userId, nullptr);
        v.assign(uid, uid + env->GetStringLength(userId));
    }
    jchar c = 0;
    v.push_back(c); //null-terminated
    jbyte *b = nullptr;
    if (msg && size)
        b = env->GetByteArrayElements(msg, nullptr);
    bool ok = SendUserMessagePush((const wchar_t*) v.data(), (const unsigned char*) b, (unsigned int) size);
    if (b)
        env->ReleaseByteArrayElements(msg, b, JNI_ABORT);
    if (uid)
        env->ReleaseStringChars(userId, uid);
    return ok;
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetCountOfJoinedChatGroups(JNIEnv *, jclass, jlong h) {
    return (jint) GetCountOfJoinedChatGroups((USocket_Server_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetJoinedGroupIds(JNIEnv *env, jclass, jlong h, jintArray groups, jint count) {
    jint *p = nullptr;
    if (count && groups)
        p = env->GetIntArrayElements(groups, nullptr);
    unsigned int res = GetJoinedGroupIds((USocket_Server_Handle) h, (unsigned int*) p, (unsigned int) count);
    if (p)
        env->ReleaseIntArrayElements(groups, p, 0);
    return (jint) res;
}

JNIEXPORT jstring JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetPeerName(JNIEnv *env, jclass, jlong h, jintArray port, jint count) {
    unsigned int portNumber = 0;
    char name[1024] = {0};
    bool ok = GetPeerName((USocket_Server_Handle) h, &portNumber, name, sizeof (name));
    jint *p = nullptr;
    if (port && count) {
        p = env->GetIntArrayElements(port, nullptr);
        p[0] = (jint) portNumber;
    }
    if (p)
        env->ReleaseIntArrayElements(port, p, 0);
    return env->NewStringUTF(name);
}

JNIEXPORT jstring JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetLocalName(JNIEnv *env, jclass) {
    char name[1024] = {0};
    unsigned int res = GetLocalName(name, sizeof (name));
    return env->NewStringUTF(name);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_HasUserId(JNIEnv *env, jclass, jstring userId) {
    std::vector<jchar> v;
    const jchar *uid = nullptr;
    if (userId) {
        uid = env->GetStringChars(userId, nullptr);
        v.assign(uid, uid + env->GetStringLength(userId));
    }
    jchar c = 0;
    v.push_back(c); //null-terminated
    bool b = HasUserId((const wchar_t*) v.data());
    if (uid)
        env->ReleaseStringChars(userId, uid);
    return b;
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_DropCurrentSlowRequest(JNIEnv *, jclass, jlong h) {
    DropCurrentSlowRequest((USocket_Server_Handle) h);
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_AddAChatGroup(JNIEnv *env, jclass, jint chatId, jstring description) {
    std::vector<jchar> v;
    const jchar *des = nullptr;
    if (description) {
        des = env->GetStringChars(description, nullptr);
        v.assign(des, des + env->GetStringLength(description));
    }
    jchar c = 0;
    v.push_back(c); //null-terminated
    AddAChatGroup((unsigned int) chatId, (const wchar_t*) v.data());
    if (des)
        env->ReleaseStringChars(description, des);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetCountOfChatGroups(JNIEnv *, jclass) {
    return (jint) GetCountOfChatGroups();
}

JNIEXPORT jintArray JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetAllCreatedChatGroups(JNIEnv *env, jclass) {
    unsigned int groups = GetCountOfChatGroups() + 2;
    std::vector<unsigned int> v(0, groups);
    unsigned int res = GetAllCreatedChatGroups(v.data(), (unsigned int) v.size());
    jintArray arr = env->NewIntArray((jsize) res);
    env->SetIntArrayRegion(arr, 0, (jsize) res, (const jint*) v.data());
    return arr;
}

JNIEXPORT jstring JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetAChatGroup(JNIEnv *env, jclass, jint chatId) {
    jchar recv[4096] = {0};
    unsigned int res = GetAChatGroup((unsigned int) chatId, (wchar_t*) recv, sizeof (recv) / sizeof (jchar));
    return env->NewString(recv, (jsize) res);
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_RemoveChatGroup(JNIEnv *, jclass, jint chatId) {
    RemoveChatGroup((unsigned int) chatId);
}

JNIEXPORT jlong JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetSocketNativeHandle(JNIEnv *, jclass, jlong h) {
    return (jlong) GetSocketNativeHandle((USocket_Server_Handle) h);
}

JNIEXPORT jlong JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetCurrentRequestIndex(JNIEnv *, jclass, jlong h) {
    return (jlong) GetCurrentRequestIndex((USocket_Server_Handle) h);
}

JNIEXPORT jbyte JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetPeerOs(JNIEnv *env, jclass, jlong h, jbooleanArray endian, jint count) {
    bool end;
    jboolean *b = nullptr;
    if (endian && count)
        b = env->GetBooleanArrayElements(endian, nullptr);
    SPA::tagOperationSystem os = GetPeerOs((USocket_Server_Handle) h, &end);
    if (b) {
        b[0] = end;
        env->ReleaseBooleanArrayElements(endian, b, 0);
    }
    return (jbyte) os;
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_SendExceptionResultIndex(JNIEnv *env, jclass, jlong h, jlong index, jstring errMessage, jbyteArray errWhere, jshort reqId, jint errCode) {
    const jchar *errMsg = nullptr;
    jbyte *errPos = nullptr;
    std::vector<jchar> vMsg;
    std::string vWhere;
    if (errMessage) {
        jsize len = env->GetStringLength(errMessage);
        errMsg = env->GetStringChars(errMessage, nullptr);
        vMsg.assign(errMsg, errMsg + len);
    }
    if (errWhere) {
        jsize len = env->GetArrayLength(errWhere);
        errPos = env->GetByteArrayElements(errWhere, nullptr);
        vWhere.assign((const char*) errPos, (const char*) errPos + len);
    }
    unsigned int res = SendExceptionResultIndex((USocket_Server_Handle) h, (SPA::UINT64) index, (const wchar_t*) vMsg.data(), vWhere.c_str(), (unsigned short) reqId, (unsigned int) errCode);
    if (errPos)
        env->ReleaseByteArrayElements(errWhere, errPos, JNI_ABORT);
    if (errMsg)
        env->ReleaseStringChars(errMessage, errMsg);
    return (jint) res;
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_SendExceptionResult(JNIEnv *env, jclass, jlong h, jstring errMessage, jbyteArray errWhere, jshort reqId, jint errCode) {
    const jchar *errMsg = nullptr;
    jbyte *errPos = nullptr;
    std::vector<jchar> vMsg;
    std::string vWhere;
    if (errMessage) {
        jsize len = env->GetStringLength(errMessage);
        errMsg = env->GetStringChars(errMessage, nullptr);
        vMsg.assign(errMsg, errMsg + len);
    }
    if (errWhere) {
        jsize len = env->GetArrayLength(errWhere);
        errPos = env->GetByteArrayElements(errWhere, nullptr);
        vWhere.assign((const char*) errPos, (const char*) errPos + len);
    }
    unsigned int res = SendExceptionResult((USocket_Server_Handle) h, (const wchar_t*) vMsg.data(), vWhere.c_str(), (unsigned short) reqId, (unsigned int) errCode);
    if (errPos)
        env->ReleaseByteArrayElements(errWhere, errPos, JNI_ABORT);
    if (errMsg)
        env->ReleaseStringChars(errMessage, errMsg);
    return (jint) res;
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_MakeRequest(JNIEnv *env, jclass, jlong h, jshort reqId, jbyteArray buffer, jint count) {
    jbyte *b = nullptr;
    if (buffer && count)
        b = env->GetByteArrayElements(buffer, nullptr);
    else
        count = 0;
    bool ok = MakeRequest((USocket_Server_Handle) h, (unsigned short) reqId, (const unsigned char*) b, (unsigned int) count);
    if (b)
        env->ReleaseByteArrayElements(buffer, b, JNI_ABORT);
    return ok;
}

JNIEXPORT jobject JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetHTTPRequestHeaders(JNIEnv *env, jclass, jlong h) {
    SPA::ServerSide::CHttpHeaderValue hv[64];
    unsigned int n, res = GetHTTPRequestHeaders((USocket_Server_Handle) h, hv, 64);
    jobjectArray arrObj = env->NewObjectArray((jsize) res, g_clsCHttpHeaderValue, nullptr);
    for (n = 0; n < res; ++n) {
        jobject obj = env->NewObject(g_clsCHttpHeaderValue, g_constructorHttpHeaderValue);
        env->SetObjectField(obj, g_fidHeader, env->NewStringUTF(hv[n].Header));
        env->SetObjectField(obj, g_fidValue, env->NewStringUTF(hv[n].Value));
        env->SetObjectArrayElement(arrObj, (jsize) n, obj);
    }
    return arrObj;
}

JNIEXPORT jstring JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetHTTPPath(JNIEnv *env, jclass, jlong h) {
    return env->NewStringUTF(GetHTTPPath((USocket_Server_Handle) h));
}

JNIEXPORT jlong JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetHTTPContentLength(JNIEnv *, jclass, jlong h) {
    return (jlong) GetHTTPContentLength((USocket_Server_Handle) h);
}

JNIEXPORT jstring JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetHTTPQuery(JNIEnv *env, jclass, jlong h) {
    return env->NewStringUTF(GetHTTPQuery((USocket_Server_Handle) h));
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_DownloadFile(JNIEnv *env, jclass, jlong h, jbyteArray filePath, jint len) {
    std::string path;
    jbyte *p = nullptr;
    if (filePath && len) {
        p = env->GetByteArrayElements(filePath, nullptr);
        path.assign((const char*) p, (const char*) p + len);
    }
    bool ok = DownloadFile((USocket_Server_Handle) h, path.c_str());
    if (p)
        env->ReleaseByteArrayElements(filePath, p, JNI_ABORT);
    return ok;
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetHTTPMethod(JNIEnv *, jclass, jlong h) {
    return (jint) GetHTTPMethod((USocket_Server_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_HTTPKeepAlive(JNIEnv *, jclass, jlong h) {
    return HTTPKeepAlive((USocket_Server_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_IsWebSocket(JNIEnv *, jclass, jlong h) {
    return IsWebSocket((USocket_Server_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_IsCrossDomain(JNIEnv *, jclass, jlong h) {
    return IsCrossDomain((USocket_Server_Handle) h);
}

JNIEXPORT jdouble JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetHTTPVersion(JNIEnv *, jclass, jlong h) {
    return GetHTTPVersion((USocket_Server_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_HTTPGZipAccepted(JNIEnv *, jclass, jlong h) {
    return HTTPGZipAccepted((USocket_Server_Handle) h);
}

JNIEXPORT jstring JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetHTTPUrl(JNIEnv *env, jclass, jlong h) {
    return env->NewStringUTF(GetHTTPUrl((USocket_Server_Handle) h));
}

JNIEXPORT jstring JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetHTTPHost(JNIEnv *env, jclass, jlong h) {
    return env->NewStringUTF(GetHTTPHost((USocket_Server_Handle) h));
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetHTTPTransport(JNIEnv *, jclass, jlong h) {
    return (jint) GetHTTPTransport((USocket_Server_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetHTTPTransferEncoding(JNIEnv *, jclass, jlong h) {
    return (jint) GetHTTPTransferEncoding((USocket_Server_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetHTTPContentMultiplax(JNIEnv *, jclass, jlong h) {
    return (jint) GetHTTPContentMultiplax((USocket_Server_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetHTTPResponseCode(JNIEnv *, jclass, jlong h, jint code) {
    return SetHTTPResponseCode((USocket_Server_Handle) h, (unsigned int) code);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetOptimistic(JNIEnv *, jclass, jint qHandle) {
    return (jint) GetOptimistic((unsigned int) qHandle);
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetOptimistic(JNIEnv *, jclass, jint qHandle, jint optimistic) {
    SetOptimistic((unsigned int) qHandle, (SPA::tagOptimistic) optimistic);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetHTTPResponseHeader(JNIEnv *env, jclass, jlong h, jbyteArray header, jbyteArray value) {
    jbyte *hpos = nullptr;
    jbyte *vpos = nullptr;
    std::string hd;
    std::string v;
    if (header) {
        jsize len = env->GetArrayLength(header);
        hpos = env->GetByteArrayElements(header, nullptr);
        hd.assign((const char*) hpos, (const char*) hpos + len);
    }
    if (value) {
        jsize len = env->GetArrayLength(value);
        vpos = env->GetByteArrayElements(value, nullptr);
        v.assign((const char*) vpos, (const char*) vpos + len);
    }
    bool ok = SetHTTPResponseHeader((USocket_Server_Handle) h, hd.c_str(), v.c_str());
    if (hpos)
        env->ReleaseByteArrayElements(header, hpos, JNI_ABORT);
    if (vpos)
        env->ReleaseByteArrayElements(value, vpos, JNI_ABORT);
    return ok;
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_SendHTTPReturnDataA(JNIEnv *env, jclass, jlong h, jbyteArray str, jint chars) {
    char *s = nullptr;
    std::string temp;
    if (str && chars) {
        s = (char*) env->GetByteArrayElements(str, nullptr);
        temp.assign(s, s + (int) chars);
    }
    unsigned int res = SendHTTPReturnDataA((USocket_Server_Handle) h, (const char*) temp.c_str(), (unsigned int) chars);
    if (s)
        env->ReleaseByteArrayElements(str, (jbyte*) s, JNI_ABORT);
    return (jint) res;
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_SendHTTPReturnDataW(JNIEnv *env, jclass, jlong h, jstring str, jint chars) {
    const jchar *s = nullptr;
    if (str)
        s = env->GetStringChars(str, nullptr);
    unsigned int res = SendHTTPReturnDataW((USocket_Server_Handle) h, (const wchar_t*) s, (unsigned int) chars);
    if (s)
        env->ReleaseStringChars(str, s);
    return (jint) res;
}

JNIEXPORT jstring JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetHTTPId(JNIEnv *env, jclass, jlong h) {
    return env->NewStringUTF(GetHTTPId((USocket_Server_Handle) h));
}

JNIEXPORT jobject JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetHTTPCurrentMultiplaxHeaders(JNIEnv *env, jclass, jlong h) {
    SPA::ServerSide::CHttpHeaderValue hv[32];
    unsigned int n, res = GetHTTPCurrentMultiplaxHeaders((USocket_Server_Handle) h, hv, 32);
    jobjectArray arrObj = env->NewObjectArray((jsize) res, g_clsCHttpHeaderValue, nullptr);
    for (n = 0; n < res; ++n) {
        jobject obj = env->NewObject(g_clsCHttpHeaderValue, g_constructorHttpHeaderValue);
        env->SetObjectField(obj, g_fidHeader, env->NewStringUTF(hv[n].Header));
        env->SetObjectField(obj, g_fidValue, env->NewStringUTF(hv[n].Value));
        env->SetObjectArrayElement(arrObj, (jsize) n, obj);
    }
    return arrObj;
}

JNIEXPORT jlong JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetSSL(JNIEnv *, jclass, jlong h) {
    return (jlong) GetSSL((USocket_Server_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetReturnRandom(JNIEnv *, jclass, jint svsId) {
    return GetReturnRandom((unsigned int) svsId);
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetReturnRandom(JNIEnv *, jclass, jint svsId, jboolean rand) {
    SetReturnRandom((unsigned int) svsId, rand ? true : false);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetSwitchTime(JNIEnv *, jclass) {
    return (jint) GetSwitchTime();
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetSwitchTime(JNIEnv *, jclass, jint newTime) {
    SetSwitchTime((unsigned int) newTime);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetCountOfClients(JNIEnv *, jclass) {
    return (jint) GetCountOfClients();
}

JNIEXPORT jlong JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetClient(JNIEnv *, jclass, jint index) {
    return (jlong) GetClient((unsigned int) index);
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetDefaultZip(JNIEnv *, jclass, jboolean b) {
    SetDefaultZip(b ? true : false);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetDefaultZip(JNIEnv *, jclass) {
    return GetDefaultZip();
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetMaxConnectionsPerClient(JNIEnv *, jclass, jint max) {
    SetMaxConnectionsPerClient((unsigned int) max);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetMaxConnectionsPerClient(JNIEnv *, jclass) {
    return (jint) GetMaxConnectionsPerClient();
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetMaxThreadIdleTimeBeforeSuicide(JNIEnv *, jclass, jint max) {
    SetMaxThreadIdleTimeBeforeSuicide((unsigned int) max);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetMaxThreadIdleTimeBeforeSuicide(JNIEnv *, jclass) {
    return (jint) GetMaxThreadIdleTimeBeforeSuicide();
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetTimerElapse(JNIEnv *, jclass, jint time) {
    SetTimerElapse((unsigned int) time);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetTimerElapse(JNIEnv *, jclass) {
    return (jint) GetTimerElapse();
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetSMInterval(JNIEnv *, jclass) {
    return (jint) GetSMInterval();
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetSMInterval(JNIEnv *, jclass, jint sm) {
    SetSMInterval((unsigned int) sm);
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetPingInterval(JNIEnv *, jclass, jint ps) {
    SetPingInterval((unsigned int) ps);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetPingInterval(JNIEnv *, jclass) {
    return (jint) GetPingInterval();
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetRecycleGlobalMemoryInterval(JNIEnv *, jclass, jint rgm) {
    SetRecycleGlobalMemoryInterval((unsigned int) rgm);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetRecycleGlobalMemoryInterval(JNIEnv *, jclass) {
    return (jint) GetRecycleGlobalMemoryInterval();
}

JNIEXPORT jlong JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetRequestCount(JNIEnv *, jclass) {
    return (jlong) GetRequestCount();
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_StartHTTPChunkResponse(JNIEnv *, jclass, jlong h) {
    return (jint) StartHTTPChunkResponse((USocket_Server_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_IsDequeuedMessageAborted(JNIEnv *, jclass, jlong h) {
    return IsDequeuedMessageAborted((USocket_Server_Handle) h);
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_AbortDequeuedMessage(JNIEnv *, jclass, jlong h) {
    AbortDequeuedMessage((USocket_Server_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_SendHTTPChunk(JNIEnv *env, jclass, jlong h, jbyteArray buffer, jint len) {
    jbyte *b = nullptr;
    if (buffer && len)
        b = env->GetByteArrayElements(buffer, nullptr);
    unsigned int res = SendHTTPChunk((USocket_Server_Handle) h, (const unsigned char*) b, (unsigned int) len);
    if (b)
        env->ReleaseByteArrayElements(buffer, b, JNI_ABORT);
    return (jint) res;
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_EndHTTPChunkResponse(JNIEnv *env, jclass, jlong h, jbyteArray buffer, jint len) {
    jbyte *b = nullptr;
    if (buffer && len)
        b = env->GetByteArrayElements(buffer, nullptr);
    unsigned int res = EndHTTPChunkResponse((USocket_Server_Handle) h, (const unsigned char*) b, (unsigned int) len);
    if (b)
        env->ReleaseByteArrayElements(buffer, b, JNI_ABORT);
    return (jint) res;
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_IsFakeRequest(JNIEnv *, jclass, jlong h) {
    return IsFakeRequest((USocket_Server_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetZip(JNIEnv *, jclass, jlong h, jboolean zip) {
    return SetZip((USocket_Server_Handle) h, zip ? true : false);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetZip(JNIEnv *, jclass, jlong h) {
    return GetZip((USocket_Server_Handle) h);
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetZipLevel(JNIEnv *, jclass, jlong h, jint zl) {
    SetZipLevel((USocket_Server_Handle) h, (SPA::tagZipLevel)zl);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetZipLevel(JNIEnv *, jclass, jlong h) {
    return (jint) GetZipLevel((USocket_Server_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_StartQueue(JNIEnv *env, jclass, jbyteArray qName, jboolean dequeueShared, jint ttl) {
    std::string q;
    jbyte *n = nullptr;
    if (qName) {
        jsize len = env->GetArrayLength(qName);
        n = env->GetByteArrayElements(qName, nullptr);
        q.assign((const char*) n, (const char*) n + len);
    }
    unsigned int res = StartQueue(q.c_str(), dequeueShared ? true : false, (unsigned int) ttl);
    if (n)
        env->ReleaseByteArrayElements(qName, n, JNI_ABORT);
    return (jint) res;
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetMessagesInDequeuing(JNIEnv *, jclass, jint qHandle) {
    return (jint) GetMessagesInDequeuing((unsigned int) qHandle);
}

JNIEXPORT jlong JNICALL Java_SPA_ServerSide_ServerCoreLoader_Enqueue(JNIEnv *env, jclass, jint qHandle, jshort reqId, jbyteArray buffer, jint len) {
    jbyte *b = nullptr;
    if (buffer && len)
        b = env->GetByteArrayElements(buffer, nullptr);
    jlong res = (jlong) Enqueue((unsigned int) qHandle, (unsigned short) reqId, (const unsigned char*) b, (unsigned int) len);
    if (b)
        env->ReleaseByteArrayElements(buffer, b, JNI_ABORT);
    return res;
}

JNIEXPORT jlong JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetMessageCount(JNIEnv *, jclass, jint qHandle) {
    return (jlong) GetMessageCount((unsigned int) qHandle);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_StopQueueByHandle(JNIEnv *, jclass, jint qHandle, jboolean permanent) {
    return StopQueueByHandle((unsigned int) qHandle, permanent ? true : false);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_StopQueueByName(JNIEnv *env, jclass, jbyteArray qName, jboolean permanent) {
    std::string q;
    jbyte *n = nullptr;
    if (qName) {
        jsize len = env->GetArrayLength(qName);
        n = env->GetByteArrayElements(qName, nullptr);
        q.assign((const char*) n, (const char*) n + len);
    }
    bool ok = StopQueueByName(q.c_str(), permanent ? true : false);
    if (n)
        env->ReleaseByteArrayElements(qName, n, JNI_ABORT);
    return ok;
}

JNIEXPORT jlong JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetQueueSize(JNIEnv *, jclass, jint qHandle) {
    return (jlong) GetQueueSize((unsigned int) qHandle);
}

JNIEXPORT jlong JNICALL Java_SPA_ServerSide_ServerCoreLoader_Dequeue(JNIEnv *env, jclass, jint qHandle, jlong h, jint messageCount, jboolean beNotifiedWhenAvailable, jint waitTime) {
    return (jlong) Dequeue((unsigned int) qHandle, (USocket_Server_Handle) h, (unsigned int) messageCount, beNotifiedWhenAvailable ? true : false, (unsigned int) waitTime);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_IsQueueStartedByName(JNIEnv *env, jclass, jbyteArray qName) {
    std::string q;
    jbyte *n = nullptr;
    if (qName) {
        jsize len = env->GetArrayLength(qName);
        n = env->GetByteArrayElements(qName, nullptr);
        q.assign((const char*) n, (const char*) n + len);
    }
    bool ok = IsQueueStartedByName(q.c_str());
    if (n)
        env->ReleaseByteArrayElements(qName, n, JNI_ABORT);
    return ok;
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_IsQueueStartedByHandle(JNIEnv *, jclass, jint qHandle) {
    return IsQueueStartedByHandle((unsigned int) qHandle);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_IsQueueSecuredByName(JNIEnv *env, jclass, jbyteArray qName) {
    std::string q;
    jbyte *n = nullptr;
    if (qName) {
        jsize len = env->GetArrayLength(qName);
        n = env->GetByteArrayElements(qName, nullptr);
        q.assign((const char*) n, (const char*) n + len);
    }
    bool ok = IsQueueSecuredByName(q.c_str());
    if (n)
        env->ReleaseByteArrayElements(qName, n, JNI_ABORT);
    return ok;
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_IsQueueSecuredByHandle(JNIEnv *, jclass, jint qHandle) {
    return IsQueueSecuredByHandle((unsigned int) qHandle);
}

JNIEXPORT jstring JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetQueueName(JNIEnv *env, jclass, jint qHandle) {
    return env->NewStringUTF(GetQueueName((unsigned int) qHandle));
}

JNIEXPORT jstring JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetQueueFileName(JNIEnv *env, jclass, jint qHandle) {
    return env->NewStringUTF(GetQueueFileName((unsigned int) qHandle));
}

JNIEXPORT jlong JNICALL Java_SPA_ServerSide_ServerCoreLoader_Dequeue2(JNIEnv *, jclass, jint qHandle, jlong h, jint maxBytes, jboolean beNotifiedWhenAvailable, jint waitTime) {
    return (jlong) Dequeue2((unsigned int) qHandle, (USocket_Server_Handle) h, (unsigned int) maxBytes, beNotifiedWhenAvailable ? true : false, (unsigned int) waitTime);
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_EnableClientDequeue(JNIEnv *, jclass, jlong h, jboolean enabled) {
    EnableClientDequeue((USocket_Server_Handle) h, enabled ? true : false);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_IsDequeueRequest(JNIEnv *, jclass, jlong h) {
    return IsDequeueRequest((USocket_Server_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_AbortJob(JNIEnv *, jclass, jint qHandle) {
    return AbortJob((unsigned int) qHandle);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_StartJob(JNIEnv *, jclass, jint qHandle) {
    return StartJob((unsigned int) qHandle);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_EndJob(JNIEnv *, jclass, jint qHandle) {
    return EndJob((unsigned int) qHandle);
}

JNIEXPORT jlong JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetJobSize(JNIEnv *, jclass, jint qHandle) {
    return (jlong) GetJobSize((unsigned int) qHandle);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetRouting(JNIEnv *, jclass, jint svsId0, jint ra0, jint svsId1, jint ra1) {
    return SetRouting((unsigned int) svsId0, (SPA::ServerSide::tagRoutingAlgorithm)ra0, (unsigned int) svsId1, (SPA::ServerSide::tagRoutingAlgorithm)ra1);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_CheckRouting(JNIEnv *, jclass, jint svsId) {
    return (jint) CheckRouting((unsigned int) svsId);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_AddAlphaRequest(JNIEnv *, jclass, jint svsId, jshort reqId) {
    return AddAlphaRequest((unsigned int) svsId, (unsigned short) reqId);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetAlphaRequestIds(JNIEnv *env, jclass, jint svsId, jshortArray reqIds, jint count) {
    jshort *ids = nullptr;
    if (reqIds && count)
        ids = env->GetShortArrayElements(reqIds, nullptr);
    else
        count = 0;
    unsigned int res = GetAlphaRequestIds((unsigned int) svsId, (unsigned short*) ids, (unsigned int) count);
    if (ids)
        env->ReleaseShortArrayElements(reqIds, ids, JNI_ABORT);
    return (jint) res;
}

JNIEXPORT jlong JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetQueueLastIndex(JNIEnv *, jclass, jint qHandle) {
    return (jlong) GetQueueLastIndex((unsigned int) qHandle);
}

JNIEXPORT jlong JNICALL Java_SPA_ServerSide_ServerCoreLoader_CancelQueuedRequestsByIndex(JNIEnv *, jclass, jint qHandle, jlong start, jlong end) {
    return (jlong) CancelQueuedRequestsByIndex((unsigned int) qHandle, (SPA::UINT64)start, (SPA::UINT64)end);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_IsDequeueShared(JNIEnv *, jclass, jint qHandle) {
    return IsDequeueShared((unsigned int) qHandle);
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetServerQueueStatus(JNIEnv *, jclass, jint qHandle) {
    return (jint) GetServerQueueStatus((unsigned int) qHandle);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_PushQueueTo(JNIEnv *env, jclass, jint qHandle, jintArray handles, jint count) {
    jint *h = nullptr;
    if (handles && count)
        h = env->GetIntArrayElements(handles, nullptr);
    else
        count = 0;
    bool ok = PushQueueTo((unsigned int) qHandle, (const unsigned int*) h, (unsigned int) count);
    if (h)
        env->ReleaseIntArrayElements(handles, h, JNI_ABORT);
    return ok;
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetMainThreads(JNIEnv *, jclass) {
    return ::GetMainThreads();
}

JNIEXPORT jint JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetTTL(JNIEnv *, jclass, jint qHandle) {
    return (jint) GetTTL((unsigned int) qHandle);
}

JNIEXPORT jlong JNICALL Java_SPA_ServerSide_ServerCoreLoader_RemoveQueuedRequestsByTTL(JNIEnv *, jclass, jint qHandle) {
    return (jlong) RemoveQueuedRequestsByTTL((unsigned int) qHandle);
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_ResetQueue(JNIEnv *, jclass, jint qHandle) {
    ResetQueue((unsigned int) qHandle);
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_ServerCoreLoader_IsServerQueueIndexPossiblyCrashed(JNIEnv *, jclass) {
    return IsServerQueueIndexPossiblyCrashed();
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetServerWorkDirectory(JNIEnv *env, jclass, jbyteArray dir) {
    jbyte *d = nullptr;
    std::string path;
    if (dir) {
        jsize len = env->GetArrayLength(dir);
        d = env->GetByteArrayElements(dir, nullptr);
        path.assign((const char*) d, (const char*) d + len);
    }
    SetServerWorkDirectory(path.c_str());
    if (d)
        env->ReleaseByteArrayElements(dir, d, JNI_ABORT);
}

JNIEXPORT jstring JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetServerWorkDirectory(JNIEnv *env, jclass) {
    return env->NewStringUTF(GetServerWorkDirectory());
}

JNIEXPORT jlong JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetLastQueueMessageTime(JNIEnv *, jclass, jint qHandle) {
    return (jlong) GetLastQueueMessageTime((unsigned int) qHandle);
}

JNIEXPORT jstring JNICALL Java_SPA_ServerSide_ServerCoreLoader_GetUServerSocketVersion(JNIEnv *env, jclass) {
    return env->NewStringUTF(GetUServerSocketVersion());
}

JNIEXPORT void JNICALL Java_SPA_ServerSide_ServerCoreLoader_SetMessageQueuePassword(JNIEnv *env, jclass, jbyteArray pwd) {
    std::string sp;
    jbyte *p = nullptr;
    if (pwd) {
        jsize len = env->GetArrayLength(pwd);
        p = env->GetByteArrayElements(pwd, nullptr);
        sp.assign((const char*) p, (const char*) p + len);
    }
    SetMessageQueuePassword(sp.c_str());
    if (p)
        env->ReleaseByteArrayElements(pwd, p, JNI_ABORT);
}

void CALLBACK OnCloseGlobal(USocket_Server_Handle Handler, int errCode) {
    JNIEnv *env;
    jint es = g_vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    env->CallStaticVoidMethod(g_clsServer, g_midOnCloseGlobal, (jlong) Handler, (jint) errCode);
    CleanException(env);
}

void CALLBACK OnSSLHandShakeCompleted(USocket_Server_Handle Handler, int errCode) {
    JNIEnv *env;
    jint es = g_vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    env->CallStaticVoidMethod(g_clsServer, g_midOnSSLHandShakeCompleted, (jlong) Handler, (jint) errCode);
    CleanException(env);
}

void CALLBACK OnAccept(USocket_Server_Handle Handler, int errCode) {
    JNIEnv *env;
    jint es = g_vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    env->CallStaticVoidMethod(g_clsServer, g_midOnAccept, (jlong) Handler, (jint) errCode);
    CleanException(env);
}

void CALLBACK OnIdle(SPA::INT64 milliseconds) {
    JNIEnv *env;
    jint es = g_vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    env->CallStaticVoidMethod(g_clsServer, g_midOnIdle, (jlong) milliseconds);
    CleanException(env);
}

bool CALLBACK OnIsPermitted(USocket_Server_Handle h, unsigned int serviceId) {
    JNIEnv *env;
    jint es = g_vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    jboolean b = env->CallStaticBooleanMethod(g_clsServer, g_midOnIsPermitted, (jlong) h, (jint) serviceId);
    CleanException(env);
    return b ? true : false;
}

void CALLBACK OnClose(USocket_Server_Handle h, int errCode) {
    JNIEnv *env;
    jint es = g_vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(es == JNI_OK);
    env->CallStaticVoidMethod(g_clsCSocketPeer, g_midOnClose, (jlong) h, (jint) errCode);
    CleanException(env);
}

void CALLBACK OnResultsSent(USocket_Server_Handle h) {
    JNIEnv *env;
    jint es = g_vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    env->CallStaticVoidMethod(g_clsCSocketPeer, g_midOnResultsSent, (jlong) h);
    CleanException(env);
}

void CALLBACK OnRequestArrive(USocket_Server_Handle h, unsigned short requestId, unsigned int len) {
    JNIEnv *env;
    jint es = g_vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(es == JNI_OK);
    env->CallStaticVoidMethod(g_clsCSocketPeer, g_midOnRequestArrive, (jlong) h, (jshort) requestId, (jint) len);
    CleanException(env);
}

void CALLBACK OnFastRequestArrive(USocket_Server_Handle h, unsigned short requestId, unsigned int len) {
    JNIEnv *env;
    jint es = g_vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(es == JNI_OK);
    env->CallStaticVoidMethod(g_clsCSocketPeer, g_midOnFastRequestArrive, (jlong) h, (jshort) requestId, (jint) len);
    CleanException(env);
}

int CALLBACK SLOW_PROCESS(unsigned short requestId, unsigned int len, USocket_Server_Handle h) {
    jint res = 0;
    JNIEnv *env;
    jint es = g_vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(es == JNI_OK);
    res = env->CallStaticIntMethod(g_clsCSocketPeer, g_midSLOW_PROCESS, (jlong) h, (jshort) requestId, (jint) len);
    CleanException(env);
    return (int) res;
}

void CALLBACK OnRequestProcessed(USocket_Server_Handle h, unsigned short requestId) {
    JNIEnv *env;
    jint es = g_vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(es == JNI_OK);
    env->CallStaticVoidMethod(g_clsCSocketPeer, g_midOnRequestProcessed, (jlong) h, (jshort) requestId);
    CleanException(env);
}

void CALLBACK OnBaseRequestCame(USocket_Server_Handle h, unsigned short requestId) {
    JNIEnv *env;
    jint es = g_vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(es == JNI_OK);
    env->CallStaticVoidMethod(g_clsCSocketPeer, g_midOnBaseRequestCame, (jlong) h, (jshort) requestId);
    CleanException(env);
}

void CALLBACK OnSwitchTo(USocket_Server_Handle h, unsigned int oldServiceId, unsigned int newServiceId) {
    JNIEnv *env;
    jint es = g_vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(es == JNI_OK);
    env->CallStaticVoidMethod(g_clsCSocketPeer, g_midOnSwitchTo, (jlong) h, (jint) oldServiceId, (jint) newServiceId);
    CleanException(env);
}

void CALLBACK OnChatRequestComing(USocket_Server_Handle h, SPA::tagChatRequestID chatRequestID, unsigned int len) {
    JNIEnv *env;
    jint es = g_vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(es == JNI_OK);
    env->CallStaticVoidMethod(g_clsCSocketPeer, g_midOnChatRequestComing, (jlong) h, (jshort) chatRequestID, (jint) len);
    CleanException(env);
}

void CALLBACK OnChatRequestCame(USocket_Server_Handle h, SPA::tagChatRequestID chatRequestId) {
    JNIEnv *env;
    jint es = g_vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(es == JNI_OK);
    env->CallStaticVoidMethod(g_clsCSocketPeer, g_midOnChatRequestCame, (jlong) h, (jshort) chatRequestId);
    CleanException(env);
}

#ifndef WIN32_64

unsigned int GetLen(const SPA::UTF16 *chars) {
    if (!chars)
        return 0;
    unsigned int len = 0;
    while (*chars) {
        ++len;
        ++chars;
    }
    return len;
}

#endif

bool CALLBACK OnHttpAuthentication(USocket_Server_Handle h, const wchar_t *userId, const wchar_t *password) {
    JNIEnv *env;
#ifdef WIN32_64
    jsize idLen = (jsize)::wcslen(userId);
    jsize pwdLen = (jsize)::wcslen(password);
#else
    jsize idLen = (jsize) GetLen((const SPA::UTF16 *)userId);
    jsize pwdLen = (jsize) GetLen((const SPA::UTF16 *)password);
#endif
    jboolean res = false;
    jint es = g_vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(es == JNI_OK);
    res = env->CallStaticBooleanMethod(g_clsCHttpPeerBase, g_midOnHttpAuthentication, (jlong) h, env->NewString((const jchar*) userId, idLen), env->NewString((const jchar*) password, pwdLen));
    CleanException(env);
    return res ? true : false;
}

void CALLBACK OnThreadEvent(SPA::ServerSide::tagThreadEvent te) {
    switch (te) {
        case SPA::ServerSide::teStarted:
#ifndef NDEBUG
            std::cout << "++++ ServerSide::teStarted @" << __FUNCTION__ << "/" << __LINE__ << std::endl;
#endif
            SetCurrentThreadAsDaemon();
            break;
        case SPA::ServerSide::teKilling:
            RemoveCurrentThreadAsDaemon();
#ifndef NDEBUG
            std::cout << "---- ServerSide::teKilling @" << __FUNCTION__ << "/" << __LINE__ << std::endl;
#endif
            break;
        default:
            assert(false);
            break;
    }
}

HINSTANCE g_hSqlite = nullptr;
typedef void (WINAPI *PSetSqliteDBGlobalConnectionString)(const wchar_t *dbConnection);
PSetSqliteDBGlobalConnectionString SetSqliteDBGlobalConnectionString = nullptr;

JNIEXPORT void JNICALL Java_SPA_ServerSide_Sqlite_SetSqliteDBGlobalConnectionString(JNIEnv *env, jclass pClass, jstring dbString) {
    SPA::CAutoLock al(g_co);
    if (!g_hSqlite) {
#ifdef WIN32_64
        g_hSqlite = ::LoadLibraryW(L"ssqlite.dll");
#else
        g_hSqlite = ::dlopen("libssqlite.so", RTLD_LAZY);
#endif
    }
    if (g_hSqlite && !SetSqliteDBGlobalConnectionString) {
        SetSqliteDBGlobalConnectionString = (PSetSqliteDBGlobalConnectionString)::GetProcAddress(g_hSqlite, "SetSqliteDBGlobalConnectionString");
    }
    if (g_hSqlite && SetSqliteDBGlobalConnectionString) {
        std::wstring s;
        if (dbString) {
            jsize len = env->GetStringLength(dbString);
            const jchar *p = env->GetStringChars(dbString, nullptr);
            s.assign(p, p + len);
            env->ReleaseStringChars(dbString, p);
        }
        SetSqliteDBGlobalConnectionString(s.c_str());
    }
}

HINSTANCE g_hStreamFile = nullptr;
typedef void (WINAPI *PSetRootDirectory)(const wchar_t *root);
PSetRootDirectory SetRootDirectory = nullptr;

JNIEXPORT void JNICALL Java_SPA_ServerSide_Sfile_SetRootDirectory(JNIEnv *env, jclass pClass, jstring root) {
    SPA::CAutoLock al(g_co);
    if (!g_hStreamFile) {
#ifdef WIN32_64
        g_hStreamFile = ::LoadLibraryW(L"ustreamfile.dll");
#else
        g_hStreamFile = ::dlopen("libustreamfile.so", RTLD_LAZY);
#endif
    }
    if (g_hStreamFile && !SetRootDirectory) {
        SetRootDirectory = (PSetRootDirectory)::GetProcAddress(g_hStreamFile, "SetRootDirectory");
    }
    if (g_hStreamFile && SetRootDirectory && root) {
        jsize len = env->GetStringLength(root);
        const jchar *p = env->GetStringChars(root, nullptr);
        std::wstring s(p, p + len);
        SetRootDirectory(s.c_str());
        env->ReleaseStringChars(root, p);
    }
}

HINSTANCE g_hOdbc = nullptr;
typedef bool(WINAPI *PDoODBCAuthentication)(USocket_Server_Handle hSocket, const wchar_t *userId, const wchar_t *password, unsigned int nSvsId, const wchar_t *odbcDriver, const wchar_t *dsn);
typedef void (WINAPI *PSetOdbcDBGlobalConnectionString)(const wchar_t *dbConnection);
static PSetOdbcDBGlobalConnectionString SetOdbcDBGlobalConnectionString = nullptr;
static PDoODBCAuthentication DoODBCAuthentication = nullptr;

JNIEXPORT void JNICALL Java_SPA_ServerSide_Odbc_SetOdbcDBGlobalConnectionString(JNIEnv *env, jclass, jstring dbConnection) {
    SPA::CAutoLock al(g_co);
    if (!g_hOdbc) {
#ifdef WIN32_64
        g_hOdbc = ::LoadLibraryW(L"sodbc.dll");
#else
        g_hOdbc = ::dlopen("libsodbc.so", RTLD_LAZY);
#endif
    }
    if (g_hOdbc && !SetOdbcDBGlobalConnectionString) {
        SetOdbcDBGlobalConnectionString = (PSetOdbcDBGlobalConnectionString)::GetProcAddress(g_hOdbc, "SetOdbcDBGlobalConnectionString");
    }
    if (g_hOdbc && SetOdbcDBGlobalConnectionString) {
        std::wstring s;
        if (dbConnection) {
            jsize len = env->GetStringLength(dbConnection);
            const jchar *p = env->GetStringChars(dbConnection, nullptr);
            s.assign(p, p + len);
            env->ReleaseStringChars(dbConnection, p);
        }
        SetOdbcDBGlobalConnectionString(s.c_str());
    }
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_Odbc_DoODBCAuthentication(JNIEnv *env, jclass, jlong h, jstring uid, jstring pwd, jint svsId, jstring driver, jstring dsn) {
    SPA::CAutoLock al(g_co);
    if (!g_hOdbc) {
#ifdef WIN32_64
        g_hOdbc = ::LoadLibraryW(L"sodbc.dll");
#else
        g_hOdbc = ::dlopen("libsodbc.so", RTLD_LAZY);
#endif
    }
    if (g_hOdbc && !DoODBCAuthentication) {
        DoODBCAuthentication = (PDoODBCAuthentication)::GetProcAddress(g_hOdbc, "DoODBCAuthentication");
    }
    bool ok = false;
    if (g_hOdbc && DoODBCAuthentication) {
        jsize len;
        std::wstring wuid, wpwd, wdriver, wdsn;
        if (uid) {
            len = env->GetStringLength(uid);
            const jchar *puid = env->GetStringChars(uid, nullptr);
            wuid.assign(puid, puid + len);
            env->ReleaseStringChars(uid, puid);
        }
        if (pwd) {
            len = env->GetStringLength(pwd);
            const jchar *ppwd = env->GetStringChars(pwd, nullptr);
            wpwd.assign(ppwd, ppwd + len);
            env->ReleaseStringChars(pwd, ppwd);
        }
        if (dsn) {
            len = env->GetStringLength(dsn);
            const jchar *pdb = env->GetStringChars(dsn, nullptr);
            wdsn.assign(pdb, pdb + len);
            env->ReleaseStringChars(dsn, pdb);
        }
        if (driver) {
            jsize len = env->GetStringLength(driver);
            const jchar *p = env->GetStringChars(driver, nullptr);
            wdriver.assign(p, p + len);
            env->ReleaseStringChars(driver, p);
        }
        ok = DoODBCAuthentication((USocket_Server_Handle) h, wuid.c_str(), wpwd.c_str(), (unsigned int) svsId, wdriver.c_str(), wdsn.c_str());
    }
    return ok;
}

HINSTANCE g_hMysql = nullptr;
typedef void (WINAPI *PSetMysqlDBGlobalConnectionString)(const wchar_t *dbConnection, bool remote);
typedef const char* (WINAPI *PSetMysqlEmbeddedOptions)(const wchar_t *options);
typedef bool(WINAPI *PDoMySQLAuthentication)(USocket_Server_Handle hSocket, const wchar_t *userId, const wchar_t *password, unsigned int nSvsId, const wchar_t *dbConnection);
static PSetMysqlDBGlobalConnectionString SetMysqlDBGlobalConnectionString = nullptr;
static PSetMysqlEmbeddedOptions SetMysqlEmbeddedOptions = nullptr;
static PDoMySQLAuthentication DoMySQLAuthentication = nullptr;

JNIEXPORT void JNICALL Java_SPA_ServerSide_Mysql_SetMysqlDBGlobalConnectionString(JNIEnv *env, jclass pClass, jstring dbConnection, jboolean remote) {
    SPA::CAutoLock al(g_co);
    if (!g_hMysql) {
#ifdef WIN32_64
        g_hMysql = ::LoadLibraryW(L"smysql.dll");
#else
        g_hMysql = ::dlopen("libsmysql.so", RTLD_LAZY);
#endif
    }
    if (g_hMysql && !SetMysqlDBGlobalConnectionString) {
        SetMysqlDBGlobalConnectionString = (PSetMysqlDBGlobalConnectionString)::GetProcAddress(g_hMysql, "SetMysqlDBGlobalConnectionString");
    }
    if (g_hMysql && SetMysqlDBGlobalConnectionString) {
        std::wstring s;
        if (dbConnection) {
            jsize len = env->GetStringLength(dbConnection);
            const jchar *p = env->GetStringChars(dbConnection, nullptr);
            s.assign(p, p + len);
            env->ReleaseStringChars(dbConnection, p);
        }
        SetMysqlDBGlobalConnectionString(s.c_str(), remote ? true : false);
    }
}

JNIEXPORT jstring JNICALL Java_SPA_ServerSide_Mysql_SetMysqlEmbeddedOptions(JNIEnv *env, jclass pClass, jstring options) {
    return env->NewStringUTF("");
}

JNIEXPORT jboolean JNICALL Java_SPA_ServerSide_Mysql_DoMySQLAuthentication(JNIEnv *env, jclass, jlong h, jstring uid, jstring pwd, jint svsId, jstring db) {
    SPA::CAutoLock al(g_co);
    if (!g_hMysql) {
#ifdef WIN32_64
        g_hMysql = ::LoadLibraryW(L"smysql.dll");
#else
        g_hMysql = ::dlopen("libsmysql.so", RTLD_LAZY);
#endif
    }
    if (g_hMysql && !DoMySQLAuthentication) {
        DoMySQLAuthentication = (PDoMySQLAuthentication)::GetProcAddress(g_hMysql, "DoMySQLAuthentication");
    }
    bool ok = false;
    if (g_hMysql && DoMySQLAuthentication) {
        jsize len;
        std::wstring wuid, wpwd, wdb;
        if (uid) {
            len = env->GetStringLength(uid);
            const jchar *puid = env->GetStringChars(uid, nullptr);
            wuid.assign(puid, puid + len);
            env->ReleaseStringChars(uid, puid);
        }
        if (pwd) {
            len = env->GetStringLength(pwd);
            const jchar *ppwd = env->GetStringChars(pwd, nullptr);
            wpwd.assign(ppwd, ppwd + len);
            env->ReleaseStringChars(pwd, ppwd);
        }
        if (db) {
            len = env->GetStringLength(db);
            const jchar *pdb = env->GetStringChars(db, nullptr);
            wdb.assign(pdb, pdb + len);
            env->ReleaseStringChars(db, pdb);
        }
        ok = DoMySQLAuthentication((USocket_Server_Handle) h, wuid.c_str(), wpwd.c_str(), (unsigned int) svsId, wdb.c_str());
    }
    return ok;
}
