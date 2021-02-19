
//#include <exception>﻿
// msqueue.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "../../ucomm/core_shared/pinc/mqfile.h"
#include <iostream>
#include <assert.h>
#include <time.h>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "../../ucomm/core_shared/pinc/getsysid.h"
#include "../../ucomm/core_shared/pinc/uzip.h"
#include <fstream>
#include <deque>
#include <boost/algorithm/string.hpp>
#if defined(__ANDROID__) || defined(ANDROID)
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#endif

#ifdef WIN32_64
#include <windows.h>
#else

#endif

using namespace std;
#if defined(__ANDROID__) || defined(ANDROID)
const int ITEM_COUNT = 1024;
#elif defined(WINCE)
const int ITEM_COUNT = 1024;
#else
const int ITEM_COUNT = 2000000;
#endif
const int Enq_Thread_Count = 2;
const int Deq_Thread_Count = 4;

SPA::CUCriticalSection g_cs;

const char *sample_str = "SocketPro -- a package of revolutionary software components written from batching, asynchrony and parallel computation with many unique and critical features. These features assist you to quickly develop high speed and scalable distributed applications on Windows and smart devices as well as web browsers."
        "Tutorial One -- The first tutorial is a hello project to support five simple requests. One of them is a slow request processed with a worker thread at the server side. The tutorial tells you how to code step-by-step at both server and client sides based on the classes of SocketProAdapter and its sub namespaces. The tutorial "
        "leads you to do two experiments that require you to use the well known tool Telnet for connecting from a client to a remote SocketPro server. The tutorial tells you SocketPro threads management at server side. It tells you what variables should be synchronized with a critical section or monitor. For client development, the "
        "tutorial is focused on how to use SocketProAdapter at client side, how to turn on/off online compressing, how to batch requests, how to do asynchrony computation, how to do synchrony computation, and how to switch between the two computation models. This tutorial leads you to do an particular experiment, freezing and de-freezing Window GUIs at run time.";

//const char *sample_str = "TEST ME FROM CHARLIE YE BEGIN_STORAGE_RETRY_BLOCK";

long Deq_Count = 0;

int GetRandom() {
#ifdef WINCE
    return 0;
#else
    ::srand((unsigned) time(nullptr));
    return (rand() % 10);
#endif
}

MQ_FILE::CMqFile *g_mqFile = nullptr;

int EnqThreadProc();
int DeqThreadProc();
int BatchDeqThreadProc();
int MaxDeqThreadProc();
void TestRegistration();
void TestDateTimeSerialization();
void TestWStringLength();
void TestZip();
MQ_FILE::CQueueInitialInfo TestQueue();
void TestQLastIndex();
void TestSysIdAndAppName();
void TestReplication();
void TestMyContainer();
std::string GetManagedAppCmd();

std::string GetJavaCmd(const std::string &all) {
    size_t move = 12;
    size_t pos = all.find(" -classpath ");
    if (pos == std::string::npos) {
        move = 4;
        pos = all.find_first_of(" -cp ");
    }
    if (pos == std::string::npos)
        return "";
    std::string str = all.substr(pos + move);
    pos = str.find(" -");
    if (pos != std::string::npos)
        str = str.substr(0, pos);
    boost::algorithm::trim(str);
    pos = str.rfind(' ');
    str = str.substr(pos + 1);
    boost::algorithm::trim(str);
    return str;
}

void TestEnqueue();
void TestEnqueue2();
void TestEnqueue3();
void TestSimpleEnqueue(const char *str);

