/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class SPA_ClientSide_ClientCoreLoader */

#ifndef _Included_SPA_ClientSide_ClientCoreLoader
#define _Included_SPA_ClientSide_ClientCoreLoader
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SetCertificateVerifyCallback
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetCertificateVerifyCallback
  (JNIEnv *, jclass, jboolean);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    CreateSocketPool
 * Signature: (Ljava/lang/Object;IIZI)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_CreateSocketPool
  (JNIEnv *, jclass, jobject, jint, jint, jboolean, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SetCs
 * Signature: (Ljava/lang/Object;J)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetCs
  (JNIEnv *, jclass, jobject, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    UseUTF16
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_UseUTF16
  (JNIEnv *, jclass);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    DestroySocketPool
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_DestroySocketPool
  (JNIEnv *, jclass, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    FindAClosedSocket
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_FindAClosedSocket
  (JNIEnv *, jclass, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    AddOneThreadIntoPool
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_AddOneThreadIntoPool
  (JNIEnv *, jclass, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetLockedSockets
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetLockedSockets
  (JNIEnv *, jclass, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetIdleSockets
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetIdleSockets
  (JNIEnv *, jclass, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetConnectedSockets
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetConnectedSockets
  (JNIEnv *, jclass, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    DisconnectAll
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_DisconnectAll
  (JNIEnv *, jclass, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    LockASocket
 * Signature: (IIJ)J
 */
JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_LockASocket
  (JNIEnv *, jclass, jint, jint, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    UnlockASocket
 * Signature: (IJ)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_UnlockASocket
  (JNIEnv *, jclass, jint, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetSocketsPerThread
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetSocketsPerThread
  (JNIEnv *, jclass, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    IsAvg
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsAvg
  (JNIEnv *, jclass, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetDisconnectedSockets
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetDisconnectedSockets
  (JNIEnv *, jclass, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetThreadCount
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetThreadCount
  (JNIEnv *, jclass, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    Close
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_Close
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetCountOfRequestsQueued
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetCountOfRequestsQueued
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetCurrentRequestID
 * Signature: (J)S
 */
JNIEXPORT jshort JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetCurrentRequestID
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetCurrentResultSize
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetCurrentResultSize
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    IsDequeuedMessageAborted
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsDequeuedMessageAborted
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    AbortDequeuedMessage
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_AbortDequeuedMessage
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetEncryptionMethod
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetEncryptionMethod
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetConnectionState
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetConnectionState
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetErrorCode
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetErrorCode
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetErrorMessage
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetErrorMessage
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetSocketPoolId
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetSocketPoolId
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    IsOpened
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsOpened
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SendRequest
 * Signature: (JS[BI)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SendRequest
  (JNIEnv *, jclass, jlong, jshort, jobject, jint, jint);


JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_RetrieveBuffer
  (JNIEnv *, jclass, jlong, jbyteArray, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    WaitAll
 * Signature: (JI)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_WaitAll
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    Cancel
 * Signature: (JI)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_Cancel
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    IsRandom
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsRandom
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetBytesInSendingBuffer
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetBytesInSendingBuffer
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetBytesInReceivingBuffer
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetBytesInReceivingBuffer
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    IsBatching
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsBatching
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetBytesBatched
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetBytesBatched
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    StartBatching
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_StartBatching
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    CommitBatching
 * Signature: (JZ)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_CommitBatching
  (JNIEnv *, jclass, jlong, jboolean);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    AbortBatching
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_AbortBatching
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SetUserID
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetUserID
  (JNIEnv *, jclass, jlong, jstring);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetUID
 * Signature: (J[CI)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetUID
  (JNIEnv *, jclass, jlong, jcharArray, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SetPassword
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetPassword
  (JNIEnv *, jclass, jlong, jstring);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SwitchTo
 * Signature: (JI)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SwitchTo
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    Connect
 * Signature: (J[BIIZZ)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_Connect
  (JNIEnv *, jclass, jlong, jbyteArray, jint, jint, jboolean, jboolean);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    Enter
 * Signature: (J[II)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_Enter
  (JNIEnv *, jclass, jlong, jintArray, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    Speak
 * Signature: (J[BI[II)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_Speak
  (JNIEnv *, jclass, jlong, jbyteArray, jint, jintArray, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SpeakEx
 * Signature: (J[BI[II)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SpeakEx
  (JNIEnv *, jclass, jlong, jbyteArray, jint, jintArray, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SendUserMessage
 * Signature: (JLjava/lang/String;[BI)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SendUserMessage
  (JNIEnv *, jclass, jlong, jstring, jbyteArray, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SendUserMessageEx
 * Signature: (JLjava/lang/String;[BI)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SendUserMessageEx
  (JNIEnv *, jclass, jlong, jstring, jbyteArray, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    StartQueue
 * Signature: (J[BIZZI)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_StartQueue
  (JNIEnv *, jclass, jlong, jbyteArray, jint, jboolean, jboolean, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SetVerifyLocation
 * Signature: ([BI)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetVerifyLocation
  (JNIEnv *, jclass, jbyteArray, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SetClientWorkDirectory
 * Signature: ([BI)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetClientWorkDirectory
  (JNIEnv *, jclass, jbyteArray, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    Exit
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_Exit
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetBytesReceived
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetBytesReceived
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetBytesSent
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetBytesSent
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetSocketNativeHandle
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetSocketNativeHandle
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetMessageCount
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetMessageCount
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetQueueSize
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetQueueSize
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetQueueLastIndex
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetQueueLastIndex
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    CancelQueuedRequestsByIndex
 * Signature: (JJJ)J
 */
JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_CancelQueuedRequestsByIndex
  (JNIEnv *, jclass, jlong, jlong, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    RemoveQueuedRequestsByTTL
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_RemoveQueuedRequestsByTTL
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetLastQueueMessageTime
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetLastQueueMessageTime
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetJobSize
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetJobSize
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetPeerOs
 * Signature: (J[ZI)B
 */
JNIEXPORT jbyte JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetPeerOs
  (JNIEnv *, jclass, jlong, jbooleanArray, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SetZip
 * Signature: (JZ)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetZip
  (JNIEnv *, jclass, jlong, jboolean);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetZip
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetZip
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SetZipLevel
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetZipLevel
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetZipLevel
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetZipLevel
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetCurrentServiceId
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetCurrentServiceId
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    StopQueue
 * Signature: (JZ)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_StopQueue
  (JNIEnv *, jclass, jlong, jboolean);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    DequeuedResult
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_DequeuedResult
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetMessagesInDequeuing
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetMessagesInDequeuing
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    IsQueueSecured
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsQueueSecured
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    IsQueueStarted
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsQueueStarted
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    DoEcho
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_DoEcho
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SetSockOpt
 * Signature: (JIII)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetSockOpt
  (JNIEnv *, jclass, jlong, jint, jint, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SetSockOptAtSvr
 * Signature: (JIII)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetSockOptAtSvr
  (JNIEnv *, jclass, jlong, jint, jint, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    TurnOnZipAtSvr
 * Signature: (JZ)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_TurnOnZipAtSvr
  (JNIEnv *, jclass, jlong, jboolean);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SetZipLevelAtSvr
 * Signature: (JI)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetZipLevelAtSvr
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SetRecvTimeout
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetRecvTimeout
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetRecvTimeout
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetRecvTimeout
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SetConnTimeout
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetConnTimeout
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetConnTimeout
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetConnTimeout
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SetAutoConn
 * Signature: (JZ)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetAutoConn
  (JNIEnv *, jclass, jlong, jboolean);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetAutoConn
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetAutoConn
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetServerPingTime
 * Signature: (J)S
 */
JNIEXPORT jshort JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetServerPingTime
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetSSL
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetSSL
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    IgnoreLastRequest
 * Signature: (JS)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IgnoreLastRequest
  (JNIEnv *, jclass, jlong, jshort);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    IsDequeueEnabled
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsDequeueEnabled
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetQueueName
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetQueueName
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetQueueFileName
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetQueueFileName
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetPeerName
 * Signature: (J[II)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetPeerName
  (JNIEnv *, jclass, jlong, jintArray, jint);


JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetBufferForCurrentThread
(JNIEnv *, jclass, jobject, jint);


/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetUCert
 * Signature: (J)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetUCert
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    Verify
 * Signature: (J[II)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_SPA_ClientSide_ClientCoreLoader_Verify
  (JNIEnv *, jclass, jlong, jintArray, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    AbortJob
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_AbortJob
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    StartJob
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_StartJob
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    EndJob
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_EndJob
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    IsRouteeRequest
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsRouteeRequest
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetRouteeCount
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetRouteeCount
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SendRouteeResult
 * Signature: (JS[BI)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SendRouteeResult
  (JNIEnv *, jclass, jlong, jshort, jobject, jint, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    IsDequeueShared
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsDequeueShared
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetClientQueueStatus
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetClientQueueStatus
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    PushQueueTo
 * Signature: (J[JI)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_PushQueueTo
  (JNIEnv *, jclass, jlong, jlongArray, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetTTL
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetTTL
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    ResetQueue
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_ResetQueue
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    IsClientQueueIndexPossiblyCrashed
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsClientQueueIndexPossiblyCrashed
  (JNIEnv *, jclass);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetClientWorkDirectory
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetClientWorkDirectory
  (JNIEnv *, jclass);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetNumberOfSocketPools
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetNumberOfSocketPools
  (JNIEnv *, jclass);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    IsRouting
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsRouting
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SetEncryptionMethod
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetEncryptionMethod
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    Shutdown
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_Shutdown
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetUClientSocketVersion
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetUClientSocketVersion
  (JNIEnv *, jclass);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SetMessageQueuePassword
 * Signature: ([BI)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetMessageQueuePassword
  (JNIEnv *, jclass, jbyteArray, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    EnableRoutingQueueIndex
 * Signature: (JZ)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_EnableRoutingQueueIndex
  (JNIEnv *, jclass, jlong, jboolean);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    IsRoutingQueueIndexEnabled
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_IsRoutingQueueIndexEnabled
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetUClientAppName
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetUClientAppName
  (JNIEnv *, jclass);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetOptimistic
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetOptimistic
  (JNIEnv *, jclass, jlong);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SetOptimistic
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetOptimistic
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SetLastCallInfo
 * Signature: ([BI)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetLastCallInfo
  (JNIEnv *, jclass, jbyteArray, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    GetQueueAutoMergeByPool
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_GetQueueAutoMergeByPool
  (JNIEnv *, jclass, jint);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SetQueueAutoMergeByPool
 * Signature: (IZ)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_SetQueueAutoMergeByPool
  (JNIEnv *, jclass, jint, jboolean);

/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    PostProcessing
 * Signature: (JIJ)V
 */
JNIEXPORT void JNICALL Java_SPA_ClientSide_ClientCoreLoader_PostProcessing
  (JNIEnv *, jclass, jlong, jint, jlong);


/*
 * Class:     SPA_ClientSide_ClientCoreLoader
 * Method:    SendInterruptRequest
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_SPA_ClientSide_ClientCoreLoader_SendInterruptRequest
(JNIEnv *, jclass, jlong, jlong);

#ifdef __cplusplus
}
#endif
#endif
