
#pragma once

#include <vector>
#include <string>
#include <deque>
#include "../include/membuffer.h"
#include "blowfish.h" 
#include <stdio.h>

#ifndef WINCE
#include <mutex>
#include <thread>
#include <condition_variable>
#include <memory>
#include <map>
#else
#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/shared_ptr.hpp>
#endif

#include <boost/unordered_map.hpp> //faster
#include "../shared/streamhead.h"
#include "../../include/ucomm.h"

#define INVALID_QUEUE_HANDLE (~0)

namespace MQ_FILE {

#pragma pack(push, 1)

    struct QAttr {
        static const SPA::UINT64 RANGE_DEQUEUED_END = 0x4000000000000000;
        static const SPA::UINT64 RANGE_DEQUEUED_START = 0x8000000000000000;
        static const SPA::UINT64 RANGE_DEQUEUED_POSITION_START = RANGE_DEQUEUED_END;
        static const SPA::UINT64 RANGE_DEQUEUED_POSITION_END = RANGE_DEQUEUED_START;

        QAttr(SPA::UINT64 pos, SPA::UINT64 index) : MessagePos(pos), MessageIndex(index) {
        }

        QAttr() : MessagePos(INVALID_NUMBER), MessageIndex(INVALID_NUMBER) {
        }

        inline bool operator==(const QAttr &qa) const {
            return (qa.MessagePos == MessagePos && qa.MessageIndex == MessageIndex);
        }

        inline bool IsValid() const {
            return ((MessagePos < RANGE_DEQUEUED_END) && (MessageIndex < RANGE_DEQUEUED_END));
        }

        SPA::UINT64 MessagePos;
        SPA::UINT64 MessageIndex;
    };

    union QueueSha1 {
        unsigned char Sha1[20];

        struct {
            unsigned int Header;
            SPA::UINT64 Mid;
            SPA::UINT64 End;
        } qs;
    };

    struct CQueueInitialInfo {
        static const SPA::UINT64 QUEUE_SHARED_INDEX = 0x8000000000000000;

        CQueueInitialInfo() {
            ::memset(this, 0, sizeof (CQueueInitialInfo));
        }
        QueueSha1 Qs;
        SPA::UINT64 MinIndex;
        unsigned char CrashSafe;
        bool Secure;
    };

    struct CDequeueConfirmInfo {

        CDequeueConfirmInfo(unsigned int handle, const QAttr &qa, bool fail, unsigned short reqId)
        : Handle(handle), QA(qa), Fail(fail), RequestId(reqId) {
        }

        CDequeueConfirmInfo(const QAttr &qa, bool fail, unsigned short reqId)
        : Handle(0), QA(qa), Fail(fail), RequestId(reqId) {
        }

        CDequeueConfirmInfo()
        : Handle(0), QA(0, 0), Fail(false), RequestId(0) {
        }

        unsigned int Handle;
        QAttr QA;
        bool Fail;
        unsigned short RequestId;
    };

    struct CBatchMessage {
        unsigned short MessageId;
        unsigned int Bytes;
        unsigned char *Buffer;
    };

#pragma pack(pop)

    struct qs_hash : public std::unary_function<QueueSha1, size_t> {

        inline size_t operator()(const QueueSha1 & qs) const {
            if (sizeof (unsigned int) == sizeof (size_t))
                return qs.qs.Header;
            return (size_t) (qs.qs.Mid + qs.qs.Header);
        }
    };

    struct qs_equal : public std::binary_function<QueueSha1, QueueSha1, bool> {

        inline bool operator()(const QueueSha1 &qs1, const QueueSha1 & qs2) const {
            return (::memcmp(&qs1, &qs2, sizeof (QueueSha1)) == 0);
        }
    };

#ifndef WINCE
	using thread = std::thread;
	using mutex = std::mutex;
	using condition_variable = std::condition_variable;
	using ms = std::chrono::milliseconds;
	typedef std::unique_lock<std::mutex> CAutoLock;
	typedef std::shared_ptr<CBlowFish> CBlowFishPtr;

#else
	using thread = boost::thread;
	using mutex = boost::mutex;
	using condition_variable = boost::condition_variable;
	typedef boost::mutex::scoped_lock CAutoLock;
	typedef boost::shared_ptr<CBlowFish> CBlowFishPtr;
#endif

