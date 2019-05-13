
#include "../../../include/uclient.h"
#include "SPA_ClientSide_ClientCoreLoader.h"
#include <unordered_map>
#include <algorithm>
#include <vector>

#ifdef WINCE
#elif defined(WIN32_64)
#else
#include <unistd.h>
#endif

#define UCERT_OBJECT_ARRAY_SIZE 10
#define UMESSAGE_OBJECT_ARRAY_SIZE 2

SPA::CUCriticalSection g_coClient;
jobject g_currObj = nullptr;
JavaVM *g_vmClient = nullptr;

SPA::CUCriticalSection g_csClient;
std::unordered_map<unsigned int, jobject> g_mapPJ;

//cache CClientSocket class and its callback method ids
jmethodID g_midOnAllRequestsProcessed = nullptr;
jmethodID g_midOnBaseRequestProcessed = nullptr;
jmethodID g_midOnEnter = nullptr;
jmethodID g_midOnExit = nullptr;
jmethodID g_midOnHandShakeCompleted = nullptr;
jmethodID g_midOnRequestProcessed = nullptr;
jmethodID g_midOnSendUserMessage = nullptr;
jmethodID g_midOnSendUserMessageEx = nullptr;
jmethodID g_midOnServerException = nullptr;
jmethodID g_midOnSocketClosed = nullptr;
jmethodID g_midOnSocketConnected = nullptr;
jmethodID g_midOnSpeak = nullptr;
jmethodID g_midOnSpeakEx = nullptr;
jmethodID g_midOnPostProcessing = nullptr;

jmethodID g_midMSContructor = nullptr;
jfieldID g_fidUserId = nullptr;
jfieldID g_fidIpAddress = nullptr;
jfieldID g_fidPort = nullptr;
jfieldID g_fidSvsID = nullptr;
jfieldID g_fidSelfMessage = nullptr;

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
#else


#endif

jclass g_classCClientSocket = nullptr;
jclass g_classCMessageSender = nullptr;

void SetCaches(JNIEnv *env) {
    jclass cls = env->FindClass("SPA/ClientSide/CClientSocket");
    //make a global class reference to CClientSocket
    g_classCClientSocket = (jclass) env->NewGlobalRef(cls);
    g_midOnAllRequestsProcessed = env->GetStaticMethodID(cls, "OnAllRequestsProcessed", "(JS)V");
    g_midOnBaseRequestProcessed = env->GetStaticMethodID(cls, "OnBaseRequestProcessed", "(JS)V");
    g_midOnEnter = env->GetStaticMethodID(cls, "OnEnter", "(JLjava/lang/Object;[I)V");
    g_midOnExit = env->GetStaticMethodID(cls, "OnExit", "(JLjava/lang/Object;[I)V");
    g_midOnHandShakeCompleted = env->GetStaticMethodID(cls, "OnHandShakeCompleted", "(JI)V");
    g_midOnRequestProcessed = env->GetStaticMethodID(cls, "OnRequestProcessed", "(JSI[BBZ)V");
    g_midOnSendUserMessage = env->GetStaticMethodID(cls, "OnSendUserMessage", "(JLjava/lang/Object;[B)V");
    g_midOnSendUserMessageEx = env->GetStaticMethodID(cls, "OnSendUserMessageEx", "(JLjava/lang/Object;[B)V");
    g_midOnServerException = env->GetStaticMethodID(cls, "OnServerException", "(JSLjava/lang/String;Ljava/lang/String;I)V");
    g_midOnSocketClosed = env->GetStaticMethodID(cls, "OnSocketClosed", "(JI)V");
    g_midOnSocketConnected = env->GetStaticMethodID(cls, "OnSocketConnected", "(JI)V");
    g_midOnSpeak = env->GetStaticMethodID(cls, "OnSpeak", "(JLjava/lang/Object;[I[B)V");
    g_midOnSpeakEx = env->GetStaticMethodID(cls, "OnSpeakEx", "(JLjava/lang/Object;[I[B)V");
    g_midOnPostProcessing = env->GetStaticMethodID(cls, "OnPostProcessing", "(JIJ)V");

    cls = env->FindClass("SPA/ClientSide/CMessageSender");
    //make a global class reference to CMessageSender
    g_classCMessageSender = (jclass) env->NewGlobalRef(cls);
    g_midMSContructor = env->GetMethodID(cls, "<init>", "()V");
    g_fidUserId = env->GetFieldID(cls, "UserId", "Ljava/lang/String;");
    g_fidIpAddress = env->GetFieldID(cls, "IpAddress", "Ljava/lang/String;");
    g_fidPort = env->GetFieldID(cls, "Port", "I");
    g_fidSvsID = env->GetFieldID(cls, "SvsID", "I");
    g_fidSelfMessage = env->GetFieldID(cls, "SelfMessage", "Z");
}

void CleanException(JNIEnv *env) {
    jthrowable ex = env->ExceptionOccurred();
    if (ex) {
        env->ExceptionClear();
    }
}

void CALLBACK OnSocketClosed(USocket_Client_Handle handler, int nError) {
    JNIEnv *env;
    jint es = g_vmClient->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(env);
    env->CallStaticVoidMethod(g_classCClientSocket, g_midOnSocketClosed, (jlong) handler, (jint) nError);
    CleanException(env);
}

void CALLBACK OnHandShakeCompleted(USocket_Client_Handle handler, int nError) {
    JNIEnv *env;
    jint es = g_vmClient->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(env);
    env->CallStaticVoidMethod(g_classCClientSocket, g_midOnHandShakeCompleted, (jlong) handler, (jint) nError);
    CleanException(env);
}

void CALLBACK OnSocketConnected(USocket_Client_Handle handler, int nError) {
    JNIEnv *env;
    jint es = g_vmClient->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(env);
    env->CallStaticVoidMethod(g_classCClientSocket, g_midOnSocketConnected, (jlong) handler, (jint) nError);
    CleanException(env);
}

void CALLBACK OnRequestProcessed(USocket_Client_Handle handler, unsigned short requestId, unsigned int len) {
    JNIEnv *env;
    bool endian;
    unsigned int size = len;
    SPA::tagOperationSystem os = GetPeerOs(handler, &endian);
    const unsigned char *arr = GetResultBuffer(handler);
    if (!arr)
        len = 0;
    jint es = g_vmClient->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(env);
    jbyteArray bytes = env->NewByteArray(len);
    if (len && bytes) {
        env->SetByteArrayRegion(bytes, (jsize) 0, (jsize) len, (const jbyte*) arr);
    }
    env->CallStaticVoidMethod(g_classCClientSocket, g_midOnRequestProcessed, (jlong) handler, (jshort) requestId, (jint) size, bytes, (jbyte) os, (jboolean) endian);
    CleanException(env);
    env->DeleteLocalRef(bytes);
}

