
//#include "stdafx.h"
#include "../../pinc/mqfile.h"
#include <string>
#include <assert.h>
#include "../../pinc/sha1.h"
#include <algorithm>
#include "../../include/membuffer.h"
#include "../../pinc/getsysid.h"
#include <algorithm> 
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <time.h>
#include "../../pinc/getsysid.h"
#include <fstream>

#ifdef WINCE
#elif defined(WIN32_64)
#include <io.h>
#include "../../pinc/WinCrashHandler.h"
#else
#include <unistd.h>
#include <signal.h>
#include <execinfo.h>
#endif

using namespace std;

SPA::CUCriticalSection g_csLCI;
SPA::CUQueue g_LastCallInfo;

void WINAPI SetLastCallInfo(const char *str) {
    SPA::CAutoLock al(g_csLCI);
    g_LastCallInfo.SetSize(0);
    g_LastCallInfo.Push(str);
}

namespace MQ_FILE {

    unsigned char CMqFile::empty[PAD_EMPTY_SIZE] ={0};

    std::time_t CMqFile::TIME_Y_2013 = SPA::Get2013();

    SPA::CUCriticalSection g_csQFile;
    std::vector<CMqFile*> g_vQFile;
    std::vector<CQLastIndex*> g_vQLastIndex;

#ifndef WIN32_64
    struct sigaction act;

    void handler(int sig, siginfo_t *info, void *ptr) {
        try {
            SPA::CAutoLock al(MQ_FILE::g_csQFile);
            for (auto it = MQ_FILE::g_vQFile.begin(), end = MQ_FILE::g_vQFile.end(); it != end; ++it) {
                (*it)->SetOptimistic(SPA::oSystemMemoryCached);
            }
            for (auto it = MQ_FILE::g_vQLastIndex.begin(), end = MQ_FILE::g_vQLastIndex.end(); it != end; ++it) {
                (*it)->FinalSave();
            }
        }        catch (std::exception & err) {

        }        catch (...) {

        }
        if (sig == SIGTSTP)
            return;
        std::ofstream myfile(SPA::GetAppName() + "_crash.txt");
        if (myfile.is_open()) {
            {
                SPA::CAutoLock al(g_csLCI);
                g_LastCallInfo.SetNull();
                myfile << "last call info = " << (const char*) g_LastCallInfo.GetBuffer() << std::endl;
            }
            myfile << "handler sig = " << sig << ", si_code = " << info->si_code << ", si_errno = " << info->si_errno << std::endl;
            switch (sig) {
                case SIGILL:
                case SIGFPE:
                case SIGSEGV:
                case SIGBUS:
                    myfile << "si_addr = " << info->si_addr << std::endl;
                    break;
                default:
                    break;
            }
            typedef void* PVOID;
            PVOID buffer[100] = {nullptr};
            //backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO);
            int nptrs = backtrace(buffer, sizeof (buffer) / sizeof (PVOID));
            char **strings = backtrace_symbols(buffer, nptrs);
            for (int n = 1; n < nptrs; ++n) {
                myfile << strings[n] << std::endl;
            }
            free(strings);
        }
        exit(sig);
    }
#endif

    CMqFile::CBadHandler::CBadHandler() {
#ifdef WINCE

#elif defined(WIN32_64)
        CCrashHandler::SetProcessExceptionHandlers();
        CCrashHandler::SetThreadExceptionHandlers();
#else
        //http://www.alexonlinux.com/signal-handling-in-linux
        //http://tiku.io/questions/71806/how-to-catch-segmentation-fault-in-linux

        memset(&act, 0, sizeof (act));
        act.sa_sigaction = handler;
        act.sa_flags = SA_SIGINFO;

        //This is an exception signal as well. Operating system sends a program this signal when it tries to access memory that does not belong to it.
        int res = sigaction(SIGSEGV, &act, nullptr); // install our handler

        //Floating point exception. This is another exception signal, issued by operating system when your application caused an exception.
        res = sigaction(SIGFPE, &act, nullptr);

        //Abort signal means you used used abort() API inside of your program.
        res = sigaction(SIGABRT, &act, nullptr);

        //Illegal instruction signal
        res = sigaction(SIGILL, &act, nullptr);

        //CTRL-C
        res = sigaction(SIGINT, &act, nullptr);

        //SIGKILL, on the contrary to SIGTERM, indicates abnormal termination of the program.
        //In contrast to SIGTERM and SIGINT, this signal cannot be caught or ignored, and the receiving process cannot perform any clean-up upon receiving this signal.
        //res = sigaction(SIGKILL, &act, nullptr);

        //Ctrl-Z sends a TSTP signal (SIGTSTP); by default, this causes the process to suspend execution.
        res = sigaction(SIGTSTP, &act, nullptr);

        //Ctrl-\ sends a QUIT signal (SIGQUIT); by default, this causes the process to terminate and dump core.
        res = sigaction(SIGQUIT, &act, nullptr);

        //This signal tells your program to terminate itself. Consider this as a signal to cleanly shut down while SIGKILL is an abnormal termination signal.
        res = sigaction(SIGTERM, &act, nullptr);

        // A signal is an asynchronous notification sent to a process or to a specific thread within the same process in order to notify it of an event that occurred.
        //res = sigaction(SIGPWR, &act, nullptr);
#endif
    }

    CMqFile::CBadHandler m_bh;

#ifdef WINCE

    int _chsize(int fd, long size) {
        DWORD cur;
        DWORD newCur;
        HANDLE h;
        BOOL ret;

        h = (HANDLE) fd;
        if (h == INVALID_HANDLE_VALUE) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return -1;
        }

        cur = SetFilePointer(h, 0, nullptr, FILE_CURRENT);
        if (cur == 0xffffffff)
            return -1;

        /* Move to where we want it.  */
        newCur = SetFilePointer(h, size, nullptr, FILE_BEGIN);
        if (newCur == 0xffffffff)
            return -1;

        /* And commit it as eof, effectivelly growing or shrinking.  */
        ret = SetEndOfFile(h);

        SetFilePointer(h, cur, nullptr, FILE_BEGIN);

        return ret ? 0 : -1;
    }

    int chsize(int fd, long size) {
        return _chsize(fd, size);
    }