    class CMqFile {
#ifndef NDEBUG
        int m_nJobBalanceDequeue;
        int m_nJobBalanceEnqueue;
        int m_nJobBalanceConfirm;
        SPA::CScopeUQueue m_sbConfirmed;
        bool FindConfirmed(const QAttr &qa);
        bool RemoveConfirmed(const QAttr &qa);
#endif
    public:
        static const unsigned int PAD_EMPTY_SIZE = 8;

        struct CBadHandler {
            CBadHandler();
        };
        static CBadHandler m_bh;

    private:
#pragma pack(push,1)
        struct MessageDecriptionHeader {
            SPA::UINT64 MessageIndex;
            unsigned int Time;
            unsigned int Len;
        };

        struct MessageDecriptionHeaderEx : public MessageDecriptionHeader, public SPA::CStreamHeader {
        };

#pragma pack(pop)
    public:
        static const SPA::INT64 FILE_SIZE_TRUNCATED = 512 * 1024;
        static const unsigned int SET_LAST_DEQUEUED_INTERVAL = 512;

    public:
        CMqFile(const char *fileName, unsigned int ttl, SPA::tagOptimistic crashSafe, bool secure, bool client = false, bool shared = false);
        //CMqFile(CMqFile && mq);

        virtual ~CMqFile();

        typedef CMqFile *PMqFile;

    public:
        //check if message queue file is available
        bool IsAvailable();

        bool IsSecure() const;

        void Reset();

        void Notify();

        bool IsDequeueOk();

        void EnableDequeue(bool enable);

        const std::string& GetRawName() const;

        const std::string& GetMQFileName() const;

        unsigned int GetLastTime();

        //compute the count of messages during dequeuing
        unsigned int GetMessagesInDequeuing();
        bool JustOne(unsigned int adding = 1);

        const QAttr* GetMessageAttributesInDequeuing();

        void ReleaseMessageAttributesInDequeuing();

        unsigned int PeekRequests(SPA::CQueuedRequestInfo *qri, unsigned int count);

        void StopQueue(SPA::tagQueueStatus = SPA::qsNormal);

        SPA::tagQueueStatus GetQueueOpenStatus() const;

        //return MQ file size in byte
        SPA::UINT64 GetMQSize();

        //compute the number of messages remaining in queue. This call is slow, and don't use the method if required only.
        SPA::UINT64 GetMessageCount();

        //return a non-zero unique message index after en-queuing a message
        virtual SPA::UINT64 Enqueue(const SPA::CStreamHeader &sh, const unsigned char *buffer, unsigned int size);

        //return a position and non-zero unique message index
        virtual SPA::UINT64 Dequeue(SPA::CUQueue &q, SPA::UINT64 &mqIndex, unsigned int waitTime);

        virtual std::vector<unsigned int> DoBatchDequeue(SPA::CUQueue &qAttr, SPA::CUQueue &qRequests, unsigned int maxSizeInByte, unsigned int waitTime);
        virtual std::vector<unsigned int> DoBatchDequeue(unsigned int requestCount, SPA::CUQueue &qAttr, SPA::CUQueue &qRequests, unsigned int waitTime);

        SPA::UINT64 BatchEnqueue(unsigned int count, const unsigned char *messages);
        std::vector<SPA::UINT64> BatchEnqueue(const SPA::CUQueue &qRequests);
        SPA::UINT64 Enqueue(const SPA::CStreamHeader &sh, SPA::CUQueue &buffer);

        //confirm the message is either dequeued successfully or failed.
        bool ConfirmDequeue(SPA::UINT64 pos, SPA::UINT64 index, bool fail = false);
        bool ConfirmDequeue(const QAttr *qa, size_t count, bool fail = false);
        unsigned int DoConfirmDequeue(const SPA::CUQueue &qDCI);
        unsigned int ConfirmDequeueJob(const QAttr *qa, size_t count, bool fail = false);
        unsigned int DoConfirmDequeue(const QAttr *qa, size_t count);

        unsigned int CancelQueuedRequests(const unsigned short *ids, unsigned int count);
        SPA::UINT64 CancelQueuedRequests(SPA::UINT64 startIndex, SPA::UINT64 endIndex);
        SPA::UINT64 CancelQueuedRequests(unsigned int startTime, unsigned int endTime);