void CALLBACK OnBaseRequestProcessed(USocket_Client_Handle handler, unsigned short requestId) {
    JNIEnv *env;
    jint es = g_vmClient->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(env);
    env->CallStaticVoidMethod(g_classCClientSocket, g_midOnBaseRequestProcessed, (jlong) handler, (jshort) requestId);
    CleanException(env);
}

void CALLBACK OnPostProcessing(USocket_Client_Handle handler, unsigned int hint, SPA::UINT64 data) {
    JNIEnv *env;
    jint es = g_vmClient->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(env);
    env->CallStaticVoidMethod(g_classCClientSocket, g_midOnPostProcessing, (jlong) handler, (jint) hint, (jlong) data);
    CleanException(env);
}

jobject createCertInfo(const SPA::CertInfo *ci, jobject *arrayObject, unsigned int count) {
    JNIEnv *env;
    assert(count >= UCERT_OBJECT_ARRAY_SIZE);
    jint es = g_vmClient->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(env);
    jclass cls = env->FindClass("SPA/ClientSide/CertInfo");
    jmethodID constructor = env->GetMethodID(cls, "<init>", "()V");
    jobject obj = env->NewObject(cls, constructor);

    jfieldID fid = env->GetFieldID(cls, "Issuer", "Ljava/lang/String;");
    arrayObject[0] = env->NewStringUTF(ci->Issuer);
    env->SetObjectField(obj, fid, arrayObject[0]);

    fid = env->GetFieldID(cls, "Subject", "Ljava/lang/String;");
    arrayObject[1] = env->NewStringUTF(ci->Subject);
    env->SetObjectField(obj, fid, arrayObject[1]);

    fid = env->GetFieldID(cls, "NotBefore", "Ljava/lang/String;");
    arrayObject[2] = env->NewStringUTF(ci->NotBefore);
    env->SetObjectField(obj, fid, arrayObject[2]);

    fid = env->GetFieldID(cls, "NotAfter", "Ljava/lang/String;");
    arrayObject[3] = env->NewStringUTF(ci->NotAfter);
    env->SetObjectField(obj, fid, arrayObject[3]);

    fid = env->GetFieldID(cls, "CertPem", "Ljava/lang/String;");
    arrayObject[4] = env->NewStringUTF(ci->CertPem);
    env->SetObjectField(obj, fid, arrayObject[4]);

    fid = env->GetFieldID(cls, "SigAlg", "Ljava/lang/String;");
    arrayObject[5] = env->NewStringUTF(ci->SigAlg);
    env->SetObjectField(obj, fid, arrayObject[5]);

    fid = env->GetFieldID(cls, "SessionInfo", "Ljava/lang/String;");
    arrayObject[6] = env->NewStringUTF(ci->SessionInfo);
    env->SetObjectField(obj, fid, arrayObject[6]);

    fid = env->GetFieldID(cls, "Validity", "Z");
    env->SetBooleanField(obj, fid, ci->Validity);

    jbyteArray bytes = env->NewByteArray((jsize) ci->PKSize);
    arrayObject[7] = bytes;
    if (ci->PKSize)
        env->SetByteArrayRegion(bytes, 0, (jsize) ci->PKSize, (const jbyte*) ci->PublicKey);
    fid = env->GetFieldID(cls, "PublicKey", "[B");
    env->SetObjectField(obj, fid, bytes);

    bytes = env->NewByteArray((jsize) ci->AlgSize);
    arrayObject[8] = bytes;
    if (ci->AlgSize)
        env->SetByteArrayRegion(bytes, 0, (jsize) ci->AlgSize, (const jbyte*) ci->Algorithm);
    fid = env->GetFieldID(cls, "Algorithm", "[B");
    env->SetObjectField(obj, fid, bytes);

    bytes = env->NewByteArray((jsize) ci->SNSize);
    arrayObject[9] = bytes;
    if (ci->SNSize)
        env->SetByteArrayRegion(bytes, 0, (jsize) ci->SNSize, (const jbyte*) ci->SerialNumber);
    fid = env->GetFieldID(cls, "SerialNumber", "[B");
    env->SetObjectField(obj, fid, bytes);
    return obj;
}

jobject create(JNIEnv *env, const SPA::ClientSide::CMessageSender &sender, jobject *arrayObject, unsigned int count) {
    const jchar *str;
    assert(env);
    assert(count >= 2);
    jobject obj = env->NewObject(g_classCMessageSender, g_midMSContructor);
    str = (const jchar *) sender.UserId;
#ifdef WIN32_64
    jsize len = (jsize)::wcslen(sender.UserId);
#else
    jsize len = GetLen(str);
#endif
    arrayObject[0] = env->NewString(str, len);
    env->SetObjectField(obj, g_fidUserId, arrayObject[0]);
    arrayObject[1] = env->NewStringUTF(sender.IpAddress);
    env->SetObjectField(obj, g_fidIpAddress, arrayObject[1]);
    jint pt = (jint) sender.Port;
    env->SetIntField(obj, g_fidPort, pt);
    env->SetIntField(obj, g_fidSvsID, (jint) sender.ServiceId);
    env->SetBooleanField(obj, g_fidSelfMessage, sender.SelfMessage);
    return obj;
}

void CALLBACK OnEnter(USocket_Client_Handle handler, SPA::ClientSide::CMessageSender sender, const unsigned int *pGroup, unsigned int count) {
    JNIEnv *env;
    jint es = g_vmClient->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(env);
    jobject arrayObject[UMESSAGE_OBJECT_ARRAY_SIZE] = {nullptr};
    jobject obj = create(env, sender, arrayObject, UMESSAGE_OBJECT_ARRAY_SIZE);
    if (!pGroup)
        count = 0;
    jintArray arrInt = env->NewIntArray((jsize) count);
    if (count)
        env->SetIntArrayRegion(arrInt, 0, (jsize) count, (const jint*) pGroup);
    env->CallStaticVoidMethod(g_classCClientSocket, g_midOnEnter, (jlong) handler, obj, arrInt);
    CleanException(env);
    env->DeleteLocalRef(arrInt);
    for (int n = 0; n < UMESSAGE_OBJECT_ARRAY_SIZE; ++n) {
        if (arrayObject[n]) {
            env->DeleteLocalRef(arrayObject[n]);
        }
    }
    env->DeleteLocalRef(obj);
}

void CALLBACK OnExit(USocket_Client_Handle handler, SPA::ClientSide::CMessageSender sender, const unsigned int *pGroup, unsigned int count) {
    JNIEnv *env;
    jint es = g_vmClient->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(env);
    jobject arrayObject[UMESSAGE_OBJECT_ARRAY_SIZE] = {nullptr};
    jobject obj = create(env, sender, arrayObject, UMESSAGE_OBJECT_ARRAY_SIZE);
    if (!pGroup)
        count = 0;
    jintArray arrInt = env->NewIntArray((jsize) count);
    if (count)
        env->SetIntArrayRegion(arrInt, 0, (jsize) count, (const jint*) pGroup);
    env->CallStaticVoidMethod(g_classCClientSocket, g_midOnExit, (jlong) handler, obj, arrInt);
    CleanException(env);
    env->DeleteLocalRef(arrInt);
    for (int n = 0; n < UMESSAGE_OBJECT_ARRAY_SIZE; ++n) {
        if (arrayObject[n]) {
            env->DeleteLocalRef(arrayObject[n]);
        }
    }
    env->DeleteLocalRef(obj);
}