#endif

    std::string CMqFile::m_strAppName;
    boost::mutex CMqFile::m_csAppName;

    CMqFile::CMqFile(const char *fileName, unsigned short port, unsigned int ttl, SPA::tagOptimistic crashSafe, bool secure, bool client, bool shared)
    : m_CurrentReadPos(0),
    m_fileName(fileName),
    m_ttl(ttl),
    m_hFile(nullptr),
    m_qOut(*m_suOut),
    m_bSecure(secure),
    m_nInternalIndex(0),
    m_msgCount(0),
    m_bCrashSafe(crashSafe),
    m_bEnableDequeue(true),
    m_qTransPos(INVALID_NUMBER),
    m_msgTransCount(0),
    m_qTransIndex(*m_suTransIndex),
    m_bClient(client),
    m_bShared(shared),
    m_nMinIndex(0),
    m_LastTime(0),
    m_qs(SPA::qsNormal),
    m_bEnd(false) {
        char str[2048 + 1] ={0};
        boost::filesystem::path rawPath(fileName);
#ifdef WINCE
        {
            SPA::CScopeUQueue su;
            const std::wstring &fileName = rawPath.filename().wstring();
            SPA::Utilities::ToUTF8(fileName.c_str(), fileName.size(), *su);
            m_rawName = (const char*) su->GetBuffer();
        }
#else
        m_rawName = rawPath.filename().string();
        {
            CAutoLock al(m_csAppName);
            if (m_strAppName.size() == 0)
                m_strAppName = SPA::GetAppName();
        }

#endif
        if (client) {
            //can't embed port # into queue file name, which will make client search by queue name complex
            //port = (unsigned short) SPA::GetOS();
            port = 0;
        }
#ifdef WINCE
        sprintf(str, "_%s_%d.mq%s", m_strAppName.c_str(), m_bSecure ? 1 : 0, client ? "c" : "s");
#elif defined(WIN32_64)
        sprintf_s(str, sizeof (str), "_%s_%d.mq%s", m_strAppName.c_str(), m_bSecure ? 1 : 0, client ? "c" : "s");
#else
        sprintf(str, "_%s_%d.mq%s", m_strAppName.c_str(), m_bSecure ? 1 : 0, client ? "c" : "s");
#endif
        m_fileName += str;

        boost::filesystem::path my_path(m_fileName);
        if (my_path.is_relative()) {
#ifdef WINCE

#else
            boost::filesystem::path default_path = boost::filesystem::current_path();
            boost::filesystem::path absolute_path = default_path / m_fileName;
            m_fileName = absolute_path.string();
#endif
        }
#ifdef WIN32_64
        std::transform(m_fileName.begin(), m_fileName.end(), m_fileName.begin(), ::tolower);
#endif
        SetFileHandlers();
        if (IsAvailable())
            SetInitialEnqueuePosition();
        if (m_LastTime == 0) {
            std::time_t rawtime = std::time(nullptr);
            m_LastTime = static_cast<unsigned int> (rawtime - TIME_Y_2013);
        }

        {
            SPA::CAutoLock al(g_csQFile);
            g_vQFile.push_back(this);
        }
    }

    CMqFile::~CMqFile() {
        {
            CAutoLock al(m_cs);
            CloseFile();
        }
        try {
            SPA::CAutoLock al(g_csQFile);
            g_vQFile.erase(std::find(g_vQFile.begin(), g_vQFile.end(), this));
        } catch (...) {

        }
    }

    SPA::tagQueueStatus CMqFile::GetQueueOpenStatus() const {
        return m_qs;
    }

    unsigned int CMqFile::GetTTL() const {
        return m_ttl;
    }

    SPA::tagOptimistic CMqFile::IsOptimistic() {
        m_cs.lock();
        SPA::tagOptimistic b = m_bCrashSafe;
        m_cs.unlock();
        return b;
    }

    void CMqFile::SetOptimistic(SPA::tagOptimistic bOptimistic) {
        m_cs.lock();
        if (bOptimistic && !m_bCrashSafe && m_hFile && m_qTransPos == INVALID_NUMBER)
            fflush(m_hFile);
        m_bCrashSafe = bOptimistic;
        m_cs.unlock();
    }

    SPA::UINT64 CMqFile::RemoveByTTL() {
        unsigned int startTime = 0;
        std::time_t rawtime = std::time(nullptr);
        unsigned int endTime = (unsigned int) (rawtime - TIME_Y_2013);
        if (endTime > m_ttl)
            endTime -= m_ttl;
        else
            endTime = 0;
        return CancelQueuedRequests(startTime, endTime);
    }

    unsigned int CMqFile::GetQueueTransSize() {
        m_cs.lock();
        unsigned int count = m_qTransIndex.GetSize() / sizeof (SPA::UINT64);
        m_cs.unlock();
        return count;
    }

    SPA::UINT64 CMqFile::GetJobSize() {
        m_cs.lock();
        SPA::UINT64 trans = m_msgTransCount;
        m_cs.unlock();
        return trans;
    }

    void CMqFile::CloseFile() {
        if (m_hFile != nullptr) {
            ::fclose(m_hFile);
            m_hFile = nullptr;
        }
        m_bEnd = false;
    }

    void CMqFile::SetFileHandlers() {
#ifdef WINCE
        m_hFile = ::fopen(m_fileName.c_str(), "r+b");
        if (m_hFile == nullptr) {
            m_hFile = ::fopen(m_fileName.c_str(), "w+b");
        }
#elif defined(WIN32_64)
        m_hFile = ::_fsopen(m_fileName.c_str(), "r+b", _SH_DENYWR);
        if (m_hFile == nullptr) {
            m_hFile = ::_fsopen(m_fileName.c_str(), "w+b", _SH_DENYWR);
        }
#else
        m_hFile = ::fopen(m_fileName.c_str(), "r+bx");
        if (m_hFile == nullptr) {
            m_hFile = ::fopen(m_fileName.c_str(), "w+bx");
        }
#endif
        if (m_hFile == nullptr)
            m_qs = SPA::qsFileError;
    }

    SPA::INT64 CMqFile::GetFileSize() {
        if (m_hFile != nullptr) {
#ifdef WINCE
            if (!m_bEnd) {
                ::fseek(m_hFile, 0, SEEK_END);
                m_bEnd = true;
            }
            if (::ferror(m_hFile) != 0) {
                assert(false);
            }
            return (SPA::UINT64)::ftell(m_hFile);
#elif defined(WIN32_64)
            if (!m_bEnd) {
                ::_fseeki64_nolock(m_hFile, 0, SEEK_END);
                m_bEnd = true;
            }
            return (SPA::INT64)::_ftelli64(m_hFile);
#else
            if (!m_bEnd) {
                ::fseeko64(m_hFile, 0, SEEK_END);
                m_bEnd = true;
            }
            return (SPA::UINT64)::ftello64(m_hFile);
#endif
        }
        return 0;
    }

    bool CMqFile::IsInDequeuing(SPA::UINT64 nMsgIndex) {
        unsigned int n, Count = m_qOut.GetSize() / sizeof (QAttr);
        const QAttr *qa = (const QAttr *) m_qOut.GetBuffer();
        for (n = 0; n < Count; ++n) {
            if (qa[n].MessageIndex == nMsgIndex)
                return true;
        }
        return false;
    }

    bool CMqFile::RemoveMsgIndex(SPA::UINT64 pos, SPA::UINT64 nMsgIndex) {
        unsigned int n, Count = m_qTransIndex.GetSize() / sizeof (SPA::UINT64);
        SPA::UINT64 *pIndex = (SPA::UINT64 *)m_qTransIndex.GetBuffer();
        for (n = 0; n < Count; ++n) {
            if (pIndex[n] == nMsgIndex) {
                m_qTransIndex.Pop(sizeof (SPA::UINT64), n * sizeof (SPA::UINT64));
                break;
            }
        }
        Count = m_qOut.GetSize() / sizeof (QAttr);
        const QAttr *qa = (const QAttr *) m_qOut.GetBuffer();
        for (n = 0; n < Count; ++n) {
            if (qa[n].MessageIndex == nMsgIndex && qa[n].MessagePos == pos) {
                m_qOut.Pop((unsigned int) sizeof (QAttr), (unsigned int) n * sizeof (QAttr));
                return true;
            }
        }
        return false;
    }

    unsigned int CMqFile::GetMessagesInDequeuing() {
        CAutoLock al(m_cs);
        return m_qOut.GetSize() / sizeof (QAttr);
    }

    const QAttr * CMqFile::GetMessageAttributesInDequeuing() {
        CAutoLock al(m_cs);
        return (const QAttr*) m_qOut.GetBuffer();
    }

    void CMqFile::ReleaseMessageAttributesInDequeuing() {
        CAutoLock al(m_cs);
        m_qTransIndex.SetSize(0);
        if (m_qOut.GetSize() / sizeof (QAttr)) {
            const QAttr *qa = (const QAttr*) m_qOut.GetBuffer();
            m_CurrentReadPos = qa->MessagePos;
            m_qOut.SetSize(0);
        }
    }

    const std::string & CMqFile::GetMQFileName() const {
        return m_fileName;
    }

    CQueueInitialInfo CMqFile::GetMQInitInfo() const {
        CQueueInitialInfo qii;
        SPA::CSHA1 sha1;
        sha1.Update((const unsigned char*) m_fileName.c_str(), (unsigned int) m_fileName.size());
        sha1.Final();
        bool ok = sha1.GetHash(qii.Qs.Sha1);
        qii.MinIndex = m_nMinIndex;
        qii.Secure = m_bSecure;
        qii.CrashSafe = m_bCrashSafe;
        return qii;
    }

    const std::string & CMqFile::GetRawName() const {
        return m_rawName;
    }

    bool CMqFile::IsDequeueOk() {
        CAutoLock al(m_cs);
        return m_bEnableDequeue;
    }

    SPA::UINT64 CMqFile::GetLastMessageIndex() {
        CAutoLock al(m_cs);
        return m_nInternalIndex;
    }

    bool CMqFile::IsDequeueShared() const {
        return m_bShared;
    }

    void CMqFile::EnableDequeue(bool enable) {
        CAutoLock al(m_cs);
        m_bEnableDequeue = enable;
    }

    int CMqFile::fflush(FILE * file) {
        int res;
#ifdef WINCE
        BOOL ok = FALSE;
        res = ::fflush(file);
        if (res == 0 && m_bCrashSafe == SPA::oDiskCommitted) {
            ok = ::FlushFileBuffers((HANDLE) GetFileDescriptor());
            return 0;
        }
        res = ::ferror(file);
        return res;
#elif defined(WIN32_64)
        BOOL ok = FALSE;
        res = ::_fflush_nolock(file);
        if (res == 0 && m_bCrashSafe == SPA::oDiskCommitted)
            ok = ::FlushFileBuffers((HANDLE) _get_osfhandle(GetFileDescriptor()));
#else
        int ok = 0;
        res = ::fflush(file);
        if (res == 0 && m_bCrashSafe == SPA::oDiskCommitted)
            ok = ::fsync(GetFileDescriptor());
#endif
        return res;
    }

    size_t CMqFile::fwrite(const void *buffer, size_t size, size_t count, FILE * stream) {
#ifdef WINCE
        size_t written = ::fwrite(buffer, size, count, stream);
        if (written == 0 && count > 0) {
            int errCode = ::ferror(stream);
            return written;
        }
        return written;
#elif defined(WIN32_64)
        return ::_fwrite_nolock(buffer, size, count, stream);
#else
        return ::fwrite(buffer, size, count, stream);
#endif
    }

    size_t CMqFile::fread(void *buffer, size_t size, size_t count, FILE * stream) {
#ifdef WINCE
        size_t read = ::fread(buffer, size, count, stream);
        if (read == 0)
            return read;
        return read;
#elif defined(WIN32_64)
        return ::_fread_nolock(buffer, size, count, stream);
#else
        return ::fread(buffer, size, count, stream);
#endif
    }

    SPA::UINT64 CMqFile::GetMQSize() {
        CAutoLock al(m_cs);
        return (SPA::UINT64)GetFileSize();
    }

    bool CMqFile::SetFilePointer(SPA::INT64 offset, int origin) {
        if (origin != SEEK_END)
            m_bEnd = false;
#ifdef WINCE
        // res may be -1
        int res = ::fseek(m_hFile, offset, origin);
        if (res == 0)
            return true;
        int error = ::ferror(m_hFile);
        return (error == 0);
#elif defined(WIN32_64)
        return (::_fseeki64_nolock(m_hFile, offset, origin) == 0);
#else
        return (::fseeko64(m_hFile, offset, origin) == 0);
#endif
    }

    void CMqFile::SetInitialEnqueuePosition() {
        size_t read;
        bool badAppend = false;
        SPA::UINT64 MergePos = INVALID_NUMBER;
        SPA::UINT64 MergeCount = 0;
        bool incomplete = false;
        int merge = 0;
        SPA::UINT64 balance = 0;
        SPA::UINT64 pos = 0;
        SPA::UINT64 fileSize = (SPA::UINT64)GetFileSize();
        bool ok = SetFilePointer(0, SEEK_SET);
        assert(ok);
        MessageDecriptionHeaderEx mdh;
        while ((pos + sizeof (mdh)) <= fileSize) {
            read = fread(&mdh, 1, sizeof (mdh), m_hFile);
            assert(read == sizeof (mdh));
            switch (mdh.RequestId) {
                case SPA::idStartMerge:
                    if (mdh.MessageIndex) {
                        MergePos = pos;
                        m_qs = SPA::qsMergeComplete;
                        merge = 1;
                        MergeCount = 1;
                        SPA::INT64 offset = -((SPA::INT64)sizeof (mdh));
                        SetFilePointer(offset, SEEK_CUR);
                        mdh.MessageIndex = 0;
                        fwrite(&mdh, sizeof (mdh), 1, m_hFile);
                        ++m_msgCount;
                    }
                    break;
                case SPA::idEndMerge:
                    if (mdh.MessageIndex) {
                        SPA::INT64 offset = -((SPA::INT64)sizeof (mdh));
                        SetFilePointer(offset, SEEK_CUR);
                        mdh.MessageIndex = 0;
                        fwrite(&mdh, sizeof (mdh), 1, m_hFile);
                        if (merge == 1) {
                            assert(MergeCount == 1);
                            MergeCount = 0;
                            --m_msgCount;
                        } else {
                            assert(merge == 0);
                            MergePos = pos;
                            assert(MergeCount == 0);
                            merge -= 1;
                        }
                    } else if (merge)
                        badAppend = true;
                    break;
                default:
                    if (merge)
                        ++MergeCount;
                    break;
            }
            if (badAppend)
                break;
            if ((pos + sizeof (MessageDecriptionHeader) + mdh.Size) > fileSize) {
                incomplete = true;
                break;
            }

            pos += sizeof (mdh);
            if (mdh.Len > sizeof (SPA::CStreamHeader)) {
                ok = SetFilePointer(mdh.Len - sizeof (SPA::CStreamHeader), SEEK_CUR);
                assert(ok);
                pos += (mdh.Len - sizeof (SPA::CStreamHeader));
            }
            if (mdh.MessageIndex) {
                if (mdh.RequestId == SPA::idStartJob) {
                    if (balance == 0)
                        m_qTransPos = pos - sizeof (mdh);
                    ++m_msgTransCount;
                    ++balance;
                } else if (mdh.RequestId == SPA::idEndJob) {
                    --balance;
                    if (balance == 0) {
                        m_msgTransCount = 0;
                        m_qTransPos = INVALID_NUMBER;
                    }
                } else if (m_qTransPos != INVALID_NUMBER)
                    ++m_msgTransCount;
            }

            if (mdh.MessageIndex) {
                m_LastTime = mdh.Time;
                if (!m_nMinIndex)
                    m_nMinIndex = mdh.MessageIndex;
                m_nInternalIndex = mdh.MessageIndex;
                ++m_msgCount;
            } else
                ++m_nInternalIndex;
        }
        //make sure any data written into disk before trancating.
        fflush(m_hFile);
        if (merge) {
            Truncate(MergePos);
            if (merge > 0) {
                m_qTransPos = INVALID_NUMBER;
                m_msgTransCount = 0;
                assert(m_msgCount >= MergeCount);
                m_msgCount -= MergeCount;
                m_nInternalIndex -= MergeCount;
                m_qs = SPA::qsMergeIncomplete;
            } else {
                m_qs = SPA::qsMergePushing;
            }
        } else if (m_qTransPos != INVALID_NUMBER) {
            assert(balance == 1);
            Truncate(m_qTransPos);
            m_qTransPos = INVALID_NUMBER;
            assert(m_msgCount >= m_msgTransCount);
            assert(m_nInternalIndex >= m_msgTransCount);
            m_nInternalIndex -= m_msgTransCount;
            m_msgCount -= m_msgTransCount;
            m_msgTransCount = 0;
            m_qs = SPA::qsJobIncomplete;
        } else if (incomplete) {
            assert(pos >= sizeof (mdh));
            pos -= sizeof (mdh);
            Truncate(pos);
            m_qs = SPA::qsCrash;
        } else if (pos < fileSize) {
            Truncate(pos);
            m_qs = SPA::qsCrash;
        } else if (m_msgCount == 0 && fileSize >= FILE_SIZE_TRUNCATED) {
            assert(balance == 0);
            assert(m_msgTransCount == 0);
            assert(pos == fileSize);
            Truncate(0);
            m_nInternalIndex = 0;
        }
        if (m_msgCount == 0)
            m_nInternalIndex = 0;
    }

    unsigned int CMqFile::PeekRequests(SPA::CQueuedRequestInfo *qri, unsigned int count) {
        size_t read;
        if (!qri || !count)
            return 0;
        unsigned char buffer[sizeof (MessageDecriptionHeader) + sizeof (SPA::CStreamHeader)];
        SPA::UINT64 pos = 0;
        unsigned int index = 0;
        CAutoLock al(m_cs);
        if (!m_hFile)
            return false;
        SPA::UINT64 fileSize = (SPA::UINT64)GetFileSize();
        if (m_qTransPos != INVALID_NUMBER) {
            assert(fileSize > m_qTransPos);
            fileSize = m_qTransPos;
        }
        bool ok = SetFilePointer(0, SEEK_SET);
        assert(ok);
        while (index < count && (pos + sizeof (buffer)) <= fileSize) {
            read = fread(buffer, sizeof (buffer), 1, m_hFile);
            assert(read == 1);
            MessageDecriptionHeader *mdh = (MessageDecriptionHeader*) buffer;
            SPA::CStreamHeader *sh = (SPA::CStreamHeader*)(buffer + sizeof (MessageDecriptionHeader));
            pos += sizeof (buffer);
            if (mdh->Len > sizeof (SPA::CStreamHeader)) {
                unsigned int move = mdh->Len - sizeof (SPA::CStreamHeader);
                ok = SetFilePointer(move, SEEK_CUR);
                assert(ok);
                pos += move;
            } else if (mdh->Len == sizeof (SPA::CStreamHeader)) {

            } else if (mdh->MessageIndex || mdh->Len) {
#ifndef NDEBUG
                std::cout << "mdh->Len == " << mdh->Len << ", index = " << mdh->MessageIndex << std::endl;
                std::cout.flush();
#endif
                assert(false);
            }

            if (mdh->MessageIndex) {
                ::memcpy(qri + index, sh, sizeof (SPA::CStreamHeader));
                ++index;
            }
        }
        return index;
    }

    SPA::UINT64 CMqFile::GetMessageCount() {
        CAutoLock al(m_cs);
        return m_msgCount;
    }

    void CMqFile::Reset() {
        CAutoLock al(m_cs);
        ResetInternal();
    }

    SPA::UINT64 CMqFile::CancelQueuedRequests(unsigned int startTime, unsigned int endTime) {
        size_t res;
        MessageDecriptionHeader mdh;
        SPA::UINT64 pos = 0;
        SPA::UINT64 cancels = 0;
        if (startTime > endTime) {
            std::swap(startTime, endTime);
        }
        CAutoLock al(m_cs);
        if (!m_hFile)
            return 0;
        SPA::UINT64 fileSize = (SPA::UINT64)GetFileSize();
        if (m_qTransPos != INVALID_NUMBER)
            fileSize = m_qTransPos;
        bool ok = SetFilePointer(0, SEEK_SET);
        assert(ok);
        while ((pos + sizeof (mdh)) <= fileSize) {
            res = fread(&mdh, sizeof (mdh), 1, m_hFile);
            assert(res == 1);
            pos += sizeof (mdh);
            if (mdh.Time >= endTime)
                break;
            if (mdh.MessageIndex && mdh.Time > startTime) {
                RemoveMsgIndex(pos, mdh.MessageIndex);
                ok = SetFilePointer(-16, SEEK_CUR); //-16 == back sizeof (MessageDecriptionHeader)
                mdh.MessageIndex = 0;
                res = fwrite(&mdh, sizeof (mdh), 1, m_hFile);
                assert(res == 1);
                ++cancels;
            }
            if (mdh.Len) {
                ok = SetFilePointer(mdh.Len, SEEK_CUR);
                assert(ok);
                pos += mdh.Len;
            }
        }
        assert(m_msgCount >= cancels);
        m_msgCount -= cancels;
        if (m_bCrashSafe && cancels)
            fflush(m_hFile);
        if (m_msgCount == 0 && GetFileSize() > FILE_SIZE_TRUNCATED) {
            assert(m_qOut.GetSize() == 0);
            ResetInternal();
        }
        return cancels;
    }

    SPA::UINT64 CMqFile::CancelQueuedRequests(SPA::UINT64 startIndex, SPA::UINT64 endIndex) {
        size_t res;
        MessageDecriptionHeader mdh;
        SPA::UINT64 pos = 0;
        SPA::UINT64 cancels = 0;
        if (startIndex == 0)
            startIndex = 1;
        if (endIndex == 0)
            endIndex = 1;
        if (startIndex > endIndex) {
            std::swap(startIndex, endIndex);
        }
        CAutoLock al(m_cs);
        if (!m_hFile)
            return 0;
        SPA::UINT64 fileSize = (SPA::UINT64)GetFileSize();
        if (m_qTransPos != INVALID_NUMBER)
            fileSize = m_qTransPos;
        bool ok = SetFilePointer(0, SEEK_SET);
        assert(ok);
        while ((pos + sizeof (mdh)) <= fileSize) {
            res = fread(&mdh, sizeof (mdh), 1, m_hFile);
            assert(res == 1);
            pos += sizeof (mdh);
            if (mdh.MessageIndex > endIndex)
                break;
            if (mdh.MessageIndex >= startIndex) {
                RemoveMsgIndex(pos, mdh.MessageIndex);
                ok = SetFilePointer(-16, SEEK_CUR); //-16 == back sizeof (MessageDecriptionHeader)
                mdh.MessageIndex = 0;
                res = fwrite(&mdh, sizeof (mdh), 1, m_hFile);
                assert(res == 1);
                ++cancels;
            }
            if (mdh.Len) {
                ok = SetFilePointer(mdh.Len, SEEK_CUR);
                assert(ok);
                pos += mdh.Len;
            }
        }
        assert(m_msgCount >= cancels);
        m_msgCount -= cancels;
        if (m_bCrashSafe && cancels)
            fflush(m_hFile);
        if (m_msgCount == 0 && GetFileSize() > FILE_SIZE_TRUNCATED) {
            assert(m_qOut.GetSize() == 0);
            ResetInternal();
        }
        return cancels;
    }

    unsigned int CMqFile::CancelQueuedRequests(const unsigned short *ids, unsigned int count) {
        size_t read;
        static const int BUFFER_SIZE = sizeof (MessageDecriptionHeader) + sizeof (SPA::CStreamHeader);
        bool hasStart = false;
        SPA::UINT64 startPos;
        SPA::UINT64 startIndex;
        unsigned int middle = 0;
        unsigned char buffer[BUFFER_SIZE];
        if (!ids || !count)
            return 0;
        std::vector<unsigned short> vStartEnd;
        std::vector<unsigned short> vTargetRequest(ids, ids + count);
        int balance = 0;
        std::vector<unsigned short>::iterator start, end = vTargetRequest.end();
        for (start = vTargetRequest.begin(); start != end; ++start) {
            unsigned short reqId = *start;
            switch (reqId) {
                case SPA::idStartJob:
                    ++balance;
                    vStartEnd.push_back(reqId);
                    break;
                case SPA::idEndJob:
                    --balance;
                    vStartEnd.push_back(reqId);
                    break;
                default:
                    break;
            }
        }

        while (balance != 0) {
            if (balance > 0) {
                start = std::find(vStartEnd.begin(), vStartEnd.end(), SPA::idStartJob);
                --balance;
            } else {
                start = std::find(vStartEnd.begin(), vStartEnd.end(), SPA::idEndJob);
                ++balance;
            }
            vStartEnd.erase(start);
        }

        SPA::UINT64 pos = 0;
        unsigned int index = 0;
        CAutoLock al(m_cs);
        if (!m_hFile)
            return 0;
        SPA::UINT64 fileSize = (SPA::UINT64)GetFileSize();
        if (m_qTransPos != INVALID_NUMBER) {
            assert(fileSize > m_qTransPos);
            fileSize = m_qTransPos;
        }
        bool ok = SetFilePointer(0, SEEK_SET);
        assert(ok);
        while (index < count && (pos + sizeof (buffer)) <= fileSize) {
            read = fread(buffer, sizeof (buffer), 1, m_hFile);
            assert(read == 1);
            MessageDecriptionHeader *mdh = (MessageDecriptionHeader*) buffer;
            SPA::CStreamHeader *sh = (SPA::CStreamHeader*)(buffer + sizeof (MessageDecriptionHeader));
            if (mdh->MessageIndex) {
                switch (sh->RequestId) {
                    case SPA::idStartJob:
                        middle = 0;
                        hasStart = true;
                        startPos = pos;
                        startIndex = mdh->MessageIndex;
                        break;
                    case SPA::idEndJob:
                        if (middle || vStartEnd.size() == 0) {
                            middle = 0;
                            //leave idStartJob and idEndJob as they are
                            break;
                        } else if (vStartEnd.size() > 0) {
                            vStartEnd.erase(std::find(vStartEnd.begin(), vStartEnd.end(), SPA::idEndJob));
                            vStartEnd.erase(std::find(vStartEnd.begin(), vStartEnd.end(), SPA::idStartJob));
                            //let the below block to set message index = 0
                        }
                    default:
                    {
                        std::vector<unsigned short>::iterator find = std::find(vTargetRequest.begin(), vTargetRequest.end(), sh->RequestId);
                        if (find != vTargetRequest.end()) {
                            vTargetRequest.erase(find);
                            ++index;
                            RemoveMsgIndex(pos, mdh->MessageIndex);
                            mdh->MessageIndex = 0;
                            SPA::INT64 offset = (-BUFFER_SIZE);
                            ok = SetFilePointer(offset, SEEK_CUR);
                            assert(ok);
                            size_t res = fwrite(buffer, sizeof (buffer), 1, m_hFile);
                            assert(res == 1);

                            if (sh->RequestId == SPA::idEndJob && hasStart) {
                                hasStart = false;
                                ++index;
                                RemoveMsgIndex(startPos, startIndex);
                                vTargetRequest.erase(std::find(vTargetRequest.begin(), vTargetRequest.end(), SPA::idStartJob));
                                ok = SetFilePointer(startPos, SEEK_SET);
                                assert(ok);
                                sh->RequestId = SPA::idStartJob;
                                res = fwrite(buffer, sizeof (buffer), 1, m_hFile);
                                assert(res == 1);
                                ok = SetFilePointer(pos + sizeof (buffer), SEEK_SET);
                                assert(ok);
                            }
                        } else {
                            ++middle;
                        }
                    }
                        break;
                }
            }
            pos += sizeof (buffer);
            if (mdh->Len > sizeof (SPA::CStreamHeader)) {
                unsigned int move = mdh->Len - sizeof (SPA::CStreamHeader);
                ok = SetFilePointer(move, SEEK_CUR);
                assert(ok);
                pos += move;
            } else if (mdh->Len == sizeof (SPA::CStreamHeader)) {

            } else if (mdh->MessageIndex || mdh->Len) {
#ifndef NDEBUG
                std::cout << "mdh->Len == " << mdh->Len << ", index = " << mdh->MessageIndex << std::endl;
                std::cout.flush();
#endif
                assert(false);
            }
        }
        assert(index <= m_msgCount);
        if (m_bCrashSafe && index)
            fflush(m_hFile);
        if (m_msgCount == 0 && GetFileSize() > FILE_SIZE_TRUNCATED) {
            assert(m_qOut.GetSize() == 0);
            ResetInternal();
        }
        m_msgCount -= index;
        return index;
    }

    void CMqFile::ResetInternal() {
        m_qOut.SetSize(0);
        m_CurrentReadPos = 0;
        m_msgCount = 0;
        m_nMinIndex = 0;
        if (!m_bCrashSafe && m_hFile)
            fflush(m_hFile);
        bool ok = Truncate(0);
        assert(ok);
    }

    bool CMqFile::Truncate(SPA::UINT64 newSize) {
        int fd = GetFileDescriptor();
        m_bEnd = false;
#ifdef WINCE
        return (chsize(fd, (__int64) newSize) == 0);
#elif defined(WIN32_64)
        return (::_chsize_s(fd, (__int64) newSize) == 0);
#else
        return (::ftruncate64(fd, (off64_t) newSize) == 0);
#endif
    }

    int CMqFile::GetFileDescriptor() {
#ifdef WINCE
        return (int) ::fileno(m_hFile);
#elif defined(WIN32_64)
        return ::_fileno(m_hFile);
#else
        return ::fileno(m_hFile);
#endif
    }

    bool CMqFile::ConfirmDequeue(const QAttr *qa, size_t count, bool fail) {
        bool ok;
        size_t res;
        if (qa == nullptr)
            count = 0;
        bool flush = false;
        CAutoLock al(m_cs);
        if (!m_hFile)
            return false;
        for (size_t n = 0; n < count; ++n) {
            bool found = RemoveMsgIndex(qa[n].MessagePos, qa[n].MessageIndex);
            if (!found) //it happens possibly if a queue is reset
                return true;
            if (fail) {
                if (qa[n].MessagePos < (SPA::UINT64)m_CurrentReadPos)
                    m_CurrentReadPos = (SPA::INT64) qa[n].MessagePos;
                m_cv.notify_all();
            } else {
                SPA::UINT64 mi = 0;
                ok = SetFilePointer(qa[n].MessagePos, SEEK_SET);
                assert(ok);
                res = fwrite(&mi, sizeof (mi), 1, m_hFile);
                assert(res == 1);
                assert(m_msgCount > 0);
                --m_msgCount;
                flush = true;
            }
        }
        if ((m_bCrashSafe || m_msgCount == 0) && flush)
            fflush(m_hFile);
        if (m_msgCount == 0 && GetFileSize() > FILE_SIZE_TRUNCATED) {
            assert(m_qOut.GetSize() == 0);
            ResetInternal();
        }
        return true;
    }

    bool CMqFile::ConfirmDequeue(SPA::UINT64 pos, SPA::UINT64 index, bool fail) {
        CAutoLock al(m_cs);
        return ConfirmDequeueInternal(pos, index, fail);
    }

    bool CMqFile::ConfirmDequeueInternal(SPA::UINT64 pos, SPA::UINT64 index, bool fail) {
        bool ok;
        size_t res;
        bool flush = false;
        if (!m_hFile)
            return false;
        bool found = RemoveMsgIndex(pos, index);
        if (!found) //it happens possibly if a queue is reset
            return true;
        if (fail) {
            if (pos < (SPA::UINT64)m_CurrentReadPos)
                m_CurrentReadPos = (SPA::INT64) pos;
            m_cv.notify_all();
        } else {
            index = 0;
            ok = SetFilePointer(pos, SEEK_SET);
            assert(ok);
            res = fwrite(&index, sizeof (index), 1, m_hFile);
            assert(res == 1);
            flush = true;
            assert(m_msgCount > 0);
            --m_msgCount;
        }
        if ((m_bCrashSafe || m_msgCount == 0) && flush)
            fflush(m_hFile);
        if (m_msgCount == 0 && GetFileSize() > FILE_SIZE_TRUNCATED) {
            assert(m_qOut.GetSize() == 0);
            ResetInternal();
        }
        return true;
    }

    vector<unsigned int> CMqFile::DoBatchDequeue(unsigned int requestCount, SPA::CUQueue &qAttr, SPA::CUQueue &qRequests, unsigned int waitTime) {
        size_t read;
        SPA::INT64 fileSize;
        bool again = false;
        unsigned int index = 0;
        QAttr attri;
        MessageDecriptionHeader mdh;
        qAttr.SetSize(0);
        qRequests.SetSize(0);
        vector<unsigned int> vMdh;
        if (!requestCount)
            return vMdh;
        CAutoLock al(m_cs);
        if (!m_hFile || !m_bEnableDequeue)
            return vMdh;
        do {
            fileSize = GetFileSize();
            if (m_qTransPos != INVALID_NUMBER) {
                assert((SPA::UINT64)fileSize > m_qTransPos);
                fileSize = m_qTransPos;
            }
            bool ok = SetFilePointer(m_CurrentReadPos, SEEK_SET);
            assert(ok);
            while (m_CurrentReadPos < fileSize) {
                if (index >= requestCount) {
                    if (!m_bShared)
                        break;
                    else if (m_qTransIndex.GetSize() == 0)
                        break;
                }
                read = fread(&mdh, sizeof (mdh), 1, m_hFile);
                if (!read)
                    break;
                assert(mdh.Len >= sizeof (SPA::CStreamHeader));
                if (mdh.MessageIndex == 0 || IsInDequeuing(mdh.MessageIndex)) {
                    if (mdh.Len) {
                        ok = SetFilePointer(mdh.Len, SEEK_CUR);
                        assert(ok);
                    }
                    m_CurrentReadPos += (sizeof (mdh) + mdh.Len);
                } else {
                    attri.MessageIndex = mdh.MessageIndex;
                    attri.MessagePos = (SPA::UINT64)m_CurrentReadPos;
                    qAttr << attri;
                    vMdh.push_back(mdh.Len);
                    if (qRequests.GetTailSize() < mdh.Len)
                        qRequests.ReallocBuffer(qRequests.GetSize() + mdh.Len);
                    read = fread((unsigned char*) qRequests.GetBuffer(qRequests.GetSize()), mdh.Len, 1, m_hFile);
                    assert(read == 1);
                    SPA::CStreamHeader *sh = (SPA::CStreamHeader *)qRequests.GetBuffer(qRequests.GetSize());
                    switch (sh->RequestId) {
                        case SPA::idStartJob:
                        case SPA::idEndJob:
                            m_qTransIndex << attri.MessageIndex;
                            break;
                        default:
                            break;
                    }
                    qRequests.SetSize(qRequests.GetSize() + mdh.Len);
                    m_CurrentReadPos += (sizeof (mdh) + mdh.Len);
                    m_qOut << attri;
                    if (m_qTransIndex.GetSize() / sizeof (SPA::UINT64) == 2)
                        break;
                    ++index;
                }
            }
            if (!vMdh.size() && waitTime && !again) {
                boost::system_time td = boost::get_system_time() + boost::posix_time::milliseconds(waitTime);
                m_cv.timed_wait(al, td);
                again = true;
            } else
                again = false;
        } while (again);
        return vMdh;
    }

    std::vector<unsigned int> CMqFile::DoBatchDequeue(SPA::CUQueue &qAttr, SPA::CUQueue &qRequests, unsigned int maxSizeInByte, unsigned int waitTime) {
        CAutoLock al(m_cs);
        return DoBatchDequeueInternal(al, qAttr, qRequests, maxSizeInByte, waitTime);
    }

    std::vector<unsigned int> CMqFile::DoBatchDequeueInternal(MQ_FILE::CAutoLock &al, SPA::CUQueue &qAttr, SPA::CUQueue &qRequests, unsigned int maxSize, unsigned int waitTime) {
        size_t read;
        SPA::INT64 fileSize;
        bool again = false;
        QAttr attri;
        MessageDecriptionHeader mdh;
        qAttr.SetSize(0);
        qRequests.SetSize(0);
        if (qRequests.GetMaxSize() < maxSize)
            qRequests.ReallocBuffer(maxSize);
        vector<unsigned int> vMdh;
        if (!m_hFile || !m_bEnableDequeue)
            return vMdh;
        do {
            fileSize = GetFileSize();
            if (m_qTransPos != INVALID_NUMBER) {
                assert((SPA::UINT64)fileSize > m_qTransPos);
                fileSize = m_qTransPos;
            }
            bool ok = SetFilePointer(m_CurrentReadPos, SEEK_SET);
            assert(ok);
            while (m_CurrentReadPos < fileSize) {
                if (qRequests.GetSize() >= maxSize) {
                    if (!m_bShared)
                        break;
                    else if (m_qTransIndex.GetSize() == 0)
                        break;
                }
                read = fread(&mdh, sizeof (mdh), 1, m_hFile);
                if (!read)
                    break;
                assert(mdh.Len >= sizeof (SPA::CStreamHeader));
                if (mdh.MessageIndex == 0 || IsInDequeuing(mdh.MessageIndex)) {
                    if (mdh.Len) {
                        ok = SetFilePointer(mdh.Len, SEEK_CUR);
                        assert(ok);
                    }
                    m_CurrentReadPos += (sizeof (mdh) + mdh.Len);
                } else {
                    attri.MessageIndex = mdh.MessageIndex;
                    attri.MessagePos = (SPA::UINT64)m_CurrentReadPos;
                    qAttr << attri;
                    vMdh.push_back(mdh.Len);
                    if (qRequests.GetTailSize() < mdh.Len)
                        qRequests.ReallocBuffer(qRequests.GetSize() + mdh.Len);
                    read = fread((unsigned char*) qRequests.GetBuffer(qRequests.GetSize()), mdh.Len, 1, m_hFile);
                    assert(read == 1);

                    SPA::CStreamHeader *sh = (SPA::CStreamHeader *)qRequests.GetBuffer(qRequests.GetSize());
                    switch (sh->RequestId) {
                        case SPA::idStartJob:
                        case SPA::idEndJob:
                            m_qTransIndex << attri.MessageIndex;
                            break;
                        default:
                            break;
                    }

                    qRequests.SetSize(qRequests.GetSize() + mdh.Len);
                    m_CurrentReadPos += (sizeof (mdh) + mdh.Len);
                    m_qOut << attri;
                    if (m_qTransIndex.GetSize() / sizeof (SPA::UINT64) == 2)
                        break;
                }
            }
            if (!vMdh.size() && waitTime && !again) {
                boost::system_time td = boost::get_system_time() + boost::posix_time::milliseconds(waitTime);
                m_cv.timed_wait(al, td);
                again = true;
            } else
                again = false;
        } while (again);
        return vMdh;
    }

    SPA::UINT64 CMqFile::Dequeue(SPA::CUQueue &q, SPA::UINT64 &mqIndex, unsigned int waitTime) {
        size_t read;
        SPA::INT64 fileSize;
        bool again = false;
        MessageDecriptionHeader mdh;
        SPA::UINT64 pos = INVALID_NUMBER;
        mqIndex = INVALID_NUMBER;
        q.SetSize(0);
        CAutoLock al(m_cs);
        if (!m_hFile || !m_bEnableDequeue)
            return pos;
        do {
            fileSize = GetFileSize();
            if (m_qTransPos != INVALID_NUMBER) {
                assert((SPA::UINT64)fileSize > m_qTransPos);
                fileSize = m_qTransPos;
            }
            bool ok = SetFilePointer(m_CurrentReadPos, SEEK_SET);
            assert(ok);
            while (m_CurrentReadPos < fileSize) {
                read = fread(&mdh, sizeof (mdh), 1, m_hFile);
                if (!read)
                    break;
                assert(mdh.Len >= sizeof (SPA::CStreamHeader));
                if (mdh.MessageIndex == 0 || IsInDequeuing(mdh.MessageIndex)) {
                    if (mdh.Len) {
                        ok = SetFilePointer(mdh.Len, SEEK_CUR);
                        assert(ok);
                    }
                    m_CurrentReadPos += (sizeof (mdh) + mdh.Len);
                } else {
                    mqIndex = mdh.MessageIndex;
                    pos = (SPA::UINT64)m_CurrentReadPos;
                    if (mdh.Len > q.GetMaxSize())
                        q.ReallocBuffer(mdh.Len);
                    read = fread((unsigned char*) q.GetBuffer(), mdh.Len, 1, m_hFile);
                    assert(read == 1);
                    SPA::CStreamHeader *sh = (SPA::CStreamHeader *)q.GetBuffer();
                    switch (sh->RequestId) {
                        case SPA::idStartJob:
                        case SPA::idEndJob:
                            m_qTransIndex << mqIndex;
                            break;
                        default:
                            break;
                    }
                    q.SetSize(mdh.Len);
                    m_CurrentReadPos += (sizeof (mdh) + mdh.Len);
                    QAttr qa(pos, mqIndex);
                    m_qOut << qa;
                    break;
                }
            }
            if (pos == INVALID_NUMBER && waitTime && !again) {
                boost::system_time td = boost::get_system_time() + boost::posix_time::milliseconds(waitTime);
                m_cv.timed_wait(al, td);
                again = true;
            } else
                again = false;
        } while (again);
        return pos;
    }

    bool CMqFile::IsAvailable() {
        CAutoLock al(m_cs);
        return (m_hFile != nullptr);
    }

    void CMqFile::StopQueue(SPA::tagQueueStatus qs) {
        {
            CAutoLock al(m_cs);
            if (m_hFile) {
                ::fclose(m_hFile);
                m_hFile = nullptr;
            }
            m_msgCount = 0;
            m_CurrentReadPos = 0;
            m_qOut.SetSize(0);
            m_qs = qs;
        }
        m_cv.notify_all();
    }

    bool CMqFile::IsSecure() const {
        return m_bSecure;
    }

    void CMqFile::Notify() {
        m_cv.notify_all();
    }

    SPA::CUQueue * CMqFile::DoEncryption(const unsigned char *buffer, unsigned int len) {
        assert(false); //shouldn't be called!
        return nullptr;
    }

    std::vector<SPA::UINT64> CMqFile::BatchEnqueueInternal(const SPA::CUQueue & qRequests) {
        bool ok;
        MessageDecriptionHeader mdh;
        std::time_t rawtime = std::time(nullptr);
        mdh.Time = static_cast<unsigned int> (rawtime - TIME_Y_2013);
        std::vector<SPA::UINT64> vIndex;
        const unsigned char *pBuffer = qRequests.GetBuffer();
        unsigned int len = qRequests.GetSize();
        SPA::CUQueue *q = nullptr;
        if (!m_hFile)
            return vIndex;
        if (!m_bEnd) {
            ok = SetFilePointer(0, SEEK_END);
            assert(ok);
            m_bEnd = true;
        }
        while (len >= sizeof (SPA::CStreamHeader)) {
            ++m_nInternalIndex;
            mdh.MessageIndex = m_nInternalIndex;
            SPA::CStreamHeader *pStreamHeader = (SPA::CStreamHeader*)pBuffer;
            len -= sizeof (SPA::CStreamHeader);
            pBuffer += sizeof (SPA::CStreamHeader);

            unsigned int size = sizeof (SPA::CStreamHeader);
            if (pStreamHeader->Size) {
                if (m_bSecure) {
                    q = DoEncryption(pBuffer, pStreamHeader->Size);
                    size += q->GetSize();
                } else
                    size += pStreamHeader->Size;
            }
            mdh.Len = size;

            size_t written = fwrite(&mdh, sizeof (mdh), 1, m_hFile); //4 bytes
            assert(written == 1);

            written = fwrite(pStreamHeader, sizeof (SPA::CStreamHeader), 1, m_hFile); //8 bytes
            assert(written == 1);

            if (pStreamHeader->Size) {
                if (m_bSecure) {
                    written = fwrite(q->GetBuffer(), q->GetSize(), 1, m_hFile);
                    SPA::CScopeUQueue::Unlock(q);
                    q = nullptr;
                } else
                    written = fwrite(pBuffer, pStreamHeader->Size, 1, m_hFile);
                assert(written == 1);
            }

            ++m_msgCount;

            if (m_msgCount == (m_qOut.GetSize() / sizeof (QAttr) + 1)) {
                m_cv.notify_all();
            }

            len -= pStreamHeader->Size;
            pBuffer += pStreamHeader->Size;

            vIndex.push_back(m_nInternalIndex);
        }
        if (m_bCrashSafe && qRequests.GetSize() >= sizeof (SPA::CStreamHeader))
            fflush(m_hFile);
        assert(!len);
        return vIndex;
    }

    std::vector<SPA::UINT64> CMqFile::BatchEnqueue(const SPA::CUQueue & qRequests) {
        CAutoLock al(m_cs);
        return BatchEnqueueInternal(qRequests);
    }

    SPA::UINT64 CMqFile::StartJob() {
        SPA::CStreamHeader sh;
        sh.RequestId = SPA::idStartJob;
        return Enqueue(sh, (const unsigned char*) nullptr, 0);
    }

    SPA::UINT64 CMqFile::EndJob() {
        SPA::CStreamHeader sh;
        sh.RequestId = SPA::idEndJob;
        return Enqueue(sh, (const unsigned char*) nullptr, 0);
    }

    bool CMqFile::AbortJob() {
        CAutoLock al(m_cs);
        if (m_qTransPos == INVALID_NUMBER)
            return false;
        fflush(m_hFile);
        if (Truncate(m_qTransPos)) {
            m_qTransPos = INVALID_NUMBER;
            m_msgCount -= m_msgTransCount;
            assert(m_nInternalIndex >= m_msgTransCount);
            m_nInternalIndex -= m_msgTransCount;
            m_msgTransCount = 0;
            return true;
        }
        return false;
    }

    SPA::UINT64 CMqFile::Enqueue(const SPA::CStreamHeader &sh, const unsigned char *buffer, unsigned int size) {
        CAutoLock al(m_cs);
        return EnqueueInternal(sh, buffer, size);
    }

    unsigned int CMqFile::GetLastTime() {
        CAutoLock al(m_cs);
        return m_LastTime;
    }

    SPA::UINT64 CMqFile::EnqueueInternal(const SPA::CStreamHeader &sh, const unsigned char *buffer, unsigned int size) {
        size_t written;
        if (buffer == nullptr)
            size = 0;
        unsigned int len = size;
        MessageDecriptionHeader mdh;
        mdh.Len = size + sizeof (sh);
        std::time_t rawtime = std::time(nullptr);
        mdh.Time = static_cast<unsigned int> (rawtime - TIME_Y_2013);
        if (!m_hFile)
            return INVALID_NUMBER;

        //for queue trans/commit
        if (sh.RequestId == SPA::idStartJob) {
            if (m_qTransPos != INVALID_NUMBER) {
                return INVALID_NUMBER;
            }
            m_qTransPos = (SPA::UINT64)GetFileSize();
            assert(m_msgTransCount == 0);
            ++m_msgTransCount;
        } else if (sh.RequestId == SPA::idEndJob) {
            if (m_qTransPos == INVALID_NUMBER) {
                return INVALID_NUMBER;
            }
        } else if (m_qTransPos != INVALID_NUMBER) {
            ++m_msgTransCount;
        }

        if (m_qTransPos == INVALID_NUMBER)
            m_LastTime = mdh.Time;
        if (!m_bEnd) {
            bool ok = SetFilePointer(0, SEEK_END);
            assert(ok);
            m_bEnd = true;
        }
        mdh.MessageIndex = ++m_nInternalIndex;
        written = fwrite(&mdh, sizeof (mdh), 1, m_hFile); //16 bytes
        assert(written == 1);

        written = fwrite(&sh, sizeof (SPA::CStreamHeader), 1, m_hFile); //8 bytes
        assert(written == 1);

        if (len) {
            written = fwrite(buffer, len, 1, m_hFile);
            assert(written == 1);
        }
        ++m_msgCount;
        if ((m_msgCount - m_msgTransCount) == (m_qOut.GetSize() / sizeof (QAttr) + 1)) {
            m_cv.notify_all();
        }

        if (sh.RequestId == SPA::idEndJob) {
            m_qTransPos = INVALID_NUMBER;
            m_msgTransCount = 0;
        }
        if (m_qTransPos == INVALID_NUMBER && m_bCrashSafe)
            fflush(m_hFile);
        return m_nInternalIndex;
    }

    SPA::UINT64 CMqFile::Enqueue(const SPA::CStreamHeader &sh, SPA::CUQueue & buffer) {
        if (m_bSecure) {
            unsigned int tail = (buffer.GetSize() % PAD_EMPTY_SIZE);
            if (tail) {
                buffer.Push(empty, (unsigned int) PAD_EMPTY_SIZE - tail);
            }
        }
        return Enqueue(sh, buffer.GetBuffer(), buffer.GetSize());
    }

    bool CMqFile::AppendTo(CMqFile **qFiles, unsigned int count) {
        unsigned int n;
        CMqFile *qFile;
        SPA::UINT64 indexMerge = 0;
        const unsigned int BATCH_DEQUEUE_SIZE = 8 * 1024;
        if (!qFiles || !count)
            return false;
        for (n = 0; n < count; ++n) {
            qFile = qFiles[n];
            if (!qFile || !qFile->IsAvailable())
                return false;
        }
        QAttr attr;
        SPA::CStreamHeader sh;
        SPA::CScopeUQueue su;
        SPA::CUQueue &qAttr = *su;
        SPA::CScopeUQueue su1;
        SPA::CUQueue &qRequests = *su1;
        SPA::CScopeUQueue su2;
        SPA::CUQueue &qFinal = *su2;
        SPA::CScopeUQueue su3;
        SPA::CUQueue &merge = *su3;

        CAutoLock al(m_cs);
        if (m_msgCount == 0)
            return true;
        //in transaction
        if (m_qTransPos != INVALID_NUMBER)
            return false;

        //adjust memory ahead to avoid repeated memory relocation in case there are not queued requests
        m_qOut.SetHeadPosition();
        if (m_msgCount * sizeof (QAttr) > m_qOut.GetTailSize()) {
            unsigned int increase = (unsigned int) (m_msgCount * sizeof (QAttr) - m_qOut.GetTailSize());
            m_qOut.ReallocBuffer(m_qOut.GetMaxSize() + increase);
        }

        sh.RequestId = SPA::idStartMerge;
        for (n = 0; n < count; ++n) {
            qFile = qFiles[n];
            do {
                qFile->m_cs.lock();
                if (qFile->m_qTransPos == INVALID_NUMBER)
                    break;
                qFile->m_cs.unlock();
                boost::system_time td = boost::get_system_time() + boost::posix_time::milliseconds(1);
                boost::this_thread::sleep(td);
            } while (true);
            attr.MessagePos = (SPA::UINT64)qFile->GetFileSize();
            attr.MessageIndex = qFile->EnqueueInternal(sh, nullptr, 0);
            if (attr.MessageIndex == 1) {
                //make sure the other peer reset last index correctly
                qFile->m_nInternalIndex = 0;
            }
            merge << attr;
        }

        sh.RequestId = SPA::idEndMerge;
        indexMerge = EnqueueInternal(sh, nullptr, 0);

        std::vector<unsigned int> vSize = DoBatchDequeueInternal(al, qAttr, qRequests, BATCH_DEQUEUE_SIZE, 0);
        const QAttr *qattr = (const QAttr *) qAttr.GetBuffer();
        while (vSize.size()) {
            unsigned int rs = qRequests.GetSize();
            if (rs > qFinal.GetMaxSize()) {
                qFinal.ReallocBuffer(rs);
            }
            for (n = 0; n < count; ++n) {
                qFile = qFiles[n];
                if (m_bSecure) {
                    unsigned len;
                    for (std::vector<unsigned int>::iterator it = vSize.begin(), end = vSize.end(); it != end; ++it) {
                        len = *it;
                        qRequests >> sh;
#if 0
                        unsigned short reqId;
                        if (sh.RequestId == 500) {
                            reqId = sh.RequestId;
                        } else if (sh.RequestId == SPA::idStartJob) {
                            reqId = sh.RequestId;
                        } else if (sh.RequestId == SPA::idEndJob) {
                            reqId = sh.RequestId;
                        } else if (sh.RequestId == SPA::idEndMerge) {
                            reqId = sh.RequestId;
                        } else {
                            assert(false);
                        }
#endif
                        qFinal << sh;
                        len -= sizeof (sh);
                        if (sh.Size) {
                            qFinal.Push(qRequests.GetBuffer(), sh.Size);
                        }
                        qRequests.Pop(len);
                    }
                    qFile->BatchEnqueueInternal(qFinal);
                    qFinal.SetSize(0);
                } else {
                    qFile->BatchEnqueueInternal(qRequests);
                }
                qRequests.SetSize(rs);
            }

            qRequests.SetSize(0);
            qAttr.SetSize(0);
            vSize = DoBatchDequeueInternal(al, qAttr, qRequests, BATCH_DEQUEUE_SIZE, 0);
            qattr = (const QAttr *) qAttr.GetBuffer();
        }
        ResetInternal();

        //clean both merge header and footer
        for (n = 0; n < count; ++n) {
            const QAttr *qattr = (const QAttr *) merge.GetBuffer(n * sizeof (QAttr));
            qFile = qFiles[n];
            attr.MessageIndex = qFile->m_nInternalIndex;
            attr.MessagePos = (SPA::UINT64)(qFile->GetFileSize() - sizeof (MessageDecriptionHeaderEx));
            qFile->RemoveMerge(*qattr, attr);
            qFile->m_cs.unlock();
        }
        return true;
    }

    void CMqFile::RemoveMerge(const QAttr &start, const QAttr & end) {
        m_qOut << start << end;
        bool ok = ConfirmDequeueInternal(start.MessagePos, start.MessageIndex, false);
        assert(ok);
        ok = ConfirmDequeueInternal(end.MessagePos, end.MessageIndex, false);
        assert(ok);
    }

    SPA::UINT64 CMqFile::Append(CMqFile & mqFile) {
        bool ok;
        const unsigned int BATCH_DEQUEUE_SIZE = 8 * 1024;
        SPA::CStreamHeader sh;
        SPA::CScopeUQueue su;
        SPA::CUQueue &qAttr = *su;
        SPA::CScopeUQueue su1;
        SPA::CUQueue &qRequests = *su1;
        SPA::CScopeUQueue su2;
        SPA::CUQueue &qFinal = *su2;

        std::vector<unsigned int> vSize = mqFile.DoBatchDequeue(qAttr, qRequests, BATCH_DEQUEUE_SIZE, 0);
        const QAttr *qattr = (const QAttr *) qAttr.GetBuffer();
        CAutoLock al(m_cs);
        while (vSize.size()) {
            if (mqFile.IsSecure()) {
                unsigned len;
                for (std::vector<unsigned int>::iterator it = vSize.begin(), end = vSize.end(); it != end; ++it) {
                    len = *it;
                    qRequests >> sh;
                    qFinal << sh;
                    len -= sizeof (sh);
                    if (sh.Size) {
                        qFinal.Push(qRequests.GetBuffer(), sh.Size);
                    }
                    qRequests.Pop(len);
                }
                std::vector<SPA::UINT64> vIndex = BatchEnqueueInternal(qFinal);
                qFinal.SetSize(0);
            } else {
                std::vector<SPA::UINT64> vIndex = BatchEnqueueInternal(qRequests);
            }
            ok = mqFile.ConfirmDequeue(qattr, qAttr.GetSize() / sizeof (QAttr), false);
            qRequests.SetSize(0);
            qAttr.SetSize(0);
            vSize = mqFile.DoBatchDequeue(qAttr, qRequests, BATCH_DEQUEUE_SIZE, 0);
            qattr = (const QAttr *) qAttr.GetBuffer();
        }
        return m_nInternalIndex;
    }

    QAttr CMqFileEx::m_qaTest(0x8765432112345678, 0x8765432112345678);

    CMqFileEx::CMqFileEx(const char *fileName, unsigned short port, unsigned int ttl, SPA::tagOptimistic crashSafe, const wchar_t *userId, const wchar_t *password, CQLastIndex *pLastIndex, bool client, bool shared)
    : CMqFile(fileName, port, ttl, crashSafe, true, client, shared) {

        std::wstring pwd(password);
        if (pwd.size() == 0) {
            StopQueue(SPA::qsBadPassword);
            return;
        }

        std::wstring s(userId);
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        SPA::CScopeUQueue su;
        su->Push(s.c_str());
        su->Push(password);

        unsigned char bytes[128] ={0};
        SPA::CSHA1 sha1;
        sha1.Update(su->GetBuffer(), su->GetSize());
        sha1.Final();
        bool ok = sha1.GetHash(bytes);
        m_bf.reset(new CBlowFish(bytes, 20));
        memset(bytes, 0, sizeof (bytes));
        su->SetSize(0);
        su->CleanTrack();
        QAttr test = m_qaTest;
        m_bf->Encrypt((unsigned char*) &test, sizeof (test));

        if (pLastIndex && GetMessageCount()) {
            QAttr qa = pLastIndex->Seek(GetMQInitInfo().Qs);
            if (!(test == qa)) {
                StopQueue(m_qs = SPA::qsBadPassword);
            }
        } else if (pLastIndex && GetMessageCount() == 0 && IsAvailable()) {
            pLastIndex->Set(GetMQInitInfo().Qs, test);
        }
    }

    SPA::UINT64 CMqFileEx::EnqueueInternal(const SPA::CStreamHeader &sh, const unsigned char *buffer, unsigned int size) {
        if (!buffer)
            size = 0;
        unsigned int extra = (size % PAD_EMPTY_SIZE);
        if (extra) {
            SPA::CScopeUQueue su;
            su->Push(buffer, size);
            su->Push(empty, PAD_EMPTY_SIZE - extra);
            m_bf->Encrypt((unsigned char *) su->GetBuffer(), su->GetSize());
            return CMqFile::EnqueueInternal(sh, su->GetBuffer(), su->GetSize());
        } else if (size)
            m_bf->Encrypt((unsigned char *) buffer, size);
        return CMqFile::EnqueueInternal(sh, buffer, size);
    }

    SPA::CUQueue * CMqFileEx::DoEncryption(const unsigned char *buffer, unsigned int len) {
        SPA::CScopeUQueue su;
        su->Push(buffer, len);
        unsigned int tail = (su->GetSize() % 8);
        if (tail) {
            su->Push(empty, (unsigned int) 8 - tail);
        }
        m_bf->Encrypt((unsigned char *) su->GetBuffer(), su->GetSize());

        //will be recycled by caller
        return su.Detach();
    }

    SPA::UINT64 CMqFileEx::Dequeue(SPA::CUQueue &q, SPA::UINT64 &mqIndex, unsigned int waitTime) {
        SPA::UINT64 res = CMqFile::Dequeue(q, mqIndex, waitTime);
        if (res == INVALID_NUMBER || mqIndex == INVALID_NUMBER) {
            return res;
        }
        assert(q.GetSize() >= sizeof (SPA::CStreamHeader));
        if (q.GetSize() > sizeof (SPA::CStreamHeader))
            m_bf->Decrypt((unsigned char*) q.GetBuffer(sizeof (SPA::CStreamHeader)), q.GetSize() - sizeof (SPA::CStreamHeader));
        SPA::CStreamHeader *sh = (SPA::CStreamHeader *)q.GetBuffer();
        unsigned int size = sh->Size + sizeof (SPA::CStreamHeader);
        q.SetSize(size);
        return res;
    }

    void CMqFileEx::PostProcess(const vector<unsigned int> &vMdh, SPA::CUQueue & qRequests) {
        unsigned int pos = 0;
#ifdef WINCE
        for (stlp_std::vector<unsigned int>::const_iterator it = vMdh.begin(), end = vMdh.end(); it != end; ++it) {
#else
        for (vector<unsigned int>::const_iterator it = vMdh.cbegin(), end = vMdh.cend(); it != end; ++it) {
#endif
            unsigned char *data = (unsigned char*) qRequests.GetBuffer(pos + sizeof (SPA::CStreamHeader));
            unsigned int size = *it;
            if (size > sizeof (SPA::CStreamHeader))
                m_bf->Decrypt(data, size - sizeof (SPA::CStreamHeader));
            pos += size;
        }
    }

    std::vector<unsigned int> CMqFileEx::DoBatchDequeueInternal(CAutoLock &al, SPA::CUQueue &qAttr, SPA::CUQueue &qRequests, unsigned int maxSize, unsigned int waitTime) {
        vector<unsigned int> vMdh = CMqFile::DoBatchDequeueInternal(al, qAttr, qRequests, maxSize, waitTime);
        PostProcess(vMdh, qRequests);
        return vMdh;
    }

    std::vector<unsigned int> CMqFileEx::DoBatchDequeue(unsigned int requestCount, SPA::CUQueue &qAttr, SPA::CUQueue &qRequests, unsigned int waitTime) {
        vector<unsigned int> vMdh = CMqFile::DoBatchDequeue(requestCount, qAttr, qRequests, waitTime);
        PostProcess(vMdh, qRequests);
        return vMdh;
    }

    CQLastIndex::CQLastIndex(const char *fileName, bool client)
    : m_nCrash(0), m_fileName(fileName), m_bDirty(false), m_hFile(nullptr), m_stop(0), m_qs(SPA::qsNormal), m_CheckSum(0) {
        if (client)
            m_fileName += "_c.qdx";
        else
            m_fileName += "_s.qdx";
        SetFileHandler();
        Load();
        m_thread = m_tg.create_thread(boost::bind(&CQLastIndex::DoFastSave, this));

        //make sure the thread is already running
        boost::system_time td = boost::get_system_time() + boost::posix_time::milliseconds(50);
        boost::this_thread::sleep(td);

        {
            SPA::CAutoLock al(g_csQFile);
            g_vQLastIndex.push_back(this);
        }
    }

    CQLastIndex::~CQLastIndex() {
        Stop();
    }

    void CQLastIndex::Stop() {
        if (!m_stop) {
            m_stop = 1;
            m_cv.notify_one();
            m_thread->join();
            Save();
            CloseFile();
            try {
                SPA::CAutoLock al(g_csQFile);
                g_vQLastIndex.erase(std::find(g_vQLastIndex.begin(), g_vQLastIndex.end(), this));
            } catch (...) {

            }
            //std::cout << "Stopped" << std::endl;
        }
    }

    void CQLastIndex::DoFastSave() {
        bool waited;
        const unsigned int LONG_WAIT_TIME = 200;
        const unsigned int SHORT_WAIT_TIME = 0;
        unsigned int waitTime = LONG_WAIT_TIME;
        do {
            {
                boost::system_time td = boost::get_system_time() + boost::posix_time::milliseconds(waitTime);
                CAutoLock al(m_cs);
                waited = m_cv.timed_wait(al, td);
            }
            if (waited || IsDirty()) {
                Save();
                waitTime = SHORT_WAIT_TIME;
            } else if (waitTime == SHORT_WAIT_TIME) {
                Save();
                waitTime = LONG_WAIT_TIME;
            }
        } while (!m_stop);
    }

    bool CQLastIndex::IsCrashed() const {
        return (m_nCrash > 0);
    }

    const char* CQLastIndex::GetFileNmae() const {
        return m_fileName.c_str();
    }

    bool CQLastIndex::IsAvailable() {
        CAutoLock al(m_csFile);
        return (m_hFile != nullptr);
    }

    void CQLastIndex::SetFileHandler() {
        m_hFile = ::fopen(m_fileName.c_str(), "r+b");
        if (m_hFile == nullptr) {
            m_hFile = ::fopen(m_fileName.c_str(), "w+b");
        }
    }

    bool CQLastIndex::IsDirty() {
        m_cs.lock();
        bool dirty = m_bDirty;
        m_cs.unlock();
        return dirty;
    }

    bool CQLastIndex::SaveCheckSum() {
        m_cs.lock();
        bool save = (m_CheckSum > 0);
        m_cs.unlock();
        if (save) {
            boost::this_thread::yield();
            unsigned int checksum = 0;
            CAutoLock al(m_csFile);
            if (m_CheckSum == 0)
                return false;
#ifdef WINCE
            ::fseek(m_hFile, 0, SEEK_SET);
#else
            ::rewind(m_hFile);
#endif
            unsigned int written = (unsigned int) ::fwrite(&checksum, 1, sizeof (checksum), m_hFile);
            assert(written == sizeof (checksum));
            ::fflush(m_hFile);
            return true;
        }
        return false;
    }

    void CQLastIndex::FinalSave() {
        Save();
    }

    bool CQLastIndex::Save() {
        unsigned int n;
        unsigned int count;
        CQLastIndex::iterator it;
        unsigned int maps;
        SPA::CScopeUQueue su;
        SPA::CUQueue &qBuffer = *su;

        {
            CAutoLock al(m_cs);
            if (m_hFile == nullptr)
                return false;
            m_bDirty = false;
            maps = (unsigned int) size();
            qBuffer << m_CheckSum << maps;
            CQLastIndex::iterator e = end();
            for (it = begin(); it != e; ++it) {
                qBuffer << it->first;
                qBuffer << it->second;
            }
            count = qBuffer.GetSize() / sizeof (unsigned int);
            unsigned int *p = (unsigned int *) qBuffer.GetBuffer();

            //don't count the first two integers (m_CheckSum and maps)
            m_CheckSum = 0;
            for (n = 2; n < count; ++n) {
                m_CheckSum += p[n];
            }
            *p = m_CheckSum;
        }

        {
            CAutoLock al(m_csFile);
            //set file pointer to the beginning
#ifdef WINCE
            ::fseek(m_hFile, 0, SEEK_SET);
#else
            ::rewind(m_hFile);
#endif
            unsigned int written = (unsigned int) ::fwrite(qBuffer.GetBuffer(), 1, qBuffer.GetSize(), m_hFile);
            assert(written == qBuffer.GetSize());
            ::fflush(m_hFile);
            m_CheckSum = 0;
        }
        return true;
    }

    void CQLastIndex::CloseFile() {
        CAutoLock al(m_csFile);
        if (m_hFile != nullptr) {
            ::fclose(m_hFile);
            m_hFile = nullptr;
        }
    }

    bool CQLastIndex::Load() {
        unsigned int n = 0;
        unsigned int maps = 0;
        unsigned int fileSize = 0;
        SPA::CScopeUQueue su;
        SPA::CUQueue &qBuffer = *su;
        m_nCrash = 0;
        CMyMap &map = *this;
        if (m_hFile == nullptr)
            SetFileHandler();
        if (m_hFile == nullptr)
            return false;
        map.clear();
        m_bDirty = false;
#ifdef WINCE
        ::fseek(m_hFile, 0, SEEK_END);
        fileSize = (unsigned int) ::ftell(m_hFile);
#elif defined(WIN32_64)
        ::_fseeki64(m_hFile, 0, SEEK_END);
        fileSize = (unsigned int) ::_ftelli64(m_hFile);
#else
        ::fseeko64(m_hFile, 0, SEEK_END);
        fileSize = (unsigned int) ::ftello64(m_hFile);
#endif
#ifdef WINCE
        ::fseek(m_hFile, 0, SEEK_SET);
#else
        ::rewind(m_hFile);
#endif
        if (fileSize == 0)
            return true;
        m_nCrash = 1;
        if (qBuffer.GetMaxSize() < fileSize + sizeof (wchar_t))
            qBuffer.ReallocBuffer(fileSize + sizeof (wchar_t));
        unsigned int read = (unsigned int) ::fread((unsigned char*) qBuffer.GetBuffer(), 1, fileSize, m_hFile);
        if (read != fileSize)
            return false;
        qBuffer.SetSize(read);
        QueueSha1 key;
        QAttr qa;
        do {
            if (qBuffer.GetSize() >= sizeof (maps))
                qBuffer >> m_CheckSum >> maps;
            else
                break;
            unsigned int *p = (unsigned int*) qBuffer.GetBuffer();
            unsigned int bytes = qBuffer.GetSize();
            for (n = 0; n < maps; ++n) {
                if (qBuffer.GetSize() < (sizeof (key) + sizeof (qa)))
                    break;
                qBuffer >> key >> qa;
                map[key] = qa;
            }
            if (n != maps)
                break;
            if (!maps) {
                m_nCrash = 0;
                break;
            }
            bytes -= qBuffer.GetSize();
            unsigned int checkSum = 0;
            unsigned int count = bytes / sizeof (unsigned int);
            for (n = 0; n < count; ++n) {
                checkSum += p[n];
            }
            if (checkSum == m_CheckSum)
                m_nCrash = 0;
        } while (false);
        return true;
    }

    void CQLastIndex::Set(const QueueSha1 &key, const QAttr & qa) {
        if (qa.MessageIndex == INVALID_NUMBER || qa.MessagePos == INVALID_NUMBER)
            return;
        {
            CMyMap &map = *this;
            CAutoLock al(m_cs);
            CQLastIndex::iterator it = find(key);
            if (it != map.end() && it->second == qa)
                return;
            map[key] = qa;
            m_bDirty = true;
        }
        m_cv.notify_one();
        SaveCheckSum();
    }

    void CQLastIndex::Remove(const QueueSha1 & key) {
        bool dirty = false;
        {
            CMyMap &map = *this;
            CAutoLock al(m_cs);
            CQLastIndex::iterator it = find(key);
            if (it != map.end()) {
                map.erase(it);
                m_bDirty = true;
                dirty = true;
            }
        }
        if (dirty) {
            m_cv.notify_one();
            SaveCheckSum();
        }
    }

    QAttr CQLastIndex::Seek(const QueueSha1 & key) {
        QAttr qa;
        CAutoLock al(m_cs);
        CMyMap &map = *this;
        CQLastIndex::iterator it = find(key);
        if (it != map.end()) {
            qa = it->second;
        }
        return qa;
    }

    CMyContainer CMyContainer::Container;

    CMyContainer::CMyContainer() {
        boost::uuids::random_generator rgen;
        boost::uuids::uuid u = rgen();
        m_bf.reset(new CBlowFish((unsigned char*) &u, sizeof (u)));
    }

    std::string CMyContainer::Get(SPA::UINT64 src) {
        CAutoLock al(m_cs);
        if (m_objp.find(src) == m_objp.end())
            return "";
        std::vector<unsigned char> &buffer = m_objp[src];
        SPA::CScopeUQueue su;
#ifdef WINCE
        su->Push(&buffer.front(), (unsigned int) buffer.size());
#else
        su->Push(buffer.data(), (unsigned int) buffer.size());
#endif
        m_bf->Decrypt((unsigned char*) su->GetBuffer(), buffer.size());
        su->SetNull();
        return (const char*) su->GetBuffer();
    }

    void CMyContainer::Set(SPA::UINT64 src, const char *pwd) {
        SPA::CScopeUQueue su;
        su->Push(pwd);
        unsigned int len = su->GetSize();
        if (len == 0) {
            CAutoLock al(m_cs);
            m_objp.erase(src);
            return;
        }
        unsigned int left = (len % CMqFile::PAD_EMPTY_SIZE);
        if (left) {
            char c = 0;
            len = CMqFile::PAD_EMPTY_SIZE - left;
            do {
                su->Push(&c, 1);
                --len;
            } while (len);
        }
        m_bf->Encrypt((unsigned char*) su->GetBuffer(), su->GetSize());
        std::vector<unsigned char> buffer(su->GetBuffer(), su->GetBuffer(su->GetSize()));
        CAutoLock al(m_cs);
        m_objp[src] = buffer;
    }

    void CMyContainer::Remove(SPA::UINT64 src) {
        CAutoLock al(m_cs);
        m_objp.erase(src);
    }
}