#if defined(__ANDROID__) || defined(ANDROID)
int debug_test() {
#elif defined(WINCE)
int _tmain(int argc, _TCHAR* argv[]) {
#else
int main(int argc, char* argv[]) {
#endif
    int n;
	for (n = 0; n < 10; ++n) {
		TestSysIdAndAppName();
	}
	TestMyContainer();
	TestDateTimeSerialization();
	TestWStringLength();
	TestZip();

    TestEnqueue();
    TestEnqueue2();
    //TestEnqueue3();
    std::string cmdName = GetManagedAppCmd();
    std::string sampleCmd = "java.exe -cp C:\\chaohu\\dev\\SocketProRoot\\bin\\java\\jspa.jar;C:\\chaohu\\dev\\SocketProRoot\\tutorials\\java\\build\\classes hello_world.client.Program -Dfile.encoding=UTF-8";
    TestSimpleEnqueue(sampleCmd.c_str());
    std::string javaCmd = GetJavaCmd(sampleCmd);
    TestReplication();
#if defined(__ANDROID__) || defined(ANDROID)

#else
    TestRegistration();
#endif
    MQ_FILE::CQueueInitialInfo qii = TestQueue();
    TestQLastIndex();
    std::cout << "Input a number and kill the app ......" << std::endl;
    cin >> n;
    return 0;
}

void TestSimpleEnqueue(const char *str) {
    bool ok;
    unsigned int n;
    static const unsigned int TEST_CYCLES = 24;
    SPA::UINT64 pos;
    SPA::UINT64 index;
    SPA::CScopeUQueue su;
    SPA::CUQueue &q = *su;
#if defined(__ANDROID__) || defined(ANDROID)
	MQ_FILE::CMqFile mqFile("/data/data/com.android_native_test/files/test_simple_enq", 30 * 24 * 3600, SPA::oMemoryCached, false);
#else
    MQ_FILE::CMqFile mqFile("test_simple_enq", 30 * 24 * 3600, SPA::tagOptimistic::oMemoryCached, false);
#endif
    SPA::CStreamHeader sh;
    sh.RequestId = 10240;
    sh.Size = (unsigned int) strlen(str);
    for (n = 0; n < TEST_CYCLES; ++n) {
        mqFile.Enqueue(sh, (const unsigned char*) str, sh.Size);
    }
    pos = mqFile.Dequeue(q, index, 0);
    while (pos != INVALID_NUMBER) {
        ok = mqFile.ConfirmDequeue(pos, index, false);
        pos = mqFile.Dequeue(q, index, 0);
    }
}

void TestMyContainer() {
    SPA::UINT64 src = 10;
    std::string pwd("test@psw");
    MQ_FILE::CMyContainer::Container.Set(src, pwd.c_str());
    std::string res = MQ_FILE::CMyContainer::Container.Get(src);
    assert(res == pwd);
}

void TestReplication() {
    bool ok;
    int n;
#if defined(WINCE) || defined(__ANDROID__) || defined(ANDROID)
    int repeats = 8 * 16;
#else
    int repeats = 8 * 1024;
#endif
    SPA::UINT64 res = 0;
    SPA::CStreamHeader sh;
    sh.RequestId = 500;
    sh.Size = sizeof (res);
#if defined(__ANDROID__) || defined(ANDROID)
	MQ_FILE::CMqFileEx qFile0("/data/data/com.android_native_test/files/q0", 30 * 24 * 3600, SPA::tagOptimistic::oSystemMemoryCached, L"socketPro", L"MyPassword", nullptr);
	MQ_FILE::CMqFileEx qFile1("/data/data/com.android_native_test/files/q1", 30 * 24 * 3600, SPA::tagOptimistic::oSystemMemoryCached, L"socketPro", L"MyPassword", nullptr);
	MQ_FILE::CMqFileEx qFile2("/data/data/com.android_native_test/files/q2", 30 * 24 * 3600, SPA::tagOptimistic::oSystemMemoryCached, L"socketPro", L"MyPassword", nullptr);

	MQ_FILE::CMqFileEx::PMqFile mqFiles[3] = { &qFile0, &qFile1, &qFile2 };

	MQ_FILE::CMqFileEx qSrc("/data/data/com.android_native_test/files/qsrc", 30 * 24 * 3600, SPA::oSystemMemoryCached, L"socketPro", L"MyPassword", nullptr);

#else
    MQ_FILE::CMqFileEx qFile0("q0", 30 * 24 * 3600, SPA::tagOptimistic::oSystemMemoryCached, L"socketPro", L"MyPassword", nullptr);
    MQ_FILE::CMqFileEx qFile1("q1", 30 * 24 * 3600, SPA::tagOptimistic::oSystemMemoryCached, L"socketPro", L"MyPassword", nullptr);
    MQ_FILE::CMqFileEx qFile2("q2", 30 * 24 * 3600, SPA::tagOptimistic::oSystemMemoryCached, L"socketPro", L"MyPassword", nullptr);

    MQ_FILE::CMqFileEx::PMqFile mqFiles[3] = {&qFile0, &qFile1, &qFile2};

    MQ_FILE::CMqFileEx qSrc("qsrc", 30 * 24 * 3600, SPA::tagOptimistic::oSystemMemoryCached, L"socketPro", L"MyPassword", nullptr);
#endif

    if (qSrc.GetQueueOpenStatus() == SPA::tagQueueStatus::qsMergePushing) {
        std::vector<MQ_FILE::CMqFile::PMqFile> vQs;
        if (qFile0.GetQueueOpenStatus() != SPA::tagQueueStatus::qsMergeComplete)
            vQs.push_back(&qFile0);
        if (qFile1.GetQueueOpenStatus() != SPA::tagQueueStatus::qsMergeComplete)
            vQs.push_back(&qFile1);
        if (qFile2.GetQueueOpenStatus() != SPA::tagQueueStatus::qsMergeComplete)
            vQs.push_back(&qFile2);

        if (vQs.size()) {
            MQ_FILE::CMqFile::PMqFile &p = vQs.front();
            ok = qSrc.AppendTo(&p, (unsigned int) vQs.size());
        } else {
            //clean all of existing queued data
            qSrc.Reset();
        }
    }

    {
        SPA::CScopeUQueue su;
        su << sample_str;
        sh.Size = su->GetSize();
        res = qSrc.StartJob();
        for (n = 0; n < repeats / 4; ++n) {
            qSrc.Enqueue(sh, su->GetBuffer(), su->GetSize());
        }
        res = qSrc.EndJob();
        for (n = 0; n < repeats / 4; ++n) {
            qSrc.Enqueue(sh, su->GetBuffer(), su->GetSize());
        }
        res = qSrc.Enqueue(sh, su->GetBuffer(), su->GetSize());
        res = qSrc.StartJob();
        for (n = 0; n < repeats / 2; ++n) {
            qSrc.Enqueue(sh, su->GetBuffer(), su->GetSize());
        }
        res = qSrc.EndJob();
    }

    ok = qSrc.AppendTo(mqFiles, 3);
}

int EnqThreadProc() {
    int *p = nullptr;
    SPA::UINT64 mqIndex;
    SPA::CScopeUQueue su;
    SPA::CStreamHeader sh;
    sh.RequestId = 100;
    SPA::CUQueue &q = *su;
    int n, MyCount = ITEM_COUNT / (Enq_Thread_Count ? Enq_Thread_Count : 1);
    for (n = 0; n < MyCount; ++n) {
        if (n && (n % 10) == 0)
            mqIndex = g_mqFile->StartJob();
        q << sample_str << n;
        sh.Size = q.GetSize();
        mqIndex = g_mqFile->Enqueue(sh, q);
        if (n && (n % 10) == 0)
            mqIndex = g_mqFile->EndJob();
        q.SetSize(0);
        //*p = 1;
    }
    return 0;
}

int DeqThreadProc() {
    int n;
    bool ok;
    SPA::UINT64 mqIndex;
    SPA::UINT64 pos;
    unsigned int waitTime = 200;
    std::string str;
    SPA::CStreamHeader sh;
    SPA::CScopeUQueue su;
    SPA::CUQueue &q = *su;
    pos = g_mqFile->Dequeue(q, mqIndex, waitTime);
    while (pos != INVALID_NUMBER) {
        q >> sh;
        if (sh.RequestId == (unsigned short)SPA::tagBaseRequestID::idStartJob || sh.RequestId == (unsigned short)SPA::tagBaseRequestID::idEndJob) {
            assert(q.GetSize() == 0);
            ok = g_mqFile->ConfirmDequeue(pos, mqIndex, false);
            pos = g_mqFile->Dequeue(q, mqIndex, waitTime);
            continue;
        }
        assert(sh.Size <= q.GetSize());
        q >> str >> n;

        assert(str == sample_str);
        assert(q.GetSize() == 0);

        bool fail = false; //(GetRandom() < 5);
        if (!fail) {
            g_cs.lock();
            ++Deq_Count;
            g_cs.unlock();
        }
        if (!g_mqFile->ConfirmDequeue(pos, mqIndex, fail))
            std::cout << "Bad pos = " << pos << ", index = " << mqIndex << std::endl;
        pos = g_mqFile->Dequeue(q, mqIndex, waitTime);
    }
    return 0;
}

int BatchDeqThreadProc() {
    int n;
    bool ok;
    unsigned int waitTime = 200;
    std::string str;
    SPA::CStreamHeader sh;
    SPA::CScopeUQueue qAttr;
    SPA::CScopeUQueue qRequests;
    do {
        std::vector<unsigned int> vSize = g_mqFile->DoBatchDequeue(10, *qAttr, *qRequests, waitTime);
        MQ_FILE::QAttr *qattr = (MQ_FILE::QAttr *)qAttr->GetBuffer();
        for (std::vector<unsigned int>::iterator it = vSize.begin(), end = vSize.end(); it != end; ++it) {
            qRequests >> sh;
            if (sh.RequestId == (unsigned short)SPA::tagBaseRequestID::idStartJob || sh.RequestId == (unsigned short)SPA::tagBaseRequestID::idEndJob) {
                ok = g_mqFile->ConfirmDequeue(qattr->MessagePos, qattr->MessageIndex, false);
                ++qattr;
                continue;
            }
            qRequests >> str >> n;
            g_cs.lock();
            ++Deq_Count;
            g_cs.unlock();
            assert(str == sample_str);
            assert(qattr->MessageIndex < MQ_FILE::QAttr::RANGE_DEQUEUED_END);
            if (!g_mqFile->ConfirmDequeue(qattr->MessagePos, qattr->MessageIndex, false))
                std::cout << "Bad pos = " << qattr->MessagePos << ", index = " << qattr->MessageIndex << std::endl;
            ++qattr;
            if (g_mqFile->IsSecure()) {
                assert(*it >= (sh.Size + sizeof (SPA::CStreamHeader)));
                qRequests->Pop(*it - sh.Size - sizeof (SPA::CStreamHeader));
            } else {
                assert(*it == (sh.Size + sizeof (SPA::CStreamHeader)));
            }
        }
        if (!vSize.size())
            break;
    } while (1);
    return 0;
}

int MaxDeqThreadProc() {
    int n;
    bool ok;
    unsigned int waitTime = 200;
    std::string str;
    SPA::CStreamHeader sh;
    SPA::CScopeUQueue qAttr;
    SPA::CScopeUQueue qRequests;
    do {
        std::vector<unsigned int> vSize = g_mqFile->DoBatchDequeue(*qAttr, *qRequests, 8 * 1024, waitTime);
        MQ_FILE::QAttr *qattr = (MQ_FILE::QAttr *)qAttr->GetBuffer();
        for (std::vector<unsigned int>::iterator it = vSize.begin(), end = vSize.end(); it != end; ++it) {
            qRequests >> sh;
            assert(qattr->MessageIndex < MQ_FILE::QAttr::RANGE_DEQUEUED_END);
            if (sh.RequestId == (unsigned short)SPA::tagBaseRequestID::idStartJob || sh.RequestId == (unsigned short)SPA::tagBaseRequestID::idEndJob) {
                ok = g_mqFile->ConfirmDequeue(qattr->MessagePos, qattr->MessageIndex, false);
                ++qattr;
                continue;
            }
            qRequests >> str >> n;
            g_cs.lock();
            ++Deq_Count;
            g_cs.unlock();
            assert(str == sample_str);
            if (!g_mqFile->ConfirmDequeue(qattr->MessagePos, qattr->MessageIndex, false))
                std::cout << "Bad pos = " << qattr->MessagePos << ", index = " << qattr->MessageIndex << std::endl;
            ++qattr;
            if (g_mqFile->IsSecure()) {
                assert(*it >= (sh.Size + sizeof (SPA::CStreamHeader)));
                qRequests->Pop(*it - sh.Size - sizeof (SPA::CStreamHeader));
            } else {
                assert(*it == (sh.Size + sizeof (SPA::CStreamHeader)));
            }
        }
        if (!vSize.size())
            break;
    } while (1);
    return 0;
}

void TestRegistration() {
    char secret[10] = {'1', '2', '3', '4', '5', 0};
    SPA::ServerSide::URegistration reg;
#ifndef WINCE
    std::string s = SPA::ServerSide::CreateKey("c:\\userver", true, secret, SPA::tagOperationSystem::osWin);
    std::ofstream outfile("key.txt");
    if (outfile) {
        outfile << s;
        outfile.close();
    }

    bool ok = SPA::ServerSide::IsRegisterred(secret, reg);

    time_t t = reg.GetEndDate();
#endif
}

void TestDateTimeSerialization() {
    SPA::CScopeUQueue su;

    SPA::UDateTime dtOut;
	std::time_t timeNow = std::time(nullptr);
	SPA::UDateTime dt(*std::localtime(&timeNow));
    su << dt;
    su >> dtOut;

    assert(dt == dtOut);
}

void TestWStringLength() {
    const wchar_t *wstr4bytes = L"㨵㫔乻亪亵亱𤭢𠜎𠻹𠾴𠾼";
    size_t len = ::wcslen(wstr4bytes);

    len = 0;
}

void TestZip() {
    SPA::CScopeUQueue su, out;
    const wchar_t *str = L"（大纪元记者万方、高紫檀报导）以北京为代表的大陆一线城市房价上涨不止，而学区房由于占尽优秀教育资源而倍受关注。㨵㫔乻亪亵亱𤭢𠜎𠻹𠾴𠾼 以北京为代表的大陆一线城市房价上涨不止，而学区房由于占尽优秀教育资源而倍受关注";
#if defined(__ANDROID__) || defined(ANDROID)
	std::basic_string<SPA::UTF16> utf16 = SPA::Utilities::ToUTF16(str, ::wcslen(str));
	std::wstring wstr = SPA::Utilities::ToWide(utf16.c_str(), utf16.size());
#elif defined(WIN32_64)
	std::wstring wstr(str);
#else
    SPA::Utilities::ToUTF16(str, ::wcslen(str), *su);
    SPA::Utilities::ToWide((const SPA::UTF16*)su->GetBuffer(), su->GetSize() / sizeof (SPA::UTF16), *out);
	std::wstring wstr((const wchar_t*)out->GetBuffer());
#endif
    assert(wstr == str);
    su->SetSize(0);
    out->SetSize(0);

    unsigned int destSize = su->GetMaxSize();

    bool ok = SPA::Compress(SPA::tagZipLevel::zlDefault, sample_str, (unsigned int) ::strlen(sample_str), (void*) su->GetBuffer(), destSize);

    su->SetSize(destSize);
    su->SetNull();

    destSize = out->GetMaxSize();

    ok = SPA::Decompress(SPA::tagZipLevel::zlDefault, su->GetBuffer(), su->GetSize(), (void*) out->GetBuffer(), destSize);

    out->SetSize(destSize);
    out->SetNull();

	assert(out->GetSize() == (unsigned int) ::strlen(sample_str));
	assert(memcmp(out->GetBuffer(), sample_str, out->GetSize()) == 0);
}

void TestEnqueue() {
	int *pInt = nullptr;
	unsigned int n;
	
	boost::posix_time::ptime t0 = boost::posix_time::microsec_clock::local_time();
#if defined(__ANDROID__) || defined(ANDROID)
	static const unsigned int TEST_CYCLES = 4 * 1024;
	MQ_FILE::CMqFile mqFile("/data/data/com.android_native_test/files/test_enq", 30 * 24 * 3600, SPA::oMemoryCached, false);
#else
	static const unsigned int TEST_CYCLES = 4 * 1024 * 1024;
    MQ_FILE::CMqFile mqFile("test_enq", 30 * 24 * 3600, SPA::tagOptimistic::oMemoryCached, false);
#endif
    SPA::CStreamHeader sh;
    for (n = 0; n < TEST_CYCLES; ++n) {
        SPA::UINT64 index = mqFile.Enqueue(sh, nullptr, 0);
        if (n == 0) {
            //throw CUExCode("test", 123);
            //throw;
            //throw exception();
        }
        //*pInt = 100;
    }
    boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();
    cout << "TestEnqueue time required = " << (t1 - t0).total_milliseconds() << ", count = " << n << endl;
}

void TestEnqueue3() {
    unsigned int n;
    static const unsigned int TEST_CYCLES = 1024;
    boost::posix_time::ptime t0 = boost::posix_time::microsec_clock::local_time();
#if defined(__ANDROID__) || defined(ANDROID)
	MQ_FILE::CMqFile mqFile("/data/data/com.android_native_test/files/test_enq3", 30 * 24 * 3600, SPA::oDiskCommitted, false);
#else
    MQ_FILE::CMqFile mqFile("test_enq3", 30 * 24 * 3600, SPA::tagOptimistic::oDiskCommitted, false);
#endif
    SPA::CStreamHeader sh;
    for (n = 0; n < TEST_CYCLES; ++n) {
        mqFile.Enqueue(sh, nullptr, 0);
    }
    boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();
    cout << "TestEnqueue3 time required = " << (t1 - t0).total_milliseconds() << ", count = " << n << endl;
}

void TestEnqueue2() {
    unsigned int n;
    static const unsigned int BATCH_SIZE = 16;
    static const unsigned int TEST_CYCLES = 256 * 1024;
    SPA::CScopeUQueue su;
    boost::posix_time::ptime t0 = boost::posix_time::microsec_clock::local_time();
#if defined(__ANDROID__) || defined(ANDROID)
	MQ_FILE::CMqFile mqFile("/data/data/com.android_native_test/files/test_enq2", 30 * 24 * 3600, SPA::oSystemMemoryCached, false);
#else
    MQ_FILE::CMqFile mqFile("test_enq2", 30 * 24 * 3600, SPA::tagOptimistic::oSystemMemoryCached, false);
#endif
    SPA::CStreamHeader sh;
    for (n = 0; n < BATCH_SIZE; ++n) {
        su << sh;
    }
    for (n = 0; n < TEST_CYCLES; ++n) {
        mqFile.BatchEnqueue(*su);
    }
    boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();
    cout << "TestEnqueue2 time required = " << (t1 - t0).total_milliseconds() << ", count = " << n * BATCH_SIZE << endl;
}

MQ_FILE::CQueueInitialInfo TestQueue() {
    int n;
#if defined(__ANDROID__) || defined(ANDROID)
	MQ_FILE::CMqFile mqFile("/data/data/com.android_native_test/files/cyetest", 30 * 24 * 3600, SPA::oSystemMemoryCached, false);
	//MQ_FILE::CMqFileEx mqFile("cyetest", 30 * 24 * 3600, SPA::oSystemMemoryCached, L"socketPro", L"MyPassword", nullptr);
#else
    MQ_FILE::CMqFile mqFile("cyetest", 30 * 24 * 3600, SPA::tagOptimistic::oSystemMemoryCached, false);
    //MQ_FILE::CMqFileEx mqFile("cyetest", 30 * 24 * 3600, SPA::oSystemMemoryCached, L"socketPro", L"MyPassword", nullptr);
#endif
    g_mqFile = &mqFile;

    unsigned int lasttime = mqFile.GetLastTime();

    SPA::UINT64 size = mqFile.GetMQSize();
    SPA::UINT64 count = mqFile.GetMessageCount();
    SPA::UINT64 index = mqFile.StartJob();
    {
        SPA::CStreamHeader sh;
        mqFile.Enqueue(sh, nullptr, 0);
    }
    bool ok = mqFile.AbortJob();
    assert(size == mqFile.GetMQSize());
    assert(count == mqFile.GetMessageCount());

    index = mqFile.StartJob();
    {
        SPA::CStreamHeader sh;
        sh.RequestId = 1;
        sh.Size = sizeof (index);
        index = mqFile.Enqueue(sh, (const unsigned char*) &index, sizeof (index));
    }
    index = mqFile.EndJob();
    unsigned short ids[3] = {(unsigned short)SPA::tagBaseRequestID::idStartJob, 1, (unsigned short)SPA::tagBaseRequestID::idEndJob};
    count = mqFile.CancelQueuedRequests(ids, 3);
    count = mqFile.GetMessageCount();

    {
        SPA::CStreamHeader sh;
        mqFile.Enqueue(sh, nullptr, 0);
        sh.RequestId = 1;
        sh.Size = sizeof (index);
        index = mqFile.Enqueue(sh, (const unsigned char*) &index, sizeof (index));
        index = mqFile.Enqueue(sh, (const unsigned char*) &index, sizeof (index));
    }

    mqFile.CancelQueuedRequests((SPA::UINT64)0, (~0));

    SPA::tagOptimistic crashSafe = g_mqFile->IsOptimistic();

    boost::posix_time::ptime t0 = boost::posix_time::microsec_clock::local_time();
    boost::thread_group enqueue_threads, dequeue_threads;
    for (n = 0; n < Enq_Thread_Count; ++n) {
        enqueue_threads.create_thread(&EnqThreadProc);
    }
    for (n = Enq_Thread_Count; n < (Enq_Thread_Count + Deq_Thread_Count); ++n) {

        //dequeue_threads.create_thread(&DeqThreadProc);
        //dequeue_threads.create_thread(&BatchDeqThreadProc);

        dequeue_threads.create_thread(&MaxDeqThreadProc);
    }
    enqueue_threads.join_all();
    dequeue_threads.join_all();
    boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration diff = t1 - t0;

    cout << "Time required = " << diff.total_milliseconds() << ", count = " << Deq_Count << endl;

    return mqFile.GetMQInitInfo();
}

void TestQLastIndex() {
    int *p = nullptr;
    MQ_FILE::QueueSha1 qs;
    MQ_FILE::QAttr qa(0, 0);
    MQ_FILE::CQLastIndex idx("test");

    ::memset(&qs, 0, sizeof (qs));
    qs.qs.Header = 12345;
    MQ_FILE::QueueSha1 qs0 = qs;
    idx.Set(qs, qa);
    qa.MessageIndex = 1;
    qa.MessagePos = 2;
    qs.qs.End = 12345;
    idx.Set(qs, qa);
    qa.MessageIndex = 1;
    qa.MessagePos = 2;
    qs.qs.End = 123456;
    idx.Set(qs, qa);
    qa.MessageIndex = 1;
    qa.MessagePos = 2;
    qs.qs.End = 3456;
    idx.Set(qs, qa);
    qa.MessageIndex = 1;
    qa.MessagePos = 2;
    qs.qs.End = 456;
    idx.Set(qs, qa);
    idx.Remove(qs0);

    //*p = 1;
}

void TestSysIdAndAppName() {
    std::string sysId = SPA::GetSysId();
    std::cout << "System Id = " << sysId << std::endl;
    std::string appName = SPA::GetAppName();
    std::cout << "App name = " << appName << std::endl;
}

#ifdef WINCE

std::string GetManagedAppCmd() {
    return "";
}

#elif defined(WIN32_64)

std::string GetManagedAppCmd() {
    DWORD pid = ::GetCurrentProcessId();
    HRESULT hr;
    ULONG res = 0;
    USES_CONVERSION;
    std::string str;
    bool b = CoInitializeEx(0, COINIT_MULTITHREADED) == S_OK;
    do {
        hr = CoInitializeSecurity(nullptr,
                -1, // COM authentication
                nullptr, // Authentication services
                nullptr, // Reserved
                RPC_C_AUTHN_LEVEL_DEFAULT, // Default authentication 
                RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
                nullptr, // Authentication info
                EOAC_NONE, // Additional capabilities 
                nullptr // Reserved
                );
        CComPtr<IWbemLocator> pIWbemLocator;
        hr = pIWbemLocator.CoCreateInstance(CLSID_WbemLocator);
        if (FAILED(hr))
            break;
        CComPtr<IWbemServices> pIWbemServices;
        hr = pIWbemLocator->ConnectServer(CComBSTR(L"ROOT\\CIMV2"), // Object path of WMI namespace
                nullptr, // User name. nullptr = current user
                nullptr, // User password. nullptr = current
                nullptr, // Locale. nullptr indicates current
                0, // Security flags.
                nullptr, // Authority (e.g. Kerberos)
                nullptr, // Context object 
                &pIWbemServices // pointer to IWbemServices proxy
                );
        if (FAILED(hr))
            break;
        hr = CoSetProxyBlanket(pIWbemServices, // Indicates the proxy to set
                RPC_C_AUTHN_WINNT, // RPC_C_AUTHN_xxx
                RPC_C_AUTHZ_NONE, // RPC_C_AUTHZ_xxx
                nullptr, // Server principal name 
                RPC_C_AUTHN_LEVEL_CALL, // RPC_C_AUTHN_LEVEL_xxx 
                RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
                nullptr, // client identity
                EOAC_NONE // proxy capabilities 
                );
        if (FAILED(hr))
            break;
        std::string cmd = "SELECT * FROM Win32_Process where ProcessId = ";
        cmd += std::to_string((SPA::INT64)pid);

        CComPtr<IEnumWbemClassObject> pIEnumWbemClassObject;
        hr = pIWbemServices->ExecQuery(CComBSTR("WQL"),
                CComBSTR(cmd.c_str()),
                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                nullptr,
                &pIEnumWbemClassObject);
        if (!FAILED(hr)) {
            CComPtr<IWbemClassObject> pIWbemClassObject;
            hr = pIEnumWbemClassObject->Next(WBEM_INFINITE, 1, &pIWbemClassObject, &res);
            if (!FAILED(hr) && pIWbemClassObject != nullptr) {
                CComVariant vt;
                hr = pIWbemClassObject->Get(L"CommandLine", 0, &vt, nullptr, nullptr);
                if (!FAILED(hr) && vt.vt == VT_BSTR)
                    str += W2A(vt.bstrVal);
            }
        }
    } while (false);
    if (b)
        CoUninitialize();
    return str;
}
#else

std::string GetManagedAppCmd() {
    return "";
}
#endif