void CALLBACK OnSpeakEx(USocket_Client_Handle handler, SPA::ClientSide::CMessageSender sender, const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size) {
    JNIEnv *env;
    jint es = g_vmClient->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(env);
    jobject arrayObject[UMESSAGE_OBJECT_ARRAY_SIZE] = {nullptr};
    jobject obj = create(env, sender, arrayObject, UMESSAGE_OBJECT_ARRAY_SIZE);
    if (!pMessage)
        size = 0;
    jbyteArray arr = env->NewByteArray((jint) size);
    if (size)
        env->SetByteArrayRegion(arr, 0, (jsize) size, (const jbyte*) pMessage);
    if (!pGroup)
        count = 0;
    jintArray arrInt = env->NewIntArray((jsize) count);
    if (count)
        env->SetIntArrayRegion(arrInt, 0, (jsize) count, (const jint*) pGroup);
    env->CallStaticVoidMethod(g_classCClientSocket, g_midOnSpeakEx, (jlong) handler, obj, arrInt, arr);
    CleanException(env);
    env->DeleteLocalRef(arrInt);
    env->DeleteLocalRef(arr);
    for (int n = 0; n < UMESSAGE_OBJECT_ARRAY_SIZE; ++n) {
        if (arrayObject[n]) {
            env->DeleteLocalRef(arrayObject[n]);
        }
    }
    env->DeleteLocalRef(obj);
}

void CALLBACK OnSpeak(USocket_Client_Handle handler, SPA::ClientSide::CMessageSender sender, const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size) {
    JNIEnv *env;
    jint es = g_vmClient->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(env);
    jobject arrayObject[UMESSAGE_OBJECT_ARRAY_SIZE] = {nullptr};
    jobject obj = create(env, sender, arrayObject, UMESSAGE_OBJECT_ARRAY_SIZE);
    if (!pMessage)
        size = 0;
    jbyteArray arr = env->NewByteArray((jint) size);
    if (size)
        env->SetByteArrayRegion(arr, 0, (jsize) size, (const jbyte*) pMessage);
    if (!pGroup)
        count = 0;
    jintArray arrInt = env->NewIntArray((jsize) count);
    if (count)
        env->SetIntArrayRegion(arrInt, 0, (jsize) count, (const jint*) pGroup);
    env->CallStaticVoidMethod(g_classCClientSocket, g_midOnSpeak, (jlong) handler, obj, arrInt, arr);
    CleanException(env);
    env->DeleteLocalRef(arrInt);
    env->DeleteLocalRef(arr);
    for (int n = 0; n < UMESSAGE_OBJECT_ARRAY_SIZE; ++n) {
        if (arrayObject[n]) {
            env->DeleteLocalRef(arrayObject[n]);
        }
    }
    env->DeleteLocalRef(obj);
}

void CALLBACK OnSendUserMessage(USocket_Client_Handle handler, SPA::ClientSide::CMessageSender sender, const unsigned char *pMessage, unsigned int size) {
    JNIEnv *env;
    jint es = g_vmClient->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(env);
    jobject arrayObject[UMESSAGE_OBJECT_ARRAY_SIZE] = {nullptr};
    jobject obj = create(env, sender, arrayObject, UMESSAGE_OBJECT_ARRAY_SIZE);
    if (!pMessage)
        size = 0;
    jbyteArray arr = env->NewByteArray((jint) size);
    if (size)
        env->SetByteArrayRegion(arr, 0, (jsize) size, (const jbyte*) pMessage);
    env->CallStaticVoidMethod(g_classCClientSocket, g_midOnSendUserMessage, (jlong) handler, obj, arr);
    CleanException(env);
    env->DeleteLocalRef(arr);
    for (int n = 0; n < UMESSAGE_OBJECT_ARRAY_SIZE; ++n) {
        if (arrayObject[n]) {
            env->DeleteLocalRef(arrayObject[n]);
        }
    }
    env->DeleteLocalRef(obj);
}

void CALLBACK OnSendUserMessageEx(USocket_Client_Handle handler, SPA::ClientSide::CMessageSender sender, const unsigned char *pMessage, unsigned int size) {
    JNIEnv *env;
    jint es = g_vmClient->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(env);
    jobject arrayObject[UMESSAGE_OBJECT_ARRAY_SIZE] = {nullptr};
    jobject obj = create(env, sender, arrayObject, UMESSAGE_OBJECT_ARRAY_SIZE);
    if (!pMessage)
        size = 0;
    jbyteArray arr = env->NewByteArray((jint) size);
    if (size)
        env->SetByteArrayRegion(arr, 0, (jsize) size, (const jbyte*) pMessage);
    env->CallStaticVoidMethod(g_classCClientSocket, g_midOnSendUserMessageEx, (jlong) handler, obj, arr);
    CleanException(env);
    env->DeleteLocalRef(arr);
    for (int n = 0; n < UMESSAGE_OBJECT_ARRAY_SIZE; ++n) {
        if (arrayObject[n]) {
            env->DeleteLocalRef(arrayObject[n]);
        }
    }
    env->DeleteLocalRef(obj);
}

void CALLBACK OnServerException(USocket_Client_Handle handler, unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode) {
    JNIEnv *env;
    jint es = g_vmClient->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(env);
#ifdef WIN32_64
    jsize size = (jsize)::wcslen(errMessage);
#else
    jsize size = (jsize) GetLen((const jchar*) errMessage);
#endif
    jobject objMsg = env->NewString((const jchar*) errMessage, size);
    jobject objWhere = env->NewStringUTF(errWhere);
    env->CallStaticVoidMethod(g_classCClientSocket, g_midOnServerException, (jlong) handler, (jshort) requestId, objMsg, objWhere, (jint) errCode);
    CleanException(env);
    env->DeleteLocalRef(objWhere);
    env->DeleteLocalRef(objMsg);
}

void CALLBACK OnAllRequestsProcessed(USocket_Client_Handle handler, unsigned short lastRequestId) {
    JNIEnv *env;
    jint es = g_vmClient->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(env);

    env->CallStaticVoidMethod(g_classCClientSocket, g_midOnAllRequestsProcessed, (jlong) handler, (jshort) lastRequestId);

    CleanException(env);
}

