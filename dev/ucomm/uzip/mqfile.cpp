
#include "../core_shared/pinc/mqfile.h"
#include <assert.h>
#include "../core_shared/pinc/sha1.h"
#include "../include/membuffer.h"
#include "../core_shared/pinc/getsysid.h"
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <fstream>

#ifdef WINCE

#elif defined(WIN32_64)
#include <io.h>
#include "../core_shared/pinc/WinCrashHandler.h"
#else

#include <unistd.h>
#include <signal.h>

#if defined(__ANDROID__) || defined(ANDROID)

#else
#include <execinfo.h>
#include <boost/thread/thread_time.hpp>
#endif
#endif

using namespace std;

MQ_FILE::mutex g_csLCI;
SPA::CUQueue g_LastCallInfo;

void WINAPI SetLastCallInfo(const char *str) {
    MQ_FILE::CAutoLock al(g_csLCI);
    g_LastCallInfo.SetSize(0);
    g_LastCallInfo.Push(str);
}

SPA::UINT64 GetTimeTick() {
	SPA::UINT64 now;
#ifdef WIN32_64

#if _WIN32_WINNT > 0x0600
	now = ::GetTickCount64();
#else
	SYSTEMTIME st;
	::GetLocalTime(&st);
	FILETIME ft;
	::SystemTimeToFileTime(&st, &ft);
	now = ft.dwHighDateTime;
	now <<= 32;
	now += ft.dwLowDateTime;
	now /= 10000;
#endif
#else
	struct timeval start;
	gettimeofday(&start, NULL);
	now = start.tv_sec * 1000 + start.tv_usec / 1000;
#endif
	return now;
}

namespace MQ_FILE {

    unsigned char CMqFile::empty[PAD_EMPTY_SIZE] = {0};

    std::time_t CMqFile::TIME_Y_2013 = SPA::Get2013();

    mutex g_csQFile;
    std::vector<CMqFile*> g_vQFile;
    std::vector<CQLastIndex*> g_vQLastIndex;

#ifndef WIN32_64
    struct sigaction act;