        SPA::UINT64 GetJobSize();

        SPA::UINT64 StartJob();
        SPA::UINT64 EndJob();
        bool AbortJob();
        unsigned int GetQueueTransSize();
        CQueueInitialInfo GetMQInitInfo() const;
        SPA::UINT64 Append(CMqFile &mqFile);
        SPA::UINT64 GetLastMessageIndex();
        bool IsDequeueShared() const;
        bool AppendTo(CMqFile **qFiles, unsigned int count);
        unsigned int GetTTL() const;
        SPA::UINT64 RemoveByTTL();
        SPA::tagOptimistic IsOptimistic();
        void SetOptimistic(SPA::tagOptimistic optimistic);

    protected:
        virtual SPA::CUQueue* DoEncryption(const unsigned char *buffer, unsigned int len);
        virtual std::vector<unsigned int> DoBatchDequeueInternal(CAutoLock &al, SPA::CUQueue &qAttr, SPA::CUQueue &qRequests, unsigned int maxSizeInByte, unsigned int waitTime);
        virtual SPA::UINT64 EnqueueInternal(const SPA::CStreamHeader &sh, const unsigned char *buffer, unsigned int size);

    private:
        CMqFile(const CMqFile &file);
        CMqFile& operator=(const CMqFile &file);

        inline void SetInitialEnqueuePosition();
        inline bool IsInDequeuing(SPA::UINT64 nMsgIndex) const;
        inline bool RemoveMsgIndex(SPA::UINT64 pos, SPA::UINT64 nMsgIndex);
        unsigned int RemoveMsgIndex(const QAttr *qa, unsigned int count);
        unsigned int RemoveMsgIndex(const CDequeueConfirmInfo *start, unsigned int count);
        inline void SetFileHandlers();
        inline bool SetFilePointer(SPA::INT64 offset, int origin);
        inline void CloseFile();
        inline void ResetInternal();
        inline int GetFileDescriptor() const;
        inline bool Truncate(SPA::UINT64 newSize);
        inline int fflush(FILE *file);
        inline size_t fwrite(const void *buffer, size_t size, size_t count, FILE *stream);
        inline size_t fread(void *buffer, size_t size, size_t count, FILE *stream);
        std::vector<SPA::UINT64> BatchEnqueueInternal(const SPA::CUQueue &qRequests);
        inline void RemoveMerge(const QAttr &start, const QAttr &end);
        bool ConfirmDequeueInternal(SPA::UINT64 pos, SPA::UINT64 index, bool fail = false);
        unsigned int MakeAllFailed(const CDequeueConfirmInfo *start, unsigned int count);
        unsigned int ConfirmRange(const CDequeueConfirmInfo *start, unsigned int count);
        unsigned int ConfirmRangeEx(const CDequeueConfirmInfo *start, unsigned int count, bool &flush);
        unsigned int ConfirmRangeEx(const QAttr *start, unsigned int count, bool &flush);
        unsigned int ConfirmRange(const QAttr *start, unsigned int count);

    protected:
        static unsigned char empty[PAD_EMPTY_SIZE];

    private:
        SPA::CScopeUQueue m_suOut;
        SPA::CScopeUQueue m_suTransIndex;
        bool m_bStrange;
        SPA::INT64 m_CurrentReadPos;
        std::string m_fileName;
        unsigned int m_ttl;
        FILE *m_hFile;
        SPA::UINT64 m_nInternalIndex;
        bool m_bSecure;
        SPA::CUQueue &m_qOut;
        std::string m_rawName;
        SPA::UINT64 m_msgCount;
        SPA::tagOptimistic m_bCrashSafe;
        bool m_bEnableDequeue;
        SPA::UINT64 m_qTransPos;
        SPA::UINT64 m_msgTransCount;
        SPA::CUQueue &m_qTransIndex;
        bool m_bClient;
        bool m_bShared;
        mutex m_cs;
        condition_variable m_cv;
        SPA::UINT64 m_nMinIndex;
        unsigned int m_LastTime;
        static mutex m_csAppName;
        bool m_bEnd;
        SPA::UINT64 m_nFileSize;

    protected:
        SPA::tagQueueStatus m_qs;