void DoCallback(JNIEnv *env, jobject spc, unsigned int pid, SPA::ClientSide::tagSocketPoolEvent spe, USocket_Client_Handle h) {
    assert(env);
    jclass thisClass = env->GetObjectClass(spc);
    jmethodID invoke = env->GetMethodID(thisClass, "invoke", "(IIJ)V");
    if (invoke) {
        jint id = (jint) pid;
        jint ev = (jint) spe;
        jlong handle = (jlong) h;
        env->CallVoidMethod(spc, invoke, id, ev, handle);
        CleanException(env);
    }
}

#ifdef WIN32_64
typedef DWORD UTHREAD_ID;
#else
typedef pthread_t UTHREAD_ID;
#endif

std::vector<std::pair<unsigned int, UTHREAD_ID> > g_vDThreadClient; //locked by g_csClient

void SetPoolThreadAsDaemon(unsigned int pid) {
#ifdef WIN32_64
    UTHREAD_ID tid = ::GetCurrentThreadId();
#else
    UTHREAD_ID tid = pthread_self();
#endif
    std::pair<unsigned int, UTHREAD_ID> pair = std::make_pair(pid, tid);
    SPA::CAutoLock al(g_csClient);
    if (std::find(g_vDThreadClient.begin(), g_vDThreadClient.end(), pair) == g_vDThreadClient.end()) {
        JNIEnv *env = nullptr;
        JavaVMAttachArgs args;
        args.version = JNI_VERSION_1_6; // choose your JNI version
        args.name = (char*) "uclient_pool_thread"; // you might want to give the java thread a name
        args.group = nullptr; // you might want to assign the java thread to a ThreadGroup
        jint res = g_vmClient->AttachCurrentThreadAsDaemon((void **) &env, &args);
        assert(res == 0);
        if (res == 0)
            g_vDThreadClient.push_back(pair);
    }
}

void RemovePoolThreadAsDaemon(unsigned int pid) {
    bool removed = true;
    SPA::CAutoLock al(g_csClient);
    while (removed) {
        removed = false;
        for (auto it = g_vDThreadClient.begin(), end = g_vDThreadClient.end(); it != end; ++it) {
            if (it->first == pid) {
                jint res = g_vmClient->DetachCurrentThread();
                assert(JNI_OK == res);
                removed = true;
                g_vDThreadClient.erase(it);
                break;
            }
        }
    }
}

void CALLBACK SPC(unsigned int pid, SPA::ClientSide::tagSocketPoolEvent spe, USocket_Client_Handle h) {
    JNIEnv *env;
    jobject spc = nullptr;
    {
        SPA::CAutoLock al(g_csClient);
        if (g_mapPJ.find(pid) != g_mapPJ.cend())
            spc = g_mapPJ[pid];
    }
    jint es = g_vmClient->GetEnv((void **) &env, JNI_VERSION_1_6);
    switch (spe) {
        case SPA::ClientSide::speThreadCreated:
            SetPoolThreadAsDaemon(pid);
            es = g_vmClient->GetEnv((void **) &env, JNI_VERSION_1_6);
            if (spc)
                DoCallback(env, spc, pid, spe, h);
            assert(JNI_OK == es);
            break;
        case SPA::ClientSide::speKillingThread:
            es = g_vmClient->GetEnv((void **) &env, JNI_VERSION_1_6);
            assert(JNI_OK == es);
            if (spc)
                DoCallback(env, spc, pid, spe, h);
            RemovePoolThreadAsDaemon(pid);
            break;
        case SPA::ClientSide::speStarted:
            assert(!spc);
            assert(g_currObj);
            assert(env);
            spc = env->NewGlobalRef(g_currObj);
        {
            SPA::CAutoLock al(g_csClient);
            g_mapPJ[pid] = spc;
        }
            if (spc)
                DoCallback(env, spc, pid, spe, h);
            break;
        case SPA::ClientSide::speShutdown:
        {
            assert(env);
            assert(spc);
            if (spc)
                DoCallback(env, spc, pid, spe, h);
            SPA::CAutoLock al(g_csClient);
            g_mapPJ.erase((unsigned int) pid);
            env->DeleteGlobalRef(spc);
        }
            break;
        default:
            assert(env);
            assert(spc);
            if (spc)
                DoCallback(env, spc, pid, spe, h);
            break;
    }
}