    void handler(int sig, siginfo_t *info, void *ptr) {
        try {
            MQ_FILE::CAutoLock al(MQ_FILE::g_csQFile);
            for (auto it = MQ_FILE::g_vQFile.begin(), end = MQ_FILE::g_vQFile.end(); it != end; ++it) {
                (*it)->SetOptimistic(SPA::oSystemMemoryCached);
            }
            for (auto it = MQ_FILE::g_vQLastIndex.begin(), end = MQ_FILE::g_vQLastIndex.end(); it != end; ++it) {
                (*it)->FinalSave();
            }
        } catch (std::exception & err) {
        } catch (...) {
        }
        if (sig == SIGTSTP)
            return;
        std::ofstream myfile(SPA::GetAppName() + "_crash.txt");
        if (myfile.is_open()) {
            {
                MQ_FILE::CAutoLock al(g_csLCI);
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
#if defined(__ANDROID__) || defined(ANDROID)
#else
            PVOID buffer[100] = {nullptr};
            //backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO);
            int nptrs = backtrace(buffer, sizeof (buffer) / sizeof (PVOID));
            char **strings = backtrace_symbols(buffer, nptrs);
            for (int n = 1; n < nptrs; ++n) {
                myfile << strings[n] << std::endl;
            }
            free(strings);
#endif
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

        int res;
        memset(&act, 0, sizeof (act));
        act.sa_sigaction = handler;
        act.sa_flags = SA_SIGINFO;

        struct sigaction old_action;
        memset(&old_action, 0, sizeof (old_action));

        //This is an exception signal as well. Operating system sends a program this signal when it tries to access memory that does not belong to it.
        sigaction(SIGSEGV, nullptr, &old_action);
        if (!old_action.sa_handler) {
            //crash lib/x86_64-linux-gnu/libpthread.so.0(+0x10340) [0x7fbd68990340] inside java environment
            res = sigaction(SIGSEGV, &act, nullptr); // install our handler
        }

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

    mutex CMqFile::m_csAppName;
    std::string CMqFile::m_strAppName;

	const std::time_t CMqFile::m_start_time = std::time(nullptr);
	const SPA::UINT64 CMqFile::m_startTick = GetTimeTick();

    CMqFile::CMqFile(const char *fileName, unsigned int ttl, SPA::tagOptimistic crashSafe, bool secure, bool client, bool shared)
    :
#ifndef NDEBUG
    m_nJobBalanceDequeue(0),
    m_nJobBalanceEnqueue(0),
    m_nJobBalanceConfirm(0),
#endif
    m_bStrange(false),
    m_CurrentReadPos(0),
    m_fileName(fileName),
    m_ttl(ttl),
    m_hFile(nullptr),
    m_nInternalIndex(0),
    m_bSecure(secure),
    m_qOut(*m_suOut),
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
    m_bEnd(false),
    m_nFileSize(0) {
        char str[2048 + 1] = {0};
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
        if (IsAvailable()) {
            SetInitialEnqueuePosition();
        }
        if (m_LastTime == 0) {
            std::time_t rawtime = std::time(nullptr);
            m_LastTime = static_cast<unsigned int> (rawtime - TIME_Y_2013);
        }

        {
            CAutoLock al(g_csQFile);
            g_vQFile.push_back(this);
        }
    }

    CMqFile::~CMqFile() {
        {
            CAutoLock al(m_cs);
            CloseFile();
        }
        try {
            CAutoLock al(g_csQFile);
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
        CAutoLock al(m_cs);
        if (bOptimistic && !m_bCrashSafe && m_hFile && m_qTransPos == INVALID_NUMBER) {
            fflush(m_hFile);
        }
        m_bCrashSafe = bOptimistic;
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
            fflush(m_hFile);
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
        int err = errno;
#endif
        if (m_hFile == nullptr)
            m_qs = SPA::qsFileError;
        else {
#if defined(WINCE) || defined(__ANDROID__) || defined(ANDROID)
            ::fseek(m_hFile, 0, SEEK_END);
            m_nFileSize = ::ftell(m_hFile);
            ::fseek(m_hFile, 0, SEEK_SET);
#elif defined(WIN32_64)
            ::_fseeki64_nolock(m_hFile, 0, SEEK_END);
            m_nFileSize = (SPA::INT64)::_ftelli64(m_hFile);
            ::_fseeki64_nolock(m_hFile, 0, SEEK_SET);
#else
            ::fseeko64(m_hFile, 0, SEEK_END);
            m_nFileSize = (SPA::UINT64)::ftello64(m_hFile);
            ::fseeko64(m_hFile, 0, SEEK_SET);
#endif
            m_bEnd = false;
        }
    }

    bool CMqFile::IsInDequeuing(SPA::UINT64 nMsgIndex) const {
        unsigned int n, Count = m_qOut.GetSize() / sizeof (QAttr);
        const QAttr *qa = (const QAttr *) m_qOut.GetBuffer();
        if (!m_bStrange && Count && (qa[Count - 1].MessageIndex + 1) == nMsgIndex)
            return false;
        for (n = 0; n < Count; ++n) {
            if (qa[n].MessageIndex == nMsgIndex)
                return true;
        }
        return false;
    }

    unsigned int CMqFile::MakeAllFailed(const CDequeueConfirmInfo *start, unsigned int count) {
        if (!start || !count)
            return 0;
        unsigned int removed = 0;
        for (unsigned int n = 0; n < count; ++n) {
            const QAttr &qa = start[n].QA;
            if (RemoveMsgIndex(qa.MessagePos, qa.MessageIndex))
                ++removed;
        }
        return removed;
    }

    unsigned int CMqFile::GetMessagesInDequeuing() {
        CAutoLock al(m_cs);
        return m_qOut.GetSize() / sizeof (QAttr);
    }

    bool CMqFile::JustOne(unsigned int adding) {
        CAutoLock al(m_cs);
        return (m_msgCount == (adding + m_qOut.GetSize() / sizeof (QAttr)));
    }

    const QAttr * CMqFile::GetMessageAttributesInDequeuing() {
        CAutoLock al(m_cs);
        return (const QAttr*) m_qOut.GetBuffer();
    }

    void CMqFile::ReleaseMessageAttributesInDequeuing() {
        CAutoLock al(m_cs);
        m_qTransIndex.SetSize(0);
        unsigned int count = m_qOut.GetSize() / sizeof (QAttr);
        if (count) {
            const QAttr *qa = (const QAttr*) m_qOut.GetBuffer();
            for (unsigned int n = 0; n < count; ++n, ++qa) {
#ifndef NDEBUG
                bool removed = this->RemoveConfirmed(*qa);
                assert(removed);
#endif
                if (qa->MessagePos < (SPA::UINT64)m_CurrentReadPos) {
                    m_CurrentReadPos = qa->MessagePos;
                }
            }
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
        if (m_bShared) {
            qii.MinIndex |= CQueueInitialInfo::QUEUE_SHARED_INDEX;
        }
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
        return m_nFileSize;
    }

    bool CMqFile::SetFilePointer(SPA::INT64 offset, int origin) {
        if (origin != SEEK_END) {
            m_bEnd = false;
        }
#if defined(WINCE) || defined(__ANDROID__) || defined(ANDROID)
        // res may be -1
        int res = ::fseek(m_hFile, offset, origin);
        if (res == 0)
            return true;
        int error = ::ferror(m_hFile);
        return (error == 0);
#elif defined(WIN32_64)
        return (::_fseeki64_nolock(m_hFile, offset, origin) == 0);
#else
#ifndef NDEBUG
        boost::system_time start = boost::get_system_time();
#endif
        bool ok = (::fseeko64(m_hFile, offset, origin) == 0);
#ifndef NDEBUG
        boost::system_time end = boost::get_system_time();
        long long timediff = (end - start).total_milliseconds();
        if (timediff > 5) {
            std::cout << "Time diff= " << timediff << ", offset=" << offset << ", origin=" << origin << std::endl;
        }
#endif
        return ok;
#endif
    }

    void CMqFile::SetInitialEnqueuePosition() {
        size_t read;
        size_t res;
        unsigned int range_num = 0;
        unsigned int range = 0;
        bool foundIndex = false;
        bool badAppend = false;
        SPA::UINT64 MergePos = INVALID_NUMBER;
        SPA::UINT64 MergeCount = 0;
        bool incomplete = false;
        int merge = 0;
        SPA::UINT64 balance = 0;
        SPA::UINT64 pos = 0;
        SPA::UINT64 range_start_pos = 0;
        SPA::UINT64 range_start_index = 0;
        bool ok = SetFilePointer(0, SEEK_SET);
        assert(ok);
        MessageDecriptionHeaderEx mdh;
        while ((pos + sizeof (mdh)) <= m_nFileSize) {
            if (!foundIndex) {
                m_CurrentReadPos = pos;
            }
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
            if ((pos + sizeof (MessageDecriptionHeader) + mdh.Size) > m_nFileSize) {
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
                if ((mdh.MessageIndex & QAttr::RANGE_DEQUEUED_START) == QAttr::RANGE_DEQUEUED_START) {
                    range_start_pos = pos - (sizeof (mdh) + mdh.Len - sizeof (SPA::CStreamHeader));
#ifndef NDEBUG
                    if (range) {
                        std::cout << "==== Bad range balance: " << range << " @" << __FUNCTION__ << " @line " << __LINE__ << std::endl;
                    }
#endif
                    ++range;
                    range_start_index = (mdh.MessageIndex & (~QAttr::RANGE_DEQUEUED_START));
                } else if ((mdh.MessageIndex & QAttr::RANGE_DEQUEUED_END) == QAttr::RANGE_DEQUEUED_END) {
#ifndef NDEBUG
                    if (!range) {
                        std::cout << "==== Bad range balance: " << range << " @" << __FUNCTION__ << " @line " << __LINE__ << std::endl;
                    }
#endif
                    --range;
                    range_start_pos = 0;
                    range_start_index = 0;
                    range_num = 0;
                } else if (!range) {
                    foundIndex = true;
                    ++m_msgCount;
                } else {
                    ++range_num;
                }
                if (mdh.RequestId == SPA::idStartJob) {
#ifndef NDEBUG
                    if (balance) {
                        std::cout << "==== Bad balance/SPA::idStartJob: " << balance << " @" << __FUNCTION__ << " @line " << __LINE__ << std::endl;
                    }
#endif
                    m_qTransPos = pos - sizeof (mdh);
                    ++m_msgTransCount;
                    balance = 1;
                } else if (mdh.RequestId == SPA::idEndJob) {
#ifndef NDEBUG
                    if (balance != 1) {
                        std::cout << "==== Bad balance/SPA::idEndJob: " << balance << " @" << __FUNCTION__ << " @line " << __LINE__ << std::endl;
                    }
#endif
                    balance = 0;
                    m_msgTransCount = 0;
                    m_qTransPos = INVALID_NUMBER;
                } else if (m_qTransPos != INVALID_NUMBER) {
                    ++m_msgTransCount;
                }
                m_LastTime = mdh.Time;
                SPA::UINT64 thisIndex = (mdh.MessageIndex & (~(QAttr::RANGE_DEQUEUED_START | QAttr::RANGE_DEQUEUED_END)));
                if (!m_nMinIndex)
                    m_nMinIndex = thisIndex;
                if (m_nInternalIndex < thisIndex)
                    m_nInternalIndex = thisIndex;
                else {
                    ++m_nInternalIndex;
                }

            } else {
                ++m_nInternalIndex;
            }
        }

        if (range) {
            ok = SetFilePointer(range_start_pos, SEEK_SET);
            assert(ok);
            range_start_index = 0;
            res = fwrite(&range_start_index, sizeof (range_start_index), 1, m_hFile);
            if (range_start_pos < (SPA::UINT64)m_CurrentReadPos) {
                m_CurrentReadPos = range_start_pos;
            }
            m_msgCount += range_num;
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
            Truncate(pos);
            m_qs = SPA::qsCrash;
        } else if (pos < m_nFileSize) {
            Truncate(pos);
            m_qs = SPA::qsCrash;
        } else if (m_msgCount == 0 && m_nFileSize >= FILE_SIZE_TRUNCATED) {
            assert(balance == 0);
            assert(m_msgTransCount == 0);
            assert(pos == m_nFileSize);
            Truncate(0);
            m_nInternalIndex = 0;
        }
        assert(m_msgTransCount == 0);
        if (m_msgCount == 0) {
            m_nInternalIndex = 0;
            ResetInternal();
        }
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
        SPA::UINT64 fileSize = m_nFileSize;
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
        bool range = false;
        MessageDecriptionHeader mdh;
        SPA::UINT64 pos = 0;
        SPA::UINT64 cancels = 0;
        if (startTime > endTime) {
            std::swap(startTime, endTime);
        }
        CAutoLock al(m_cs);
        if (!m_hFile)
            return 0;
        SPA::UINT64 fileSize = m_nFileSize;
        if (m_qTransPos != INVALID_NUMBER)
            fileSize = m_qTransPos;
        bool ok = SetFilePointer(0, SEEK_SET);
        assert(ok);
        while ((pos + sizeof (mdh)) <= fileSize) {
            res = fread(&mdh, sizeof (mdh), 1, m_hFile);
            assert(res == 1);
            pos += sizeof (mdh);
            if ((mdh.MessageIndex & QAttr::RANGE_DEQUEUED_END) == QAttr::RANGE_DEQUEUED_END) {
                range = true;
                if (mdh.Len) {
                    ok = SetFilePointer(mdh.Len, SEEK_CUR);
                    assert(ok);
                    pos += mdh.Len;
                }
                continue;
            } else if ((mdh.MessageIndex & QAttr::RANGE_DEQUEUED_END) == QAttr::RANGE_DEQUEUED_END) {
                range = false;
                if (mdh.Len) {
                    ok = SetFilePointer(mdh.Len, SEEK_CUR);
                    assert(ok);
                    pos += mdh.Len;
                }
                continue;
            } else if (range) {
                if (mdh.Len) {
                    ok = SetFilePointer(mdh.Len, SEEK_CUR);
                    assert(ok);
                    pos += mdh.Len;
                }
                continue;
            }
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
        if (m_msgCount == 0 && m_nFileSize > FILE_SIZE_TRUNCATED) {
            assert(m_qOut.GetSize() == 0);
            ResetInternal();
        }
        return cancels;
    }

    SPA::UINT64 CMqFile::CancelQueuedRequests(SPA::UINT64 startIndex, SPA::UINT64 endIndex) {
        size_t res;
        bool range = false;
        MessageDecriptionHeader mdh;
        SPA::UINT64 pos = 0;
        SPA::UINT64 cancels = 0;
        if (startIndex == 0)
            startIndex = 1;
        if (endIndex == 0)
            endIndex = 1;
        if (startIndex >= QAttr::RANGE_DEQUEUED_END)
            startIndex = QAttr::RANGE_DEQUEUED_END - 1;
        if (endIndex >= QAttr::RANGE_DEQUEUED_END)
            endIndex = QAttr::RANGE_DEQUEUED_END - 1;
        if (startIndex > endIndex) {
            std::swap(startIndex, endIndex);
        }
        CAutoLock al(m_cs);
        if (!m_hFile)
            return 0;
        SPA::UINT64 fileSize = m_nFileSize;
        if (m_qTransPos != INVALID_NUMBER)
            fileSize = m_qTransPos;
        bool ok = SetFilePointer(0, SEEK_SET);
        assert(ok);
        while ((pos + sizeof (mdh)) <= fileSize) {
            res = fread(&mdh, sizeof (mdh), 1, m_hFile);
            assert(res == 1);
            pos += sizeof (mdh);
            if ((mdh.MessageIndex & QAttr::RANGE_DEQUEUED_END) == QAttr::RANGE_DEQUEUED_END) {
                range = true;
                if (mdh.Len) {
                    ok = SetFilePointer(mdh.Len, SEEK_CUR);
                    assert(ok);
                    pos += mdh.Len;
                }
                continue;
            } else if ((mdh.MessageIndex & QAttr::RANGE_DEQUEUED_END) == QAttr::RANGE_DEQUEUED_END) {
                range = false;
                if (mdh.Len) {
                    ok = SetFilePointer(mdh.Len, SEEK_CUR);
                    assert(ok);
                    pos += mdh.Len;
                }
                continue;
            } else if (range) {
                if (mdh.Len) {
                    ok = SetFilePointer(mdh.Len, SEEK_CUR);
                    assert(ok);
                    pos += mdh.Len;
                }
                continue;
            }
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
        if (m_msgCount == 0 && m_nFileSize > FILE_SIZE_TRUNCATED) {
            assert(m_qOut.GetSize() == 0);
            ResetInternal();
        }
        return cancels;
    }

    unsigned int CMqFile::CancelQueuedRequests(const unsigned short *ids, unsigned int count) {
        size_t read;
        bool range = false;
        bool hasStart = false;
        SPA::UINT64 startPos;
        SPA::UINT64 startIndex;
        unsigned int middle = 0;
        MessageDecriptionHeaderEx mdh;
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
        SPA::UINT64 fileSize = m_nFileSize;
        if (m_qTransPos != INVALID_NUMBER) {
            assert(fileSize > m_qTransPos);
            fileSize = m_qTransPos;
        }
        bool ok = SetFilePointer(0, SEEK_SET);
        assert(ok);
        while (index < count && (pos + sizeof (mdh)) <= fileSize) {
            read = fread(&mdh, sizeof (mdh), 1, m_hFile);
            assert(read == 1);
            if ((mdh.MessageIndex & QAttr::RANGE_DEQUEUED_END) == QAttr::RANGE_DEQUEUED_END) {
                range = true;
                pos += sizeof (mdh);
                if (mdh.Len > sizeof (SPA::CStreamHeader)) {
                    unsigned int move = mdh.Len - sizeof (SPA::CStreamHeader);
                    ok = SetFilePointer(move, SEEK_CUR);
                    assert(ok);
                    pos += move;
                }
                continue;
            } else if ((mdh.MessageIndex & QAttr::RANGE_DEQUEUED_END) == QAttr::RANGE_DEQUEUED_END) {
                range = false;
                pos += sizeof (mdh);
                if (mdh.Len > sizeof (SPA::CStreamHeader)) {
                    unsigned int move = mdh.Len - sizeof (SPA::CStreamHeader);
                    ok = SetFilePointer(move, SEEK_CUR);
                    assert(ok);
                    pos += move;
                }
                continue;
            } else if (range) {
                pos += sizeof (mdh);
                if (mdh.Len > sizeof (SPA::CStreamHeader)) {
                    unsigned int move = mdh.Len - sizeof (SPA::CStreamHeader);
                    ok = SetFilePointer(move, SEEK_CUR);
                    assert(ok);
                    pos += move;
                }
                continue;
            }
            SPA::CStreamHeader *sh = &mdh;
            assert(mdh.MessageIndex < QAttr::RANGE_DEQUEUED_END);
            if (mdh.MessageIndex) {
                switch (sh->RequestId) {
                    case SPA::idStartJob:
                        middle = 0;
                        hasStart = true;
                        startPos = pos;
                        startIndex = mdh.MessageIndex;
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
                            RemoveMsgIndex(pos, mdh.MessageIndex);
                            mdh.MessageIndex = 0;
                            SPA::INT64 offset = -24; //sizeof(mdh);
                            ok = SetFilePointer(offset, SEEK_CUR);
                            assert(ok);
                            size_t res = fwrite(&mdh, sizeof (mdh), 1, m_hFile);
                            assert(res == 1);

                            if (sh->RequestId == SPA::idEndJob && hasStart) {
                                hasStart = false;
                                ++index;
                                RemoveMsgIndex(startPos, startIndex);
                                vTargetRequest.erase(std::find(vTargetRequest.begin(), vTargetRequest.end(), SPA::idStartJob));
                                ok = SetFilePointer(startPos, SEEK_SET);
                                assert(ok);
                                sh->RequestId = SPA::idStartJob;
                                res = fwrite(&mdh, sizeof (mdh), 1, m_hFile);
                                assert(res == 1);
                                ok = SetFilePointer(pos + sizeof (mdh), SEEK_SET);
                                assert(ok);
                            }
                        } else {
                            ++middle;
                        }
                    }
                        break;
                }
            }
            pos += sizeof (mdh);
            if (mdh.Len > sizeof (SPA::CStreamHeader)) {
                unsigned int move = mdh.Len - sizeof (SPA::CStreamHeader);
                ok = SetFilePointer(move, SEEK_CUR);
                assert(ok);
                pos += move;
            } else if (mdh.Len == sizeof (SPA::CStreamHeader)) {

            } else if (mdh.MessageIndex || mdh.Len) {
#ifndef NDEBUG
                std::cout << "mdh->Len == " << mdh.Len << ", index = " << mdh.MessageIndex << std::endl;
                std::cout.flush();
#endif
                assert(false);
            }
        }
        assert(index <= m_msgCount);
        if (m_bCrashSafe && index)
            fflush(m_hFile);
        if (m_msgCount == 0 && m_nFileSize > FILE_SIZE_TRUNCATED) {
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
        if (m_hFile)
            fflush(m_hFile);
        m_bStrange = false;
        bool ok = Truncate(0);
#ifndef NDEBUG
        m_sbConfirmed->SetSize(0);
#endif
        assert(ok);
    }

    bool CMqFile::Truncate(SPA::UINT64 newSize) {
        bool ok;
        int fd = GetFileDescriptor();
        m_bEnd = false;
#ifdef WINCE
        ok = (chsize(fd, (__int64) newSize) == 0);
#elif defined(WIN32_64)
        ok = (::_chsize_s(fd, (__int64) newSize) == 0);
#else
        ok = (::ftruncate64(fd, (off64_t) newSize) == 0);
#endif
        m_nFileSize = newSize;
        return ok;
    }

    int CMqFile::GetFileDescriptor() const {
#ifdef WINCE
        return (int) ::fileno(m_hFile);
#elif defined(WIN32_64)
        return ::_fileno(m_hFile);
#else
        return fileno(m_hFile);
#endif
    }

    unsigned int CMqFile::DoConfirmDequeue(const QAttr *qa, size_t count) {
        bool flush = false;
        CAutoLock al(m_cs);
        if (!m_hFile)
            return 0;
        unsigned int confirms = ConfirmRangeEx(qa, (unsigned int) count, flush);
#ifndef NDEBUG
        if (m_nJobBalanceConfirm && m_msgCount == 0) {
            std::cout << "---- DoConfirmDequeue: bad job confirm balance " << m_nJobBalanceConfirm << std::endl;
        }
#endif
        if ((m_bCrashSafe || m_msgCount == 0) && flush)
            fflush(m_hFile);
        if (m_msgCount == 0 && m_nFileSize > FILE_SIZE_TRUNCATED) {
            assert(m_qOut.GetSize() == 0);
            ResetInternal();
        }
        return confirms;
    }

    unsigned int CMqFile::DoConfirmDequeue(const SPA::CUQueue &qDCI) {
        bool flush = false;
        CAutoLock al(m_cs);
        if (!m_hFile)
            return 0;
        const CDequeueConfirmInfo *pDCIStart = (const CDequeueConfirmInfo *) qDCI.GetBuffer();
        unsigned int count = qDCI.GetSize() / sizeof (CDequeueConfirmInfo);
        unsigned int confirms = ConfirmRangeEx(pDCIStart, count, flush);
#ifndef NDEBUG
        if (m_nJobBalanceConfirm && m_msgCount == 0) {
            std::cout << "---- DoConfirmDequeue: bad job confirm balance " << m_nJobBalanceConfirm << std::endl;
        }
#endif
        if ((m_bCrashSafe || m_msgCount == 0) && flush)
            fflush(m_hFile);
        if (m_msgCount == 0 && m_nFileSize > FILE_SIZE_TRUNCATED) {
            assert(m_qOut.GetSize() == 0);
            ResetInternal();
        }
        return confirms;
    }

    unsigned int CMqFile::ConfirmRangeEx(const QAttr *start, unsigned int count, bool &flush) {
        unsigned int confirms = 0;
        int continues = 0;
        SPA::UINT64 prev_index = 0;
        const QAttr *dciPrev = nullptr;
        for (unsigned int n = 0; n < count; ++n) {
            const QAttr *dci = start + n;
            if (dciPrev == nullptr || (dci->MessageIndex - dciPrev->MessageIndex) == 1) {
                dciPrev = dci;
                ++continues;
            } else {
                if (continues > 3) {
                    flush = true;
                    //find beginning one
                    const QAttr*p = dci - continues;
                    confirms += ConfirmRange(p, (unsigned int) continues);
                } else if (continues) {
                    flush = true;
                    //find beginning one
                    const QAttr *p = dci - continues;
                    for (int j = 0; j < continues; ++j) {
                        confirms += (ConfirmDequeueInternal(p->MessagePos, p->MessageIndex, false) ? 1 : 0);
                        ++p;
                    }
                }
                dciPrev = dci;
                continues = 1;
            }
        }
        if (continues) {
            flush = true;
            //find beginning one
            const QAttr *p = dciPrev - (continues - 1);
            if (continues > 3) {
                confirms += ConfirmRange(p, (unsigned int) continues);
            } else {
                for (int j = 0; j < continues; ++j) {
                    confirms += (ConfirmDequeueInternal(p->MessagePos, p->MessageIndex, false) ? 1 : 0);
                    ++p;
                }
            }
        }
        return confirms;
    }

    unsigned int CMqFile::ConfirmRangeEx(const CDequeueConfirmInfo *start, unsigned int count, bool &flush) {
        unsigned int confirms = 0;
        int continues = 0;
        SPA::UINT64 prev_index = 0;
        const CDequeueConfirmInfo *dciPrev = nullptr;
        for (unsigned int n = 0; n < count; ++n) {
            const CDequeueConfirmInfo *dci = start + n;
            assert(dci->RequestId != SPA::idStartJob);
            assert(dci->RequestId != SPA::idEndJob);
            if (dci->Fail) {
                if (continues > 3) {
                    //find beginning one
                    flush = true;
                    const CDequeueConfirmInfo *p = dci - continues;
                    confirms += ConfirmRange(p, (unsigned int) continues);
                } else if (continues) {
                    //find beginning one
                    flush = true;
                    const CDequeueConfirmInfo *p = dci - continues;
                    for (int j = 0; j < continues; ++j) {
#ifndef NDEBUG
                        if (p->Fail) {
                            assert(false);
                        }
#endif	
                        confirms += (ConfirmDequeueInternal(p->QA.MessagePos, p->QA.MessageIndex, false) ? 1 : 0);
                        ++p;
                    }
                }
                continues = 0;
                confirms += (RemoveMsgIndex(dci->QA.MessagePos, dci->QA.MessageIndex) ? 1 : 0);
                m_bStrange = true;
#ifndef NDEBUG
                bool removed = RemoveConfirmed(dci->QA);
                assert(removed);
#endif
                if (dci->QA.MessagePos < (SPA::UINT64)m_CurrentReadPos) {
                    m_CurrentReadPos = dci->QA.MessagePos;
                }
                dciPrev = nullptr;
                continue;
            }
            if (dciPrev == nullptr || (dci->QA.MessageIndex - dciPrev->QA.MessageIndex) == 1) {
                dciPrev = dci;
                ++continues;
            } else {
                if (continues > 3) {
                    flush = true;
                    //find beginning one
                    const CDequeueConfirmInfo *p = dci - continues;
                    confirms += ConfirmRange(p, (unsigned int) continues);
                } else if (continues) {
                    flush = true;
                    //find beginning one
                    const CDequeueConfirmInfo *p = dci - continues;
                    for (int j = 0; j < continues; ++j) {
#ifndef NDEBUG
                        if (p->Fail) {
                            assert(false);
                        }
#endif	
                        confirms += (ConfirmDequeueInternal(p->QA.MessagePos, p->QA.MessageIndex, false) ? 1 : 0);
                        ++p;
                    }
                }
                dciPrev = dci;
                continues = 1;
            }
        }
        if (continues) {
            flush = true;
            //find beginning one
            const CDequeueConfirmInfo *p = dciPrev - (continues - 1);
            if (continues > 3) {
                confirms += ConfirmRange(p, (unsigned int) continues);
            } else {
                for (int j = 0; j < continues; ++j) {
#ifndef NDEBUG
                    if (p->Fail) {
                        assert(false);
                    }
#endif	
                    confirms += (ConfirmDequeueInternal(p->QA.MessagePos, p->QA.MessageIndex, false) ? 1 : 0);
                    ++p;
                }
            }
        }
        return confirms;
    }

    unsigned int CMqFile::ConfirmDequeueJob(const QAttr *qa, size_t count, bool fail) {
        size_t n;
        bool ok;
        size_t res;
        bool flush = false;
        if (!qa || !count)
            return 0;
        assert(count >= 2);
        unsigned int removed = 0;
#ifndef NDEBUG
        MessageDecriptionHeaderEx mdh;
#endif
        CAutoLock al(m_cs);
        if (!m_hFile)
            return 0;
        for (n = 0; n < count; ++n) {
            removed += (RemoveMsgIndex(qa[n].MessagePos, qa[n].MessageIndex) ? 1 : 0);
            if (fail) {
                m_bStrange = true;
#ifndef NDEBUG
                bool r = RemoveConfirmed(qa[n]);
                assert(r);
#endif
            }
        }
        if (!fail) {
            //ConfirmRange(pStartJob, jobs);
            assert(m_msgCount >= removed);
            //start

            ok = SetFilePointer(qa->MessagePos, SEEK_SET);
            assert(ok);
#ifndef NDEBUG
            res = fread(&mdh, 1, sizeof (mdh), m_hFile);
            assert(res == sizeof (mdh));
            if (mdh.MessageIndex != qa->MessageIndex) {
                std::cout << "---- Confirm range start: mdh.MessageIndex = " << mdh.MessageIndex << ", qa->MessageIndex = " << qa->MessageIndex << std::endl;
            }
            ok = SetFilePointer(qa->MessagePos, SEEK_SET);
            assert(ok);
#endif
            SPA::UINT64 index = (qa->MessageIndex | QAttr::RANGE_DEQUEUED_START);
            res = fwrite(&index, sizeof (index), 1, m_hFile);
            assert(res == 1);

            const QAttr *end = qa + (count - 1);
            ok = SetFilePointer(end->MessagePos, SEEK_SET);
            assert(ok);
#ifndef NDEBUG
            res = fread(&mdh, 1, sizeof (mdh), m_hFile);
            assert(res == sizeof (mdh));
            if (mdh.MessageIndex != end->MessageIndex) {
                std::cout << "---- Confirm range start: mdh.MessageIndex = " << mdh.MessageIndex << ", end->MessagePos = " << end->MessagePos << std::endl;
            }
            ok = SetFilePointer(end->MessagePos, SEEK_SET);
            assert(ok);
#endif
            index = (end->MessageIndex | QAttr::RANGE_DEQUEUED_END);
            res = fwrite(&index, sizeof (index), 1, m_hFile);
            assert(res == 1);
            m_msgCount -= removed;
            flush = true;
        } else {
            if (m_CurrentReadPos > (SPA::INT64)(qa->MessagePos)) {
                m_CurrentReadPos = (SPA::INT64)(qa->MessagePos);
            }
            if (m_bShared) {
                m_nInternalIndex += 3;
                for (n = 0; n < count; ++n) {
                    ok = SetFilePointer(qa[n].MessagePos, SEEK_SET);
                    assert(ok);
                    ++m_nInternalIndex;
                    res = fwrite(&m_nInternalIndex, sizeof (m_nInternalIndex), 1, m_hFile);
                    assert(res == 1);
                }
            }
            m_cv.notify_all();
        }
#ifndef NDEBUG
        if (m_nJobBalanceConfirm && m_msgCount == 0) {
            std::cout << "---- ConfirmDequeueJob: bad job confirm balance " << m_nJobBalanceConfirm << std::endl;
        }
#endif
        if ((m_bCrashSafe || m_msgCount == 0) && flush)
            fflush(m_hFile);
        if (m_msgCount == 0 && m_nFileSize > FILE_SIZE_TRUNCATED) {
            assert(m_qOut.GetSize() == 0);
            ResetInternal();
        }
        return removed;
    }

    bool CMqFile::ConfirmDequeue(const QAttr *qa, size_t count, bool fail) {
        bool ok;
        bool jumped = false;
        size_t res;
        if (qa == nullptr)
            count = 0;
        bool flush = false;
#ifndef NDEBUG
        MessageDecriptionHeaderEx mdh;
#endif
        SPA::UINT64 index = 0;
        CAutoLock al(m_cs);
        if (!m_hFile)
            return false;
        for (size_t n = 0; n < count; ++n) {
            bool found = RemoveMsgIndex(qa[n].MessagePos, qa[n].MessageIndex);
            if (!found) {
                //it happens possibly if a queue is reset
                //assert(false);
                return true;
            }
            if (fail) {
                m_bStrange = true;
#ifndef NDEBUG
                bool removed = RemoveConfirmed(qa[n]);
                assert(removed);
#endif
                if (qa[n].MessagePos < (SPA::UINT64)m_CurrentReadPos)
                    m_CurrentReadPos = (SPA::INT64) qa[n].MessagePos;
                if (m_bShared) {
                    if (!jumped) {
                        m_nInternalIndex += 3;
                        jumped = true;
                    }
                    ok = SetFilePointer(qa[n].MessagePos, SEEK_SET);
                    assert(ok);
                    ++m_nInternalIndex;
                    res = fwrite(&m_nInternalIndex, sizeof (m_nInternalIndex), 1, m_hFile);
                    assert(res == 1);
                }
                m_cv.notify_all();
            } else {
                assert(m_msgCount > 0);
                --m_msgCount;
                ok = SetFilePointer(qa[n].MessagePos, SEEK_SET);
                assert(ok);
#ifndef NDEBUG
                res = fread(&mdh, 1, sizeof (mdh), m_hFile);
                assert(res == sizeof (mdh));
                if (mdh.MessageIndex != qa[n].MessageIndex) {
                    std::cout << "---- ConfirmDequeue: mdh.MessageIndex = " << mdh.MessageIndex << ", qa[n].MessageIndex = " << qa[n].MessageIndex << std::endl;
                }
                switch (mdh.RequestId) {
                    case SPA::idStartJob:
                        ++m_nJobBalanceConfirm;
                        break;
                    case SPA::idEndJob:
                        --m_nJobBalanceConfirm;
                        break;
                    default:
                        break;
                }
                if (m_nJobBalanceConfirm > 1 || m_nJobBalanceConfirm < 0) {
                    std::cout << "---- ConfirmDequeue: bad job confirm balance " << m_nJobBalanceConfirm << std::endl;
                }
                ok = SetFilePointer(qa[n].MessagePos, SEEK_SET);
                assert(ok);
#endif
                res = fwrite(&index, sizeof (index), 1, m_hFile);
                assert(res == 1);
                flush = true;
            }
        }
#ifndef NDEBUG
        if (m_nJobBalanceConfirm && m_msgCount == 0) {
            std::cout << "---- ConfirmDequeue: bad job confirm balance " << m_nJobBalanceConfirm << std::endl;
        }
#endif
        if ((m_bCrashSafe || m_msgCount == 0) && flush)
            fflush(m_hFile);
        if (m_msgCount == 0 && m_nFileSize > FILE_SIZE_TRUNCATED) {
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
#ifndef NDEBUG
        MessageDecriptionHeaderEx mdh;
#endif
        bool found = RemoveMsgIndex(pos, index);
        if (!found) {
            //it happens possibly if a queue is reset
            //assert(false);
            return true;
        }
        if (fail) {
            m_bStrange = true;
#ifndef NDEBUG
            QAttr qa(pos, index);
            bool removed = RemoveConfirmed(qa);
            assert(removed);
#endif
            if (pos < (SPA::UINT64)m_CurrentReadPos)
                m_CurrentReadPos = (SPA::INT64) pos;
            if (m_bShared) {
                ok = SetFilePointer(pos, SEEK_SET);
                assert(ok);
                m_nInternalIndex += 4;
                res = fwrite(&m_nInternalIndex, sizeof (m_nInternalIndex), 1, m_hFile);
                assert(res == 1);
            }
            m_cv.notify_all();
        } else {
            assert(m_msgCount > 0);
            ok = SetFilePointer(pos, SEEK_SET);
            assert(ok);
#ifndef NDEBUG
            res = fread(&mdh, 1, sizeof (mdh), m_hFile);
            assert(res == sizeof (mdh));
            if (mdh.MessageIndex != index) {
                std::cout << "---- ConfirmDequeueInternal: mdh.MessageIndex = " << mdh.MessageIndex << ", index = " << index << std::endl;
            }
            switch (mdh.RequestId) {
                case SPA::idStartJob:
                    ++m_nJobBalanceConfirm;
                    break;
                case SPA::idEndJob:
                    --m_nJobBalanceConfirm;
                    break;
                default:
                    break;
            }
            if (m_nJobBalanceConfirm > 1 || m_nJobBalanceConfirm < 0) {
                std::cout << "---- ConfirmDequeueInternal: bad job confirm balance " << m_nJobBalanceConfirm << std::endl;
            }
            ok = SetFilePointer(pos, SEEK_SET);
            assert(ok);
#endif
            index = 0;
            res = fwrite(&index, sizeof (index), 1, m_hFile);
            assert(res == 1);
            flush = true;
            --m_msgCount;
        }
#ifndef NDEBUG
        if (m_nJobBalanceConfirm && m_msgCount == 0) {
            std::cout << "---- ConfirmDequeueInternal: bad job confirm balance " << m_nJobBalanceConfirm << std::endl;
        }
#endif
        if ((m_bCrashSafe || m_msgCount == 0) && flush)
            fflush(m_hFile);
        if (m_msgCount == 0 && m_nFileSize > FILE_SIZE_TRUNCATED) {
            assert(m_qOut.GetSize() == 0);
            ResetInternal();
        }
        return true;
    }

    unsigned int CMqFile::ConfirmRange(const QAttr *start, unsigned int count) {
        size_t res;
        SPA::UINT64 index;
        bool ok;
        assert(count >= 2);
        assert(m_msgCount >= count);
        unsigned int removed = RemoveMsgIndex(start, count);

#ifndef NDEBUG
        MessageDecriptionHeaderEx mdh;
#endif
        const QAttr *end = start + (count - 1);

        //start
        ok = SetFilePointer(start->MessagePos, SEEK_SET);
        assert(ok);
#ifndef NDEBUG
        res = fread(&mdh, 1, sizeof (mdh), m_hFile);
        assert(res == sizeof (mdh));
        if (mdh.MessageIndex != start->MessageIndex) {
            std::cout << "---- Confirm range start: start->MessageIndex = " << mdh.MessageIndex << ", start->MessageIndex = " << start->MessageIndex << std::endl;
        }
        ok = SetFilePointer(start->MessagePos, SEEK_SET);
        assert(ok);
#endif
        index = (start->MessageIndex | QAttr::RANGE_DEQUEUED_START);
        res = fwrite(&index, sizeof (index), 1, m_hFile);
        assert(res == 1);

        ok = SetFilePointer(end->MessagePos, SEEK_SET);
        assert(ok);
#ifndef NDEBUG
        res = fread(&mdh, 1, sizeof (mdh), m_hFile);
        assert(res == sizeof (mdh));
        if (mdh.MessageIndex != end->MessageIndex) {
            std::cout << "---- Confirm range start: end->MessageIndex = " << mdh.MessageIndex << ", end->MessagePos = " << end->MessagePos << std::endl;
        }
        ok = SetFilePointer(end->MessagePos, SEEK_SET);
        assert(ok);
#endif
        index = (end->MessageIndex | QAttr::RANGE_DEQUEUED_END);
        res = fwrite(&index, sizeof (index), 1, m_hFile);
        assert(res == 1);
        m_msgCount -= removed;
        return removed;
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

    unsigned int CMqFile::RemoveMsgIndex(const QAttr *qa, unsigned int size) {
        if (!qa)
            size = 0;
        unsigned int removed = 0;
        unsigned int n, Count = m_qTransIndex.GetSize() / sizeof (SPA::UINT64);
        SPA::UINT64 *pIndex = (SPA::UINT64 *)m_qTransIndex.GetBuffer();
        for (n = 0; n < Count; ++n) {
            if (pIndex[n] == qa->MessageIndex) {
                assert((Count - n) >= size);
#ifndef NDEBUG
                for (unsigned int j = 0; j < size; ++j) {
                    assert(qa[j].MessageIndex == pIndex[n + j]);
                }
#endif
                m_qTransIndex.Pop((unsigned int) (sizeof (SPA::UINT64) * size), (unsigned int) (n * sizeof (SPA::UINT64)));
                break;
            }
        }
        Count = m_qOut.GetSize() / sizeof (QAttr);
        const QAttr *qaOut = (const QAttr *) m_qOut.GetBuffer();
        if (m_bStrange) {
            for (n = 0; n < size; ++n) {
                removed += (RemoveMsgIndex(qa[n].MessagePos, qa[n].MessageIndex) ? 1 : 0);
            }
            assert(removed == size);
        } else {
            for (n = 0; n < Count; ++n) {
                if (qaOut[n].MessageIndex == qa->MessageIndex && qaOut[n].MessagePos == qa->MessagePos) {
                    assert((Count - n) >= size);
#ifndef NDEBUG
                    for (unsigned int j = 0; j < size; ++j) {
                        assert(qa[j].MessageIndex == qaOut[n + j].MessageIndex);
                        assert(qa[j].MessagePos == qaOut[n + j].MessagePos);
                    }
#endif
                    removed = size;
                    m_qOut.Pop((unsigned int) (sizeof (QAttr) * size), (unsigned int) n * sizeof (QAttr));
                    break;
                }
            }
        }
        return removed;
    }

    unsigned int CMqFile::RemoveMsgIndex(const CDequeueConfirmInfo *start, unsigned int size) {
        if (!start)
            size = 0;
        unsigned int removed = 0;
        unsigned int n, Count = m_qTransIndex.GetSize() / sizeof (SPA::UINT64);
        SPA::UINT64 *pIndex = (SPA::UINT64 *)m_qTransIndex.GetBuffer();
        for (n = 0; n < Count; ++n) {
            if (pIndex[n] == start->QA.MessageIndex) {
                assert((Count - n) >= size);
#ifndef NDEBUG
                for (unsigned int j = 0; j < size; ++j) {
                    assert(start[j].QA.MessageIndex == pIndex[n + j]);
                }
#endif
                m_qTransIndex.Pop((unsigned int) (sizeof (SPA::UINT64) * size), (unsigned int) (n * sizeof (SPA::UINT64)));
                break;
            }
        }
        Count = m_qOut.GetSize() / sizeof (QAttr);
        const QAttr *qaOut = (const QAttr *) m_qOut.GetBuffer();
        if (m_bStrange) {
            for (n = 0; n < size; ++n) {
                removed += (RemoveMsgIndex(start[n].QA.MessagePos, start[n].QA.MessageIndex) ? 1 : 0);
            }
            assert(removed == size);
        } else {
            for (n = 0; n < Count; ++n) {
                if (qaOut[n].MessageIndex == start->QA.MessageIndex && qaOut[n].MessagePos == start->QA.MessagePos) {
#ifndef NDEBUG
                    assert((Count - n) >= size);
                    for (unsigned int j = 0; j < size; ++j) {
                        assert(start[j].QA.MessageIndex == qaOut[n + j].MessageIndex);
                        assert(start[j].QA.MessagePos == qaOut[n + j].MessagePos);
                    }
#endif
                    removed = size;
                    m_qOut.Pop((unsigned int) (sizeof (QAttr) * size), (unsigned int) n * sizeof (QAttr));
                    break;
                }
            }
        }
        return removed;
    }

    unsigned int CMqFile::ConfirmRange(const CDequeueConfirmInfo *start, unsigned int count) {
        size_t res;
        SPA::UINT64 index;
        bool ok;
        assert(count >= 2);
#ifndef NDEBUG
        if (m_msgCount < count) {
            assert(false);
        }
#endif
        unsigned int removed = RemoveMsgIndex(start, count);
#ifndef NDEBUG
        MessageDecriptionHeaderEx mdh;
        for (unsigned int n = 0; n < count; ++n) {
            if (start[n].Fail) {
                assert(false);
            }
        }
#endif
        const CDequeueConfirmInfo *end = start + (count - 1);

        //start
        ok = SetFilePointer(start->QA.MessagePos, SEEK_SET);
        assert(ok);
#ifndef NDEBUG
        res = fread(&mdh, 1, sizeof (mdh), m_hFile);
        assert(res == sizeof (mdh));
        if (mdh.MessageIndex != start->QA.MessageIndex) {
            std::cout << "---- Confirm range start: mdh.MessageIndex = " << mdh.MessageIndex << ", start->QA.MessageIndex = " << start->QA.MessageIndex << std::endl;
        }
        ok = SetFilePointer(start->QA.MessagePos, SEEK_SET);
        assert(ok);
#endif
        index = (start->QA.MessageIndex | QAttr::RANGE_DEQUEUED_START);
        res = fwrite(&index, sizeof (index), 1, m_hFile);
        assert(res == 1);

        ok = SetFilePointer(end->QA.MessagePos, SEEK_SET);
        assert(ok);
#ifndef NDEBUG
        res = fread(&mdh, 1, sizeof (mdh), m_hFile);
        assert(res == sizeof (mdh));
        if (mdh.MessageIndex != end->QA.MessageIndex) {
            std::cout << "---- Confirm range start: mdh.MessageIndex = " << mdh.MessageIndex << ", end->QA.MessagePos = " << end->QA.MessagePos << std::endl;
        }
        ok = SetFilePointer(end->QA.MessagePos, SEEK_SET);
        assert(ok);
#endif
        index = (end->QA.MessageIndex | QAttr::RANGE_DEQUEUED_END);
        res = fwrite(&index, sizeof (index), 1, m_hFile);
        assert(res == 1);
        m_msgCount -= removed;
        return removed;
    }

    vector<unsigned int> CMqFile::DoBatchDequeue(unsigned int requestCount, SPA::CUQueue &qAttr, SPA::CUQueue &qRequests, unsigned int waitTime) {
        size_t read;
        SPA::INT64 fileSize;
        bool again = false;
        unsigned int range = 0;
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
        vMdh.reserve(64);
        do {
            fileSize = m_nFileSize;
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
                if (range || mdh.MessageIndex == 0 || mdh.MessageIndex >= QAttr::RANGE_DEQUEUED_END || IsInDequeuing(mdh.MessageIndex)) {
                    if (mdh.Len) {
                        ok = SetFilePointer(mdh.Len, SEEK_CUR);
                        assert(ok);
                    }
                    m_CurrentReadPos += (sizeof (mdh) + mdh.Len);
                    if ((mdh.MessageIndex & QAttr::RANGE_DEQUEUED_START) == QAttr::RANGE_DEQUEUED_START) {
#ifndef NDEBUG
                        if (range) {
                            std::cout << __FUNCTION__ << ", bad range balance = " << range << std::endl;
                        }
#endif
                        ++range;
                    } else if ((mdh.MessageIndex & QAttr::RANGE_DEQUEUED_END) == QAttr::RANGE_DEQUEUED_END) {
#ifndef NDEBUG
                        if (!range) {
                            std::cout << "==== Bad range balance: " << range << " @" << __FUNCTION__ << " @line " << __LINE__ << std::endl;
                        }
#endif
                        --range;
                    }
                } else {
                    attri.MessageIndex = mdh.MessageIndex;
                    attri.MessagePos = (SPA::UINT64)m_CurrentReadPos;
#ifndef NDEBUG
                    bool found = FindConfirmed(attri);
                    if (found) {
                        assert(false);
                    }
                    m_sbConfirmed << attri;
#endif
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
                            assert(attri.MessageIndex < QAttr::RANGE_DEQUEUED_END);
                            m_qTransIndex << attri.MessageIndex;
                            break;
                        default:
                            break;
                    }
#ifndef NDEBUG
                    if (sh->RequestId == SPA::idStartJob) {
                        ++m_nJobBalanceDequeue;
                    } else if (sh->RequestId == SPA::idEndJob) {
                        --m_nJobBalanceDequeue;
                    }
                    if (m_nJobBalanceDequeue > 1 || m_nJobBalanceDequeue < 0) {
                        std::cout << __FUNCTION__ << ", bad job balance = " << m_nJobBalanceDequeue << std::endl;
                    }
#endif
                    qRequests.SetSize(qRequests.GetSize() + mdh.Len);
                    m_CurrentReadPos += (sizeof (mdh) + mdh.Len);
                    m_qOut << attri;
                    if ((m_qTransIndex.GetSize() % (2 * sizeof (SPA::UINT64))) == 0 && vMdh.size() >= requestCount)
                        break;
                    ++index;
                }
            }
            if (!vMdh.size() && waitTime && !again) {
#ifndef WINCE
                m_cv.wait_for(al, ms(waitTime));
#else
                boost::system_time td = boost::get_system_time() + boost::posix_time::milliseconds(waitTime);
                m_cv.timed_wait(al, td);
#endif
                again = true;
            } else
                again = false;
        } while (again);
        return vMdh;
    }

#ifndef NDEBUG

    bool CMqFile::FindConfirmed(const QAttr &qa) {
        unsigned int count = m_sbConfirmed->GetSize() / sizeof (QAttr);
        const QAttr *start = (const QAttr *) m_sbConfirmed->GetBuffer();
        for (unsigned int n = 0; n < count; ++n, ++start) {
            if (start->MessageIndex == qa.MessageIndex) {
                assert(start->MessagePos == qa.MessagePos);
                return true;
            }
        }
        return false;
    }

    bool CMqFile::RemoveConfirmed(const QAttr &qa) {
        unsigned int count = m_sbConfirmed->GetSize() / sizeof (QAttr);
        const QAttr *start = (const QAttr *) m_sbConfirmed->GetBuffer();
        for (unsigned int n = 0; n < count; ++n, ++start) {
            if (start->MessageIndex == qa.MessageIndex) {
                assert(start->MessagePos == qa.MessagePos);
                m_sbConfirmed->Pop((unsigned int) sizeof (QAttr), (unsigned int) (n * sizeof (QAttr)));
                return true;
            }
        }
        return false;
    }
#endif

    std::vector<unsigned int> CMqFile::DoBatchDequeue(SPA::CUQueue &qAttr, SPA::CUQueue &qRequests, unsigned int maxSizeInByte, unsigned int waitTime) {
        CAutoLock al(m_cs);
#ifndef NDEBUG
        std::vector<unsigned int> v = DoBatchDequeueInternal(al, qAttr, qRequests, maxSizeInByte, waitTime);
        m_sbConfirmed->Push(qAttr.GetBuffer(), qAttr.GetSize());
        return v;
#else
        return DoBatchDequeueInternal(al, qAttr, qRequests, maxSizeInByte, waitTime);
#endif
    }

    std::vector<unsigned int> CMqFile::DoBatchDequeueInternal(CAutoLock &al, SPA::CUQueue &qAttr, SPA::CUQueue &qRequests, unsigned int maxSize, unsigned int waitTime) {
        size_t read;
        SPA::INT64 fileSize;
        bool again = false;
        unsigned int range = 0;
        QAttr attri;
        MessageDecriptionHeader mdh;
        qAttr.SetSize(0);
        qRequests.SetSize(0);
        if (qRequests.GetMaxSize() < maxSize)
            qRequests.ReallocBuffer(maxSize);
        vector<unsigned int> vMdh;
        if (!m_hFile || !m_bEnableDequeue)
            return vMdh;
        vMdh.reserve(64);
        do {
            fileSize = m_nFileSize;
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
                if (range || mdh.MessageIndex == 0 || mdh.MessageIndex >= QAttr::RANGE_DEQUEUED_END || IsInDequeuing(mdh.MessageIndex)) {
                    if (mdh.Len) {
                        ok = SetFilePointer(mdh.Len, SEEK_CUR);
                        assert(ok);
                    }
                    m_CurrentReadPos += (sizeof (mdh) + mdh.Len);
                    if ((mdh.MessageIndex & QAttr::RANGE_DEQUEUED_START) == QAttr::RANGE_DEQUEUED_START) {
#ifndef NDEBUG
                        if (range) {
                            std::cout << __FUNCTION__ << ", bad range balance = " << range << std::endl;
                        }
#endif
                        ++range;
                    } else if ((mdh.MessageIndex & QAttr::RANGE_DEQUEUED_END) == QAttr::RANGE_DEQUEUED_END) {
#ifndef NDEBUG
                        if (!range) {
                            std::cout << "==== Bad range balance: " << range << " @" << __FUNCTION__ << " @line " << __LINE__ << std::endl;
                        }
#endif
                        --range;
                    }
                } else {
                    attri.MessageIndex = mdh.MessageIndex;
                    attri.MessagePos = (SPA::UINT64)m_CurrentReadPos;
#ifndef NDEBUG
                    bool found = FindConfirmed(attri);
                    if (found) {
                        assert(false);
                    }
#endif
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
                            assert(attri.MessageIndex < QAttr::RANGE_DEQUEUED_END);
                            m_qTransIndex << attri.MessageIndex;
                            break;
                        default:
                            break;
                    }
#ifndef NDEBUG
                    if (sh->RequestId == SPA::idStartJob) {
                        ++m_nJobBalanceDequeue;
                    } else if (sh->RequestId == SPA::idEndJob) {
                        --m_nJobBalanceDequeue;
                    }
                    if (m_nJobBalanceDequeue > 1 || m_nJobBalanceDequeue < 0) {
                        std::cout << __FUNCTION__ << ", bad job dequeue balance = " << m_nJobBalanceDequeue << std::endl;
                    }
#endif
                    qRequests.SetSize(qRequests.GetSize() + mdh.Len);
                    m_CurrentReadPos += (sizeof (mdh) + mdh.Len);
                    m_qOut << attri;
                    if ((m_qTransIndex.GetSize() % (2 * sizeof (SPA::UINT64))) == 0 && qRequests.GetSize() >= maxSize)
                        break;
                }
            }
            if (!vMdh.size() && waitTime && !again) {
#ifndef WINCE
                m_cv.wait_for(al, ms(waitTime));
#else
                boost::system_time td = boost::get_system_time() + boost::posix_time::milliseconds(waitTime);
                m_cv.timed_wait(al, td);
#endif
                again = true;
            } else
                again = false;
        } while (again);
        return vMdh;
    }

    SPA::UINT64 CMqFile::Dequeue(SPA::CUQueue &q, SPA::UINT64 &mqIndex, unsigned int waitTime) {
        size_t read;
        unsigned int range = 0;
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
            fileSize = m_nFileSize;
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
                if (range || mdh.MessageIndex == 0 || mdh.MessageIndex >= QAttr::RANGE_DEQUEUED_END || IsInDequeuing(mdh.MessageIndex)) {
                    if (mdh.Len) {
                        ok = SetFilePointer(mdh.Len, SEEK_CUR);
                        assert(ok);
                    }
                    m_CurrentReadPos += (sizeof (mdh) + mdh.Len);
                    if ((mdh.MessageIndex & QAttr::RANGE_DEQUEUED_START) == QAttr::RANGE_DEQUEUED_START) {
#ifndef NDEBUG
                        if (range) {
                            std::cout << __FUNCTION__ << ", bad range balance = " << range << std::endl;
                        }
#endif
                        ++range;
                    } else if ((mdh.MessageIndex & QAttr::RANGE_DEQUEUED_END) == QAttr::RANGE_DEQUEUED_END) {
#ifndef NDEBUG
                        if (!range) {
                            std::cout << "==== Bad range balance: " << range << " @" << __FUNCTION__ << " @line " << __LINE__ << std::endl;
                        }
#endif
                        --range;
                    }
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
#ifndef NDEBUG
                    if (sh->RequestId == SPA::idStartJob) {
                        ++m_nJobBalanceDequeue;
                    } else if (sh->RequestId == SPA::idEndJob) {
                        --m_nJobBalanceDequeue;
                    }
                    if (m_nJobBalanceDequeue > 1 || m_nJobBalanceDequeue < 0) {
                        std::cout << __FUNCTION__ << ", bad job dequeue balance = " << m_nJobBalanceDequeue << std::endl;
                    }
#endif
                    q.SetSize(mdh.Len);
                    m_CurrentReadPos += (sizeof (mdh) + mdh.Len);
                    QAttr qa(pos, mqIndex);
                    m_qOut << qa;
                    break;
                }
            }
            if (pos == INVALID_NUMBER && waitTime && !again) {
#ifndef WINCE
                m_cv.wait_for(al, ms(waitTime));
#else
                boost::system_time td = boost::get_system_time() + boost::posix_time::milliseconds(waitTime);
                m_cv.timed_wait(al, td);
#endif
                again = true;
            } else
                again = false;
        } while (again);
#ifndef NDEBUG
        QAttr qa(pos, mqIndex);
        bool found = FindConfirmed(qa);
        if (found) {
            assert(false);
        }
#endif
        return pos;
    }

    bool CMqFile::IsAvailable() {
        //CAutoLock al(m_cs); //remove this lock because queue is locked for starting or stopping at caller's level
        return (nullptr != m_hFile);
    }

    void CMqFile::StopQueue(SPA::tagQueueStatus qs) {
        {
            CAutoLock al(m_cs);
            if (m_hFile) {
                fflush(m_hFile);
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
#ifndef NDEBUG
            if (pStreamHeader->RequestId == SPA::idStartJob) {
                ++m_nJobBalanceEnqueue;
            } else if (pStreamHeader->RequestId == SPA::idEndJob) {
                --m_nJobBalanceEnqueue;
            }
            if (m_nJobBalanceEnqueue > 1 || m_nJobBalanceEnqueue < 0) {
                std::cout << __FUNCTION__ << ", bad job enqueue balance = " << m_nJobBalanceEnqueue << std::endl;
            }
#endif
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
            m_nFileSize += sizeof (mdh);

            written = fwrite(pStreamHeader, sizeof (SPA::CStreamHeader), 1, m_hFile); //8 bytes
            assert(written == 1);
            m_nFileSize += sizeof (SPA::CStreamHeader);

            if (pStreamHeader->Size) {
                if (m_bSecure) {
                    written = fwrite(q->GetBuffer(), q->GetSize(), 1, m_hFile);
                    m_nFileSize += q->GetSize();
                    SPA::CScopeUQueue::Unlock(q);
                    q = nullptr;
                } else {
                    written = fwrite(pBuffer, pStreamHeader->Size, 1, m_hFile);
                    m_nFileSize += pStreamHeader->Size;
                }
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

    SPA::UINT64 CMqFile::BatchEnqueue(unsigned int count, const unsigned char *messages) {
        if (!messages) {
            count = 0;
        }
        SPA::CStreamHeader sh;
        CAutoLock al(m_cs);
        SPA::UINT64 index = m_nInternalIndex;
        for (unsigned int n = 0; n < count; ++n) {
            sh.RequestId = *((unsigned short*) messages);
            messages += sizeof (unsigned short);
            sh.Size = *((unsigned int*) messages);
            messages += sizeof (unsigned int);
            index = EnqueueInternal(sh, messages, sh.Size);
            messages += sh.Size;
        }
        return index;
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
#ifndef NDEBUG
        --m_nJobBalanceEnqueue;
#endif
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
        if (buffer == nullptr)
            size = 0;
        MessageDecriptionHeader mdh;
        mdh.Len = size + sizeof (sh);
        std::time_t rawtime = m_start_time + (size_t)((GetTimeTick() - m_startTick) / 1000);
        mdh.Time = static_cast<unsigned int> (rawtime - TIME_Y_2013);
        if (!m_hFile)
            return INVALID_NUMBER;

        //for queue trans/commit
        if (sh.RequestId == SPA::idStartJob) {
            if (m_qTransPos != INVALID_NUMBER) {
                return INVALID_NUMBER;
            }
#ifndef NDEBUG
            ++m_nJobBalanceEnqueue;
#endif
            m_qTransPos = m_nFileSize;
            assert(m_msgTransCount == 0);
            ++m_msgTransCount;
        } else if (sh.RequestId == SPA::idEndJob) {
            if (m_qTransPos == INVALID_NUMBER) {
                return INVALID_NUMBER;
            }
#ifndef NDEBUG
            --m_nJobBalanceEnqueue;
#endif
        } else if (m_qTransPos != INVALID_NUMBER) {
            ++m_msgTransCount;
        }

#ifndef NDEBUG
        if (m_nJobBalanceEnqueue > 1 || m_nJobBalanceEnqueue < 0) {
            std::cout << __FUNCTION__ << ", bad job enqueue balance = " << m_nJobBalanceEnqueue << std::endl;
        }
#endif
        if (m_qTransPos == INVALID_NUMBER)
            m_LastTime = mdh.Time;
        if (!m_bEnd) {
            bool ok = SetFilePointer(0, SEEK_END);
            assert(ok);
            m_bEnd = true;
        }
        mdh.MessageIndex = ++m_nInternalIndex;
        size_t written = fwrite(&mdh, sizeof (mdh), 1, m_hFile); //16 bytes
        assert(written == 1);
        m_nFileSize += sizeof (mdh);

        written = fwrite(&sh, sizeof (SPA::CStreamHeader), 1, m_hFile); //8 bytes
        assert(written == 1);
        m_nFileSize += sizeof (SPA::CStreamHeader);

        if (size) {
            written = fwrite(buffer, size, 1, m_hFile);
            assert(written == 1);
            m_nFileSize += size;
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
            if (qFile && qFile->IsAvailable()) {
                qFile->SetOptimistic(SPA::oSystemMemoryCached);
                continue;
            }
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
        fflush(m_hFile);
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
#ifndef WINCE
                sleep_for(ms(1));
#else
                boost::system_time td = boost::get_system_time() + boost::posix_time::milliseconds(1);
                sleep(td);
#endif
            } while (true);
            attr.MessagePos = qFile->m_nFileSize;
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
            attr.MessagePos = qFile->m_nFileSize - sizeof (MessageDecriptionHeaderEx);
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

    CMqFileEx::CMqFileEx(const char *fileName, unsigned int ttl, SPA::tagOptimistic crashSafe, const wchar_t *userId, const wchar_t *password, CQLastIndex *pLastIndex, bool client, bool shared)
    : CMqFile(fileName, ttl, crashSafe, true, client, shared) {

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

        unsigned char bytes[128] = {0};
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
#ifndef WINCE
        m_thread = new std::thread(&CQLastIndex::DoFastSave, this);
        sleep_for(ms(50));
#else
        m_thread = m_tg.create_thread(boost::bind(&CQLastIndex::DoFastSave, this));
        //make sure the thread is already running
        boost::system_time td = boost::get_system_time() + boost::posix_time::milliseconds(50);
        sleep(td);
#endif
        CAutoLock al(g_csQFile);
        g_vQLastIndex.push_back(this);
    }

    CQLastIndex::~CQLastIndex() {
        Stop();
    }

    void CQLastIndex::Stop() {
        try {
            CAutoLock al(g_csQFile);
            if (!m_stop) {
                m_stop = 1;
                try {
                    std::vector<CQLastIndex*>::iterator it = g_vQLastIndex.end();
                    if (g_vQLastIndex.size())
                        it = std::find(g_vQLastIndex.begin(), g_vQLastIndex.end(), this);
                    if (it != g_vQLastIndex.end()) {
                        g_vQLastIndex.erase(it);
                    }
                } catch (...) {
                }
                m_cv.notify_one();
                if (m_thread->joinable()) {
                    m_thread->join();
                }
                Save();
                CloseFile();
                //std::cout << "Stopped" << std::endl;
            }
        } catch (...) {

        }
    }

    void CQLastIndex::DoFastSave() {
        bool waited;
        const unsigned int LONG_WAIT_TIME = 200;
        const unsigned int SHORT_WAIT_TIME = 0;
        unsigned int waitTime = LONG_WAIT_TIME;
        do {
            {
#ifndef WINCE
                CAutoLock al(m_cs);
                waited = (m_cv.wait_for(al, ms(waitTime)) == std::cv_status::no_timeout);
#else
                boost::system_time td = boost::get_system_time() + boost::posix_time::milliseconds(waitTime);
                CAutoLock al(m_cs);
                waited = m_cv.timed_wait(al, td);
#endif
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
#ifndef WINCE
            yield();
#else
            yield();
#endif
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
            if (m_hFile == nullptr)
                return false;
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
#if defined(WINCE) || defined(__ANDROID__) || defined(ANDROID)
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
        QAttr qa(INVALID_NUMBER, INVALID_NUMBER);
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
        SPA::CScopeUQueue su;
        {
            CAutoLock al(m_cs);
            if (m_objp.find(src) == m_objp.end())
                return "";
            std::vector<unsigned char> &buffer = m_objp[src];
#ifdef WINCE
            su->Push(&buffer.front(), (unsigned int) buffer.size());
#else
            su->Push(buffer.data(), (unsigned int) buffer.size());
#endif
            m_bf->Decrypt((unsigned char*) su->GetBuffer(), buffer.size());
        }
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