    public:
        static std::string m_strAppName;
        static std::time_t TIME_Y_2013;
    };

#ifdef WINCE
    int _chsize(int fd, long size);
    int chsize(int fd, long size);
#endif

    class CQLastIndex;

    class CMqFileEx : public CMqFile {
    private:
        static QAttr m_qaTest;

    public:
        CMqFileEx(const char *fileName, unsigned int ttl, SPA::tagOptimistic crashSafe, const wchar_t *userId, const wchar_t *password, CQLastIndex *pLastIndex, bool client = false, bool shared = false);

    public:
        virtual SPA::UINT64 Dequeue(SPA::CUQueue &q, SPA::UINT64 &mqIndex, unsigned int waitTime);
        virtual std::vector<unsigned int> DoBatchDequeue(unsigned int requestCount, SPA::CUQueue &qAttr, SPA::CUQueue &qRequests, unsigned int waitTime);

    protected:
        virtual SPA::CUQueue* DoEncryption(const unsigned char *buffer, unsigned int len);
        virtual std::vector<unsigned int> DoBatchDequeueInternal(CAutoLock &al, SPA::CUQueue &qAttr, SPA::CUQueue &qRequests, unsigned int maxSize, unsigned int waitTime);
        virtual SPA::UINT64 EnqueueInternal(const SPA::CStreamHeader &sh, const unsigned char *buffer, unsigned int size);

    private:
        void PostProcess(const std::vector<unsigned int> &vMdh, SPA::CUQueue &qRequests);

    private:
        CMqFileEx(const CMqFileEx &file);
        CMqFileEx& operator=(const CMqFileEx &file);

    private:
		CBlowFishPtr m_bf;
    };

    /*
    #if defined(__ANDROID__) || defined(ANDROID)
        typedef std::unordered_map<QueueSha1, QAttr, qs_hash, qs_equal> CMyMap;
    #else
        typedef boost::unordered_map<QueueSha1, QAttr, qs_hash, qs_equal> CMyMap;
    #endif
     */

    typedef boost::unordered_map<QueueSha1, QAttr, qs_hash, qs_equal> CMyMap;

    class CQLastIndex : private CMyMap {
    public:
        CQLastIndex(const char *fileName, bool client = false);
        virtual ~CQLastIndex();

    public:
        const char *GetFileNmae() const;
        bool IsAvailable();
        inline bool IsDirty();
        void Set(const QueueSha1 &key, const QAttr &qa);
        void Remove(const QueueSha1 &key);
        QAttr Seek(const QueueSha1 &key);
        bool IsCrashed() const;
        void FinalSave();
        void Stop();

    private:
        //disable copy constructor and assignment operator
        CQLastIndex(const CQLastIndex &qli);
        CQLastIndex& operator=(const CQLastIndex &qli);

        bool Save();
        void SetFileHandler();
        void CloseFile();
        void DoFastSave();
        bool Load();
        bool SaveCheckSum();

    private:
        unsigned int m_nCrash;
        std::string m_fileName;
        bool m_bDirty;
        FILE *m_hFile;
        mutex m_cs;
        condition_variable m_cv;
        mutex m_csFile;
        thread *m_thread;
        volatile long m_stop;
        SPA::tagQueueStatus m_qs;
        unsigned int m_CheckSum;
    };

    class CMyContainer {
    private:
        CMyContainer();
        CMyContainer(const CMyContainer &c);

    public:
        static CMyContainer Container;
        std::string Get(SPA::UINT64 src);
        void Set(SPA::UINT64 src, const char *pwd);
        void Remove(SPA::UINT64 src);

    private:
        std::map<SPA::UINT64, std::vector<unsigned char> > m_objp;

    private:
        CMyContainer& operator=(const CMyContainer &c);

    private:
		CBlowFishPtr m_bf;
        mutex m_cs;
    };

    extern mutex g_csQFile;
    extern std::vector<CMqFile*> g_vQFile;
    extern std::vector<CQLastIndex*> g_vQLastIndex;

#ifndef WINCE
	typedef std::shared_ptr<CMqFile> CFilePtr;
	typedef std::shared_ptr<CQLastIndex> CQLastIndexPtr;
#else
	typedef boost::shared_ptr<CMqFile> CFilePtr;
	typedef boost::shared_ptr<CQLastIndex> CQLastIndexPtr;
#endif
};