bool CALLBACK CertCallback(bool preverified, int depth, int errorCode, const char *errMsg, SPA::CertInfo *ci) {
    JNIEnv *env;
    jint es = g_vmClient->GetEnv((void **) &env, JNI_VERSION_1_6);
    assert(env);
    jclass cls = env->FindClass("SPA/ClientSide/CClientSocket$SSL");
    jmethodID mid = env->GetStaticMethodID(cls, "Verify", "(ZIILjava/lang/String;Ljava/lang/Object;)Z");
    jobject arrayObject[UCERT_OBJECT_ARRAY_SIZE] = {nullptr};
    jobject uerrmsg = env->NewStringUTF(errMsg);
    jobject ucert = createCertInfo(ci, arrayObject, UCERT_OBJECT_ARRAY_SIZE);
    jboolean b = env->CallStaticBooleanMethod(cls, mid, (jboolean) preverified, (jint) depth, (jint) errorCode, uerrmsg, ucert);
    CleanException(env);
    for (int n = 0; n < UCERT_OBJECT_ARRAY_SIZE; ++n) {
        env->DeleteLocalRef(arrayObject[n]);
    }
    env->DeleteLocalRef(uerrmsg);
    env->DeleteLocalRef(ucert);
    return (b ? true : false);
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetCertificateVerifyCallback(JNIEnv *, jclass, jboolean enabled) {
    if (enabled)
        SetCertificateVerifyCallback(CertCallback);
    else
        SetCertificateVerifyCallback(nullptr);
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_UseUTF16(JNIEnv *env, jclass) {
#ifndef WIN32_64
    UseUTF16();
#endif
    SetCaches(env);
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetCs(JNIEnv *env, jclass, jobject cs, jlong h) {
    USocket_Client_Handle native = (USocket_Client_Handle) h;
    SetOnAllRequestsProcessed(native, OnAllRequestsProcessed);
    SetOnBaseRequestProcessed(native, OnBaseRequestProcessed);
    SetOnEnter(native, OnEnter);
    SetOnExit(native, OnExit);
    SetOnHandShakeCompleted(native, OnHandShakeCompleted);
    SetOnRequestProcessed(native, OnRequestProcessed);
    SetOnSendUserMessage(native, OnSendUserMessage);
    SetOnSendUserMessageEx(native, OnSendUserMessageEx);
    SetOnServerException(native, OnServerException);
    SetOnSocketClosed(native, OnSocketClosed);
    SetOnSocketConnected(native, OnSocketConnected);
    SetOnSpeak(native, OnSpeak);
    SetOnSpeakEx(native, OnSpeakEx);
    SetOnPostProcessing(native, OnPostProcessing);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_CreateSocketPool(JNIEnv *env, jclass spObj, jobject spc, jint maxSocketsPerThread, jint maxThreads, jboolean bAvg, jint ta) {
    unsigned int id;
    {
        SPA::CAutoLock al(g_coClient);
        if (g_vmClient == nullptr) {
            jint es = env->GetJavaVM(&g_vmClient);
            assert(JNI_OK == es);
            es = env->EnsureLocalCapacity(32);
            assert(JNI_OK == es);
        }
        g_currObj = spc;
        id = CreateSocketPool(SPC,
                (unsigned int) maxSocketsPerThread,
                (unsigned int) maxThreads,
                bAvg ? true : false,
                (SPA::tagThreadApartment)ta);
    }
    return (jint) id;
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_DestroySocketPool(JNIEnv *env, jclass spObj, jint pid) {
    bool ok = true;
    if (pid)
        ok = DestroySocketPool((unsigned int) pid);
    return ok;
}

JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_FindAClosedSocket(JNIEnv *, jclass, jint pid) {
    return (jlong) FindAClosedSocket((unsigned int) pid);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_AddOneThreadIntoPool(JNIEnv *, jclass, jint pid) {
    return AddOneThreadIntoPool((unsigned int) pid);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetLockedSockets(JNIEnv *, jclass, jint pid) {
    return (jint) GetLockedSockets((unsigned int) pid);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetIdleSockets(JNIEnv *, jclass, jint pid) {
    return (jint) GetIdleSockets((unsigned int) pid);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetConnectedSockets(JNIEnv *, jclass, jint pid) {
    return (jint) GetConnectedSockets((unsigned int) pid);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_DisconnectAll(JNIEnv *, jclass, jint pid) {
    return DisconnectAll((unsigned int) pid);
}

JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_LockASocket(JNIEnv *, jclass, jint pid, jint timeout, jlong h) {
    return (jlong) LockASocket((unsigned int) pid, (unsigned int) timeout, (USocket_Client_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_UnlockASocket(JNIEnv *, jclass, jint pid, jlong h) {
    return UnlockASocket((unsigned int) pid, (USocket_Client_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetSocketsPerThread(JNIEnv *, jclass, jint pid) {
    return (jint) GetSocketsPerThread((unsigned int) pid);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsAvg(JNIEnv *, jclass, jint pid) {
    return IsAvg((unsigned int) pid);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetDisconnectedSockets(JNIEnv *, jclass, jint pid) {
    return (jint) GetDisconnectedSockets((unsigned int) pid);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetThreadCount(JNIEnv *, jclass, jint pid) {
    return (jint) GetThreadCount((unsigned int) pid);
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_Close(JNIEnv *, jclass, jlong h) {
    Close((USocket_Client_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetCountOfRequestsQueued(JNIEnv *, jclass, jlong h) {
    return (jint) GetCountOfRequestsQueued((USocket_Client_Handle) h);
}

JNIEXPORT jshort JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetCurrentRequestID(JNIEnv *, jclass, jlong h) {
    return (jshort) GetCurrentRequestID((USocket_Client_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetCurrentResultSize(JNIEnv *, jclass, jlong h) {
    return (jint) GetCurrentResultSize((USocket_Client_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsDequeuedMessageAborted(JNIEnv *, jclass, jlong h) {
    return IsDequeuedMessageAborted((USocket_Client_Handle) h);
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_AbortDequeuedMessage(JNIEnv *, jclass, jlong h) {
    AbortDequeuedMessage((USocket_Client_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetEncryptionMethod(JNIEnv *env, jclass, jlong h) {
    return (jint) GetEncryptionMethod((USocket_Client_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetConnectionState(JNIEnv *, jclass, jlong h) {
    return (jint) GetConnectionState((USocket_Client_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetErrorCode(JNIEnv *, jclass, jlong h) {
    return GetErrorCode((USocket_Client_Handle) h);
}

JNIEXPORT jstring JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetErrorMessage(JNIEnv *env, jclass, jlong h) {
    char str[2049];
    unsigned int res = GetErrorMessage((USocket_Client_Handle) h, str, 2049);
    return env->NewStringUTF(str);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetSocketPoolId(JNIEnv *, jclass, jlong h) {
    return (jint) GetSocketPoolId((USocket_Client_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsOpened(JNIEnv *, jclass, jlong h) {
    return IsOpened((USocket_Client_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SendRequest(JNIEnv *env, jclass, jlong h, jshort reqId, jbyteArray bytes, jint len) {
    //can't use GetPrimitiveArrayCritical for thread dead-lock
    jbyte *arr = env->GetByteArrayElements(bytes, nullptr);
    if (!arr)
        len = 0;
    jboolean b = SendRequest((USocket_Client_Handle) h, (unsigned short) reqId, (const unsigned char*) arr, (unsigned int) len);
    env->ReleaseByteArrayElements(bytes, arr, JNI_ABORT);
    return b;
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetLastCallInfo(JNIEnv *env, jclass, jbyteArray buffer, jint len) {
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

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_WaitAll(JNIEnv *, jclass, jlong h, jint ms) {
    return WaitAll((USocket_Client_Handle) h, (unsigned int) ms);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_Cancel(JNIEnv *, jclass, jlong h, jint cancel) {
    return Cancel((USocket_Client_Handle) h, (unsigned int) cancel);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsRandom(JNIEnv *, jclass, jlong h) {
    return IsRandom((USocket_Client_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetBytesInSendingBuffer(JNIEnv *, jclass, jlong h) {
    return (jint) GetBytesInSendingBuffer((USocket_Client_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetBytesInReceivingBuffer(JNIEnv *, jclass, jlong h) {
    return (jint) GetBytesInReceivingBuffer((USocket_Client_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsBatching(JNIEnv *, jclass, jlong h) {
    return IsBatching((USocket_Client_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetBytesBatched(JNIEnv *, jclass, jlong h) {
    return (jint) GetBytesBatched((USocket_Client_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_StartBatching(JNIEnv *, jclass, jlong h) {
    return StartBatching((USocket_Client_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_CommitBatching(JNIEnv *, jclass, jlong h, jboolean serverBatching) {
    return CommitBatching((USocket_Client_Handle) h, serverBatching ? true : false);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_AbortBatching(JNIEnv *, jclass, jlong h) {
    return AbortBatching((USocket_Client_Handle) h);
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetUserID(JNIEnv *env, jclass, jlong h, jstring userId) {
    if (userId) {
        const jchar *uid = env->GetStringChars(userId, nullptr);
        SetUserID((USocket_Client_Handle) h, (const wchar_t*) uid);
        env->ReleaseStringChars(userId, uid);
    } else
        SetUserID((USocket_Client_Handle) h, L"");

}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetUID(JNIEnv *env, jclass, jlong h, jcharArray userId, jint chars) {
    jint res = 0;
    if (userId && chars) {
        jchar *recv = env->GetCharArrayElements(userId, nullptr);
        res = (jint) GetUID((USocket_Client_Handle) h, (wchar_t*) recv, (unsigned int) chars);
        env->ReleaseCharArrayElements(userId, recv, 0);
    }
    return res;
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetPassword(JNIEnv *env, jclass, jlong h, jstring pwd) {
    if (pwd) {
        const jchar *secret = env->GetStringChars(pwd, nullptr);
        SetPassword((USocket_Client_Handle) h, (const wchar_t*) secret);
        env->ReleaseStringChars(pwd, secret);
    } else
        SetPassword((USocket_Client_Handle) h, L"");
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SwitchTo(JNIEnv *, jclass, jlong h, jint serviceId) {
    return SwitchTo((USocket_Client_Handle) h, (unsigned int) serviceId);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_Connect(JNIEnv *env, jclass, jlong h, jbyteArray host, jint len, jint port, jboolean sync, jboolean v6) {
    if (!host)
        return false;
    jbyte *arr = env->GetByteArrayElements(host, nullptr);
    std::string s(arr, arr + len);
    bool ok = Connect((USocket_Client_Handle) h, s.c_str(), (unsigned int) port, sync ? true : false, v6 ? true : false);
    env->ReleaseByteArrayElements(host, arr, JNI_ABORT);
    return ok;
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_Enter(JNIEnv *env, jclass, jlong h, jintArray groups, jint count) {
    if (groups && count) {
        jint *grps = env->GetIntArrayElements(groups, nullptr);
        bool ok = Enter((USocket_Client_Handle) h, (const unsigned int*) grps, (unsigned int) count);
        env->ReleaseIntArrayElements(groups, grps, JNI_ABORT);
        return ok;
    }
    return Enter((USocket_Client_Handle) h, nullptr, 0);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_Speak(JNIEnv *env, jclass, jlong h, jbyteArray message, jint size, jintArray groups, jint count) {
    jbyte *msg = nullptr;
    jint *grps = nullptr;
    if (message && size)
        msg = env->GetByteArrayElements(message, nullptr);
    else
        size = 0;
    if (groups && count)
        grps = env->GetIntArrayElements(groups, nullptr);
    else
        count = 0;
    bool ok = Speak((USocket_Client_Handle) h, (const unsigned char*) msg, (unsigned int) size, (const unsigned int*) grps, (unsigned int) count);
    if (msg)
        env->ReleaseByteArrayElements(message, msg, JNI_ABORT);
    if (grps)
        env->ReleaseIntArrayElements(groups, grps, JNI_ABORT);
    return ok;
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SpeakEx(JNIEnv *env, jclass, jlong h, jbyteArray message, jint size, jintArray groups, jint count) {
    jbyte *msg = nullptr;
    jint *grps = nullptr;
    if (message && size)
        msg = env->GetByteArrayElements(message, nullptr);
    else
        size = 0;
    if (groups && count)
        grps = env->GetIntArrayElements(groups, nullptr);
    else
        count = 0;
    bool ok = SpeakEx((USocket_Client_Handle) h, (const unsigned char*) msg, (unsigned int) size, (const unsigned int*) grps, (unsigned int) count);
    if (msg)
        env->ReleaseByteArrayElements(message, msg, JNI_ABORT);
    if (grps)
        env->ReleaseIntArrayElements(groups, grps, JNI_ABORT);
    return ok;
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SendUserMessage(JNIEnv *env, jclass, jlong h, jstring userId, jbyteArray message, jint size) {
    bool ok;
    jbyte *msg = nullptr;
    if (message && size)
        msg = env->GetByteArrayElements(message, nullptr);
    else
        size = 0;
    if (userId) {
        const jchar *uid = env->GetStringChars(userId, nullptr);
        std::vector<jchar> v(uid, uid + env->GetStringLength(userId));
        jchar c = 0;
        v.push_back(c); //null-terminated
        ok = SendUserMessage((USocket_Client_Handle) h, (const wchar_t *) v.data(), (const unsigned char*) msg, (unsigned int) size);
        if (uid)
            env->ReleaseStringChars(userId, uid);
    } else
        ok = SendUserMessage((USocket_Client_Handle) h, L"", (const unsigned char*) msg, (unsigned int) size);
    if (msg)
        env->ReleaseByteArrayElements(message, msg, JNI_ABORT);
    return ok;
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SendUserMessageEx(JNIEnv *env, jclass, jlong h, jstring userId, jbyteArray message, jint size) {
    bool ok;
    jbyte *msg = nullptr;
    if (message && size)
        msg = env->GetByteArrayElements(message, nullptr);
    else
        size = 0;
    if (userId) {
        const jchar *uid = env->GetStringChars(userId, nullptr);
        std::vector<jchar> v(uid, uid + env->GetStringLength(userId));
        jchar c = 0;
        v.push_back(c); //null-terminated
        ok = SendUserMessageEx((USocket_Client_Handle) h, (const wchar_t*) v.data(), (const unsigned char*) msg, (unsigned int) size);
        if (uid)
            env->ReleaseStringChars(userId, uid);
    } else
        ok = SendUserMessageEx((USocket_Client_Handle) h, L"", (const unsigned char*) msg, (unsigned int) size);
    if (msg)
        env->ReleaseByteArrayElements(message, msg, JNI_ABORT);
    return ok;
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_StartQueue(JNIEnv *env, jclass, jlong h, jbyteArray qName, jint len, jboolean secure, jboolean dequeueShared, jint ttl) {
    std::string s;
    const char *name = "";
    if (len && qName) {
        name = (const char*) env->GetByteArrayElements(qName, nullptr);
        s.assign(name, name + len);
    } else
        len = 0;
    bool ok = StartQueue((USocket_Client_Handle) h, s.c_str(), secure ? true : false, dequeueShared ? true : false, (unsigned int) ttl);
    if (len)
        env->ReleaseByteArrayElements(qName, (jbyte*) name, JNI_ABORT);
    return ok;
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetVerifyLocation(JNIEnv *env, jclass, jbyteArray path, jint len) {
    std::string s;
    const char *p = "";
    if (len && path) {
        p = (const char*) env->GetByteArrayElements(path, nullptr);
        s.assign(p, p + len);
    } else
        len = 0;
    bool ok = SetVerifyLocation(s.c_str());
    if (len)
        env->ReleaseByteArrayElements(path, (jbyte*) p, JNI_ABORT);
    return ok;
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetClientWorkDirectory(JNIEnv *env, jclass, jbyteArray dir, jint len) {
    std::string s;
    const char *d = "";
    if (len && dir) {
        d = (const char*) env->GetByteArrayElements(dir, nullptr);
        s.assign(d, d + len);
    } else
        len = 0;
    SetClientWorkDirectory(s.c_str());
    if (len)
        env->ReleaseByteArrayElements(dir, (jbyte*) d, JNI_ABORT);
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_Exit(JNIEnv *, jclass, jlong h) {
    Exit((USocket_Client_Handle) h);
}

JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetBytesReceived(JNIEnv *, jclass, jlong h) {
    return (jlong) GetBytesReceived((USocket_Client_Handle) h);
}

JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetBytesSent(JNIEnv *, jclass, jlong h) {
    return (jlong) GetBytesSent((USocket_Client_Handle) h);
}

JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetSocketNativeHandle(JNIEnv *, jclass, jlong h) {
    return (jlong) GetSocketNativeHandle((USocket_Client_Handle) h);
}

JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetMessageCount(JNIEnv *, jclass, jlong h) {
    return (jlong) GetMessageCount((USocket_Client_Handle) h);
}

JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetQueueSize(JNIEnv *, jclass, jlong h) {
    return (jlong) GetQueueSize((USocket_Client_Handle) h);
}

JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetQueueLastIndex(JNIEnv *, jclass, jlong h) {
    return (jlong) GetQueueLastIndex((USocket_Client_Handle) h);
}

JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_CancelQueuedRequestsByIndex(JNIEnv *, jclass, jlong h, jlong start, jlong end) {
    return (jlong) CancelQueuedRequestsByIndex((USocket_Client_Handle) h, (SPA::UINT64)start, (SPA::UINT64)end);
}

JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_RemoveQueuedRequestsByTTL(JNIEnv *, jclass, jlong h) {
    return (jlong) RemoveQueuedRequestsByTTL((USocket_Client_Handle) h);
}

JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetLastQueueMessageTime(JNIEnv *, jclass, jlong h) {
    return (jlong) GetLastQueueMessageTime((USocket_Client_Handle) h);
}

JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetJobSize(JNIEnv *, jclass, jlong h) {
    return (jlong) GetJobSize((USocket_Client_Handle) h);
}

JNIEXPORT jbyte JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetPeerOs(JNIEnv *env, jclass, jlong h, jbooleanArray endian, jint len) {
    bool b;
    SPA::tagOperationSystem os = GetPeerOs((USocket_Client_Handle) h, &b);
    if (endian && len) {
        jboolean *e = env->GetBooleanArrayElements(endian, nullptr);
        e[0] = b;
        env->ReleaseBooleanArrayElements(endian, e, 0);
    }
    return (jbyte) os;
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetZip(JNIEnv *, jclass, jlong h, jboolean zip) {
    SetZip((USocket_Client_Handle) h, zip ? true : false);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetZip(JNIEnv *, jclass, jlong h) {
    return GetZip((USocket_Client_Handle) h);
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetZipLevel(JNIEnv *, jclass, jlong h, jint zipLevel) {
    SetZipLevel((USocket_Client_Handle) h, (SPA::tagZipLevel)zipLevel);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetZipLevel(JNIEnv *, jclass, jlong h) {
    return (jint) GetZipLevel((USocket_Client_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetCurrentServiceId(JNIEnv *, jclass, jlong h) {
    return (jint) GetCurrentServiceId((USocket_Client_Handle) h);
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_StopQueue(JNIEnv *, jclass, jlong h, jboolean del) {
    StopQueue((USocket_Client_Handle) h, del ? true : false);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_DequeuedResult(JNIEnv *, jclass, jlong h) {
    return DequeuedResult((USocket_Client_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetMessagesInDequeuing(JNIEnv *, jclass, jlong h) {
    return (jint) GetMessagesInDequeuing((USocket_Client_Handle) h);
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_PostProcessing(JNIEnv *, jclass, jlong h, jint hint, jlong data) {
    PostProcessing((USocket_Client_Handle) h, (unsigned int) hint, (SPA::UINT64)data);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsQueueSecured(JNIEnv *, jclass, jlong h) {
    return IsQueueSecured((USocket_Client_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsQueueStarted(JNIEnv *, jclass, jlong h) {
    return IsQueueStarted((USocket_Client_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_DoEcho(JNIEnv *, jclass, jlong h) {
    return DoEcho((USocket_Client_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetSockOpt(JNIEnv *, jclass, jlong h, jint optName, jint optValue, jint level) {
    return SetSockOpt((USocket_Client_Handle) h, (SPA::tagSocketOption)optName, (int) optValue, (SPA::tagSocketLevel)level);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetSockOptAtSvr(JNIEnv *, jclass, jlong h, jint optName, jint optValue, jint level) {
    return SetSockOptAtSvr((USocket_Client_Handle) h, (SPA::tagSocketOption)optName, (int) optValue, (SPA::tagSocketLevel)level);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_TurnOnZipAtSvr(JNIEnv *, jclass, jlong h, jboolean zip) {
    return TurnOnZipAtSvr((USocket_Client_Handle) h, zip ? true : false);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetZipLevelAtSvr(JNIEnv *, jclass, jlong h, jint zipLevel) {
    return SetZipLevelAtSvr((USocket_Client_Handle) h, (SPA::tagZipLevel)zipLevel);
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetRecvTimeout(JNIEnv *, jclass, jlong h, jint timeout) {
    SetRecvTimeout((USocket_Client_Handle) h, (unsigned int) timeout);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetRecvTimeout(JNIEnv *, jclass, jlong h) {
    return (jint) GetRecvTimeout((USocket_Client_Handle) h);
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetConnTimeout(JNIEnv *, jclass, jlong h, jint timeout) {
    SetConnTimeout((USocket_Client_Handle) h, (unsigned int) timeout);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetConnTimeout(JNIEnv *, jclass, jlong h) {
    return (jint) GetConnTimeout((USocket_Client_Handle) h);
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetAutoConn(JNIEnv *, jclass, jlong h, jboolean autoConn) {
    SetAutoConn((USocket_Client_Handle) h, autoConn ? true : false);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetOptimistic(JNIEnv *, jclass, jlong h) {
    return (jint) GetOptimistic((USocket_Client_Handle) h);
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetOptimistic(JNIEnv *, jclass, jlong h, jint optimistic) {
    SetOptimistic((USocket_Client_Handle) h, (SPA::tagOptimistic) optimistic);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetAutoConn(JNIEnv *, jclass, jlong h) {
    return GetAutoConn((USocket_Client_Handle) h);
}

JNIEXPORT jshort JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetServerPingTime(JNIEnv *, jclass, jlong h) {
    return (jshort) GetServerPingTime((USocket_Client_Handle) h);
}

JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetSSL(JNIEnv *, jclass, jlong h) {
    return (jlong) GetSSL((USocket_Client_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IgnoreLastRequest(JNIEnv *env, jclass, jlong h, jshort reqId) {
    return IgnoreLastRequest((USocket_Client_Handle) h, (unsigned short) reqId);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsDequeueEnabled(JNIEnv *, jclass, jlong h) {
    return IsDequeueEnabled((USocket_Client_Handle) h);
}

JNIEXPORT jstring JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetQueueName(JNIEnv *env, jclass, jlong h) {
    const char *qName = GetQueueName((USocket_Client_Handle) h);
    if (qName)
        return env->NewStringUTF(qName);
    return env->NewStringUTF("");
}

JNIEXPORT jstring JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetQueueFileName(JNIEnv *env, jclass, jlong h) {
    const char *qName = GetQueueFileName((USocket_Client_Handle) h);
    if (qName)
        return env->NewStringUTF(qName);
    return env->NewStringUTF("");
}

JNIEXPORT jstring JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetPeerName(JNIEnv *env, jclass, jlong h, jintArray peerPort, jint count) {
    unsigned int port;
    char name[256] = {0};
    if (!GetPeerName((USocket_Client_Handle) h, &port, name, sizeof (name)))
        return env->NewStringUTF("");
    size_t len = ::strlen(name);
    if (peerPort && count) {
        jint *p = env->GetIntArrayElements(peerPort, nullptr);
        *p = (jint) port;
        env->ReleaseIntArrayElements(peerPort, p, 0);
    }
    return env->NewStringUTF(name);
}

JNIEXPORT jobject JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetUCert(JNIEnv *env, jclass, jlong h) {
    jobject arrayObject[UCERT_OBJECT_ARRAY_SIZE] = {nullptr};
    SPA::CertInfo *ci = GetUCert((USocket_Client_Handle) h);
    jobject ucert = createCertInfo(ci, arrayObject, UCERT_OBJECT_ARRAY_SIZE);
    return ucert;
}

JNIEXPORT jstring JNICALL Java_SPA_ClientSide_ClientCoreLoader_Verify(JNIEnv *env, jclass, jlong h, jintArray code, jint count) {
    jstring s;
    int errCode = 0;
    const char *str = Verify((USocket_Client_Handle) h, &errCode);
    if (str)
        s = env->NewStringUTF(str);
    else
        s = env->NewStringUTF("");
    if (code && count) {
        jint *p = env->GetIntArrayElements(code, nullptr);
        p[0] = errCode;
        env->ReleaseIntArrayElements(code, p, 0);
    }
    return s;
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_AbortJob(JNIEnv *, jclass, jlong h) {
    return AbortJob((USocket_Client_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_StartJob(JNIEnv *, jclass, jlong h) {
    return StartJob((USocket_Client_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_EndJob(JNIEnv *, jclass, jlong h) {
    return EndJob((USocket_Client_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsRouteeRequest(JNIEnv *, jclass, jlong h) {
    return IsRouteeRequest((USocket_Client_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetRouteeCount(JNIEnv *, jclass, jlong h) {
    return (jint) GetRouteeCount((USocket_Client_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SendRouteeResult(JNIEnv *env, jclass, jlong h, jshort reqId, jbyteArray bytes, jint len) {
    jbyte *arr = env->GetByteArrayElements(bytes, nullptr);
    if (!arr)
        len = 0;
    jboolean b = SendRouteeResult((USocket_Client_Handle) h, (unsigned short) reqId, (const unsigned char*) arr, (unsigned int) len);
    env->ReleaseByteArrayElements(bytes, arr, JNI_ABORT);
    return b;
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsDequeueShared(JNIEnv *, jclass, jlong h) {
    return IsDequeueShared((USocket_Client_Handle) h);
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetClientQueueStatus(JNIEnv *, jclass, jlong h) {
    return (jint) GetClientQueueStatus((USocket_Client_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_PushQueueTo(JNIEnv *env, jclass, jlong h, jlongArray handles, jint count) {
    if (handles && count) {
        jlong *p = env->GetLongArrayElements(handles, nullptr);
        USocket_Client_Handle *pSockets = new USocket_Client_Handle[count];
        for (jint n = 0; n < count; ++n) {
            pSockets[n] = reinterpret_cast<USocket_Client_Handle> (p[n]);
        }
        bool ok = PushQueueTo((USocket_Client_Handle) h, pSockets, (unsigned int) count);
        env->ReleaseLongArrayElements(handles, p, JNI_ABORT);
        delete []pSockets;
        return ok;
    }
    return true;
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetTTL(JNIEnv *, jclass, jlong h) {
    return (jint) GetTTL((USocket_Client_Handle) h);
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_ResetQueue(JNIEnv *, jclass, jlong h) {
    ResetQueue((USocket_Client_Handle) h);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsClientQueueIndexPossiblyCrashed(JNIEnv *, jclass) {
    return IsClientQueueIndexPossiblyCrashed();
}

JNIEXPORT jstring JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetClientWorkDirectory(JNIEnv *env, jclass) {
    const char *path = GetClientWorkDirectory();
    if (path)
        return env->NewStringUTF(path);
    return env->NewStringUTF("");
}

JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetNumberOfSocketPools(JNIEnv *env, jclass obj) {
    return (jint) GetNumberOfSocketPools();
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsRouting(JNIEnv *, jclass, jlong h) {
    return IsRouting((USocket_Client_Handle) h);
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetEncryptionMethod(JNIEnv *, jclass, jlong h, jint em) {
    SetEncryptionMethod((USocket_Client_Handle) h, (SPA::tagEncryptionMethod)em);
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_Shutdown(JNIEnv *, jclass, jlong h, jint how) {
    Shutdown((USocket_Client_Handle) h, (SPA::tagShutdownType)how);
}

JNIEXPORT jstring JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetUClientSocketVersion(JNIEnv *env, jclass) {
    const char *v = GetUClientSocketVersion();
    if (v)
        return env->NewStringUTF(v);
    return env->NewStringUTF("");
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetMessageQueuePassword(JNIEnv *env, jclass, jbyteArray pwd, jint chars) {
    if (pwd && chars) {
        jbyte *p = env->GetByteArrayElements(pwd, nullptr);
        const char *start = (const char*) p;
        std::string s(start, start + chars);
        SetMessageQueuePassword(s.c_str());
        env->ReleaseByteArrayElements(pwd, p, JNI_ABORT);
    } else
        SetMessageQueuePassword("");
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_EnableRoutingQueueIndex(JNIEnv *, jclass, jlong h, jboolean b) {
    EnableRoutingQueueIndex((USocket_Client_Handle) h, b ? true : false);
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsRoutingQueueIndexEnabled(JNIEnv *, jclass, jlong h) {
    return IsRoutingQueueIndexEnabled((USocket_Client_Handle) h);
}

JNIEXPORT jstring JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetUClientAppName(JNIEnv *env, jclass) {
    const char *appname = GetUClientAppName();
    if (appname)
        return env->NewStringUTF(appname);
    return env->NewStringUTF("");
}

JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetQueueAutoMergeByPool(JNIEnv *, jclass, jint poolId) {
    return GetQueueAutoMergeByPool((unsigned int) poolId);
}

JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetQueueAutoMergeByPool(JNIEnv *, jclass, jint poolId, jboolean merge) {
    return SetQueueAutoMergeByPool((unsigned int) poolId, merge ? true : false);
}
