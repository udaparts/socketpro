
// MemPackTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../../include/membuffer.h"
#include <iostream>
#include <fstream>
#include <deque>
#include <queue>

using namespace std;

namespace SPA {

    struct CMbTest {
        int n;
        std::string s;
    };

    CUQueue& operator<<(CUQueue &q, const CMbTest& mb) {
        q << mb.n << mb.s;
        return q;
    }

    CUQueue& operator>>(CUQueue &q, CMbTest& mb) {
        q >> mb.n >> mb.s;
        return q;
    }

    CScopeUQueue TestMovableConstructor() {
        CScopeUQueue su;
        su << (float) 23.45;
        su << "test ASCII";
        su << L"TEST UNICODE";
        su << 34567.23;
        return su;
    }
}

#ifdef WIN32_64
typedef CComVariant CMyVaraint;
typedef CComBSTR CMyString;
#else
typedef SPA::CBVariant CMyVaraint;
typedef std::wstring CMyString;
#endif

void TestTime() {
    timeb tb;
#ifdef WIN32_64
    __timeb64 mytb;
    ftime(&mytb);
#else
    timeb mytb;
    ftime(&mytb);
#endif
    SPA::UDateTime dt;
    memset(&dt, 0, sizeof (dt));
    tm *p = gmtime(&mytb.time);
    dt.Year = p->tm_year + 1900;
    dt.Month = p->tm_mon + 1;
    dt.Day = p->tm_mday;
    dt.Hour = p->tm_hour;
    dt.Minute = p->tm_min;
    dt.Second = p->tm_sec;
    time_t mt = dt.ToTime();
    assert(mt == mytb.time);
    tb = dt.ToTimeb();
    //assert(tb.timezone == mytb.timezone);
#ifdef WIN32_64
    SYSTEMTIME st;
    double dTime;
    ::memset(&st, 0, sizeof (st));
    st.wYear = dt.Year;
    st.wMonth = dt.Month;
    st.wDay = dt.Day;
    st.wHour = dt.Hour;
    st.wMinute = dt.Minute;
    st.wSecond = dt.Second;
    ::SystemTimeToVariantTime(&st, &dTime);
    time_t now = SPA::WindowVariantTimeToCTime(dTime);
    assert(now == mt);
#endif
}

const char *testA = "This is a UNIT test from UCOMM";
std::wstring testW(L"传温家宝政治局再提政");
std::wstring aW0(L"人士还透露近年来温家宝多次提出要政治改革");
std::wstring aW1(L"为了维持中共的权贵腐败特权利益，为了维持一党独裁带来的特权利");

void ReadFile() {
#ifdef WIN32_64
    std::ifstream is("mem_file_nix.data", std::ios::in | std::ios::binary);
#else
    std::ifstream is("mem_file_win.data", std::ios::in | std::ios::binary);
#endif
    if (!is.is_open())
        return;

    std::string aOut;
    std::wstring wOut;
    CMyVaraint vtInt64;
    CMyVaraint vtWstrArray;
    CMyVaraint vtUShortArray;
    CMyVaraint vtBoolArray;
    CMyVaraint vtDate;
    CMyVaraint vtDateArray;
    CMyVaraint vtBool;
    CMyVaraint vtNull;
    std::string str;
    std::wstring wstr;
    CMyVaraint vtFileTime;
    CMyVaraint vtFileTimeArray;

#ifdef WIN32_64
    SPA::CScopeUQueue su(SPA::osLinux, false);
#else
    CMyVaraint vtCyArray;
    CMyVaraint vtDecArray;
    SPA::CScopeUQueue su(SPA::osWin, false);
#endif
    is.seekg(0, std::ios::end);
    std::streamoff len = is.tellg();
    is.seekg(0, std::ios::beg);
    if (len > su->GetMaxSize())
        su->ReallocBuffer((unsigned int) len + 128);
    is.read((char*) su->GetBuffer(), (unsigned int) len);
    su->SetSize((unsigned int) len);

    su >> vtBoolArray;
    su >> vtInt64;
    su >> wOut;
    assert(wOut == testW);
    su >> vtUShortArray;
    su >> aOut;
    assert(aOut == testA);
    su >> vtBool;
#ifdef WIN32_64

#else
    assert(boost::get<bool>(vtBool) == true);
#endif

    su >> vtDate;
#ifdef WIN32_64

#else
    SPA::UDateTime &udt = boost::get<SPA::UDateTime > (vtDate);
    udt.Microsecond = 0;
#endif
    su >> vtWstrArray;
#ifdef WIN32_64

#else
    std::vector<std::wstring> &vWString = boost::get<std::vector<std::wstring> > (vtWstrArray);
    vWString.clear();
#endif
    su >> vtDateArray;
    su >> vtNull;
    su >> str;
    su >> wstr;
#ifdef WIN32_64

#else
    std::vector<SPA::UDateTime> &vUDT = boost::get<std::vector<SPA::UDateTime> > (vtDateArray);
    vUDT.clear();

    su >> vtCyArray;
    su >> vtFileTime;
    su >> vtDecArray;
    su >> vtFileTimeArray;
#endif
    assert(su->GetSize() == 0);
}

void NixCreateFile() {
    CMyVaraint vtInt64(1234567890000);
    CMyVaraint vtWstrArray;
    CMyVaraint vtUShortArray;
    CMyVaraint vtBoolArray;
    CMyVaraint vtDate;
    CMyVaraint vtDateArray;
    CMyVaraint vtBool(true);
    CMyVaraint vtNull;
    char *str = NULL;
    wchar_t *wstr = NULL;

    ofstream outfile("mem_file_nix.data", ios::out | ios::binary | ios::trunc);
    SPA::CScopeUQueue su(SPA::osWin, SPA::IsBigEndian()); //save file which will be parsed on winddow platforms

#ifndef WIN32_64
    std::vector<std::wstring> vWStr;
    vWStr.push_back(aW0);
    vWStr.push_back(aW1);
    vtWstrArray = vWStr;
    std::vector<bool> vBool;
    vBool.push_back(true);
    vBool.push_back(false);
    vBool.push_back(true);
    vtBoolArray = vBool;
    std::vector<unsigned short> vUShort;
    vUShort.push_back(25);
    vUShort.push_back(2456);
    vtUShortArray = vUShort;
    SPA::UDateTime udt;
    udt.Year = 2012;
    udt.Month = 4;
    udt.Day = 29;
    udt.Hour = 10;
    udt.Minute = 45;
    udt.Second = 57;
    udt.Millisecond = 456;
    udt.Microsecond = 234;
    std::vector<SPA::UDateTime> vUDT;
    vUDT.push_back(udt);
    udt.Year += 1;
    vUDT.push_back(udt);
    udt.Year += 2;
    vUDT.push_back(udt);
    vtDateArray = vUDT;
    udt.Year += 3;
    vtDate = udt;

#endif
    su << vtBoolArray;
    su << vtInt64;
    su << testW;
    su << vtUShortArray;
    su << testA;
    su << vtBool;
    su << vtDate;
    su << vtWstrArray;
    su << vtDateArray;
    su << vtNull;
    su << str;
    su << wstr;
    outfile.write((const char*) su->GetBuffer(), su->GetSize());
    outfile.close();
}

void WinCreateFile() {
    CMyVaraint vtInt64(1234567890000);
    CMyVaraint vtWstrArray;
    CMyVaraint vtUShortArray;
    CMyVaraint vtBoolArray;
    CMyVaraint vtDate;
    CMyVaraint vtDateArray;
    CMyVaraint vtBool(true);
    CMyVaraint vtCyArray;
    CMyVaraint vtDecArray;
    CMyVaraint vtNull;
    char *str = NULL;
    wchar_t *wstr = NULL;
    SPA::UINT64 uiFileTime;
    CMyVaraint vtFileTime;
    CMyVaraint vtFileTimeArray;

    ofstream outfile("mem_file_win.data", ios::out | ios::binary | ios::trunc);

    //save file which will be parsed on linux platforms
    SPA::CScopeUQueue su(SPA::osLinux, SPA::IsBigEndian());

#ifdef WIN32_64
    FILETIME ft;
    SYSTEMTIME st;
    st.wYear = 2012;
    st.wMonth = 5;
    st.wDay = 28;
    st.wHour = 8;
    st.wMinute = 31;
    st.wSecond = 25;
    st.wMilliseconds = 587;
    ::SystemTimeToFileTime(&st, &ft);
    uiFileTime = ft.dwHighDateTime;
    uiFileTime <<= 32;
    uiFileTime += ft.dwLowDateTime;
    uiFileTime += 2110;
    ::SystemTimeToVariantTime(&st, &vtDate.date);
    vtDate.vt = VT_DATE;

    SAFEARRAYBOUND sab[1] = {3, 0};
    SAFEARRAY *pSafeArray = ::SafeArrayCreate(VT_BOOL, 1, sab);
    VARIANT_BOOL *pBool;
    ::SafeArrayAccessData(pSafeArray, (void**) &pBool);
    pBool[0] = VARIANT_TRUE;
    pBool[1] = VARIANT_FALSE;
    pBool[2] = VARIANT_TRUE;
    ::SafeArrayUnaccessData(pSafeArray);
    vtBoolArray.vt = (VT_ARRAY | VT_BOOL);
    vtBoolArray.parray = pSafeArray;

    sab[0].cElements = 2;
    pSafeArray = ::SafeArrayCreate(VT_UI2, 1, sab);
    unsigned short *pUShort;
    ::SafeArrayAccessData(pSafeArray, (void**) &pUShort);
    pUShort[0] = 25;
    pUShort[1] = 2456;
    ::SafeArrayUnaccessData(pSafeArray);
    vtUShortArray.vt = (VT_ARRAY | VT_UI2);
    vtUShortArray.parray = pSafeArray;

    sab[0].cElements = 3;
    pSafeArray = ::SafeArrayCreate(VT_DATE, 1, sab);
    DATE *pDate;
    ::SafeArrayAccessData(pSafeArray, (void**) &pDate);
    pDate[0] = vtDate.date;
    st.wYear += 1;
    ::SystemTimeToVariantTime(&st, pDate + 1);
    st.wYear += 1;
    ::SystemTimeToVariantTime(&st, pDate + 2);
    ::SafeArrayUnaccessData(pSafeArray);
    vtDateArray.vt = (VT_ARRAY | VT_DATE);
    vtDateArray.parray = pSafeArray;

    sab[0].cElements = 2;
    pSafeArray = ::SafeArrayCreate(VT_BSTR, 1, sab);
    BSTR *pBstr;
    ::SafeArrayAccessData(pSafeArray, (void**) &pBstr);
    pBstr[0] = ::SysAllocString(aW0.c_str());
    pBstr[1] = ::SysAllocString(aW1.c_str());
    ::SafeArrayUnaccessData(pSafeArray);
    vtWstrArray.vt = (VT_ARRAY | VT_BSTR);
    vtWstrArray.parray = pSafeArray;

    sab[0].cElements = 2;
    pSafeArray = ::SafeArrayCreate(VT_CY, 1, sab);
    CY *pCy;
    ::SafeArrayAccessData(pSafeArray, (void**) &pCy);
    pCy[0].int64 = 1234567890;
    pCy[1].int64 = 9876543210;
    ::SafeArrayUnaccessData(pSafeArray);
    vtCyArray.vt = (VT_ARRAY | VT_CY);
    vtCyArray.parray = pSafeArray;

    sab[0].cElements = 2;
    pSafeArray = ::SafeArrayCreate(VT_DECIMAL, 1, sab);
    DECIMAL *pDec;
    ::SafeArrayAccessData(pSafeArray, (void**) &pDec);
    pDec[0].Lo64 = 1234567890123;
    pDec[0].scale = 5;
    pDec[0].sign = 0;
    pDec[1].Lo64 = 9876543210123;
    pDec[1].scale = 4;
    pDec[1].sign = 1;
    ::SafeArrayUnaccessData(pSafeArray);
    vtDecArray.vt = (VT_ARRAY | VT_DECIMAL);
    vtDecArray.parray = pSafeArray;

    vtFileTime.ullVal = uiFileTime;
    vtFileTime.vt = VT_FILETIME;

    SPA::UINT64 *pFileTime;
    sab[0].cElements = 1;
    pSafeArray = ::SafeArrayCreate(VT_UI8, 1, sab);
    ::SafeArrayAccessData(pSafeArray, (void**) &pFileTime);
    pFileTime[0] = uiFileTime;
    ::SafeArrayUnaccessData(pSafeArray);
    vtFileTimeArray.vt = (VT_ARRAY | VT_FILETIME);
    vtFileTimeArray.parray = pSafeArray;

#endif

    su << vtBoolArray;
    su << vtInt64;
    su << testW;
    su << vtUShortArray;
    su << testA;
    su << vtBool;
    su << vtDate;
    su << vtWstrArray;
    su << vtDateArray;
    su << vtNull;
    su << str;
    su << wstr;
    su << vtCyArray;
    su << vtFileTime;
    su << vtDecArray;
    su << vtFileTimeArray;
    
#ifdef WIN32_64
    //make sure it back to VT_UI8 or VT_ARRAY|VT_UI8  so that the test will no be crashed.
    vtFileTime.vt = VT_UI8;
    vtFileTimeArray.vt = (VT_ARRAY | VT_UI8);
#endif

    outfile.write((const char*) su->GetBuffer(), su->GetSize());
    outfile.close();
}

void TestMCOnSameOS() {
    std::string asciiTest;
    CMyString UStr;
    float f;
    double d;
    SPA::CScopeUQueue su = SPA::TestMovableConstructor();
    su >> f;
    su >> asciiTest;
    su >> UStr;
    su >> d;
    assert(f == (float) 23.45);
    assert(asciiTest == "test ASCII");
    assert(UStr == L"TEST UNICODE");
    assert(d == 34567.23);
}

void SmokeTestOnSameMachine() {
    int n;
    unsigned short us = 25;
    unsigned short usOut;
    unsigned int un = 101;
    unsigned int unOut;
    std::string strOut, s;
    std::wstring ws(L"部分父母后悔赴港生子"), ws0, wsOut;
    SPA::CUQueue mc;
    SPA::CUQueue mcOut;
    mc << "test";
    {
        SPA::CMbTest mbTest;
        SPA::CMbTest mbTestOut;
        mbTest.n = 25;
        mbTest.s = "test structure";
        SPA::CScopeUQueue sc;
        sc << mc;
        sc << ws;
        sc << mbTest;
        sc << 12;
        sc << us;
        sc << "This is a test";
        sc << un;
        sc << L"MyUNICODE TEST";
        sc >> mcOut >> wsOut >> mbTestOut >> n >> usOut >> s >> unOut >> ws0;
        assert(mc.GetSize() == mcOut.GetSize());
        assert(memcmp(mc.GetBuffer(), mcOut.GetBuffer(), mc.GetSize()) == 0);
        assert(wsOut == ws);
        assert(mbTestOut.n == mbTest.n);
        assert(mbTestOut.s == mbTest.s);
        assert(n == 12);
        assert(usOut == us);
        assert(s == "This is a test");
        assert(un == unOut);
        assert(ws0 == L"MyUNICODE TEST");
        assert(sc->GetSize() == 0);
    }
}

void DequeueTest()
{
	deque<SPA::CScopeUQueue> deqUQueue;

	SPA::CScopeUQueue su, su1;

	su << 1 << 123.456 <<"Test";
	su1 << true << L"MyTest";

	deqUQueue.push_back((SPA::CScopeUQueue&&)su);
	deqUQueue.push_back((SPA::CScopeUQueue&&)su1);

	su.Swap(deqUQueue[0]);
	deqUQueue.pop_front();

	su1.Swap(deqUQueue[0]);
	deqUQueue.pop_front();
}

int main(int argc, char* argv[]) {
    TestTime();
    TestMCOnSameOS();
    SmokeTestOnSameMachine();
#ifdef WIN32_64
    WinCreateFile();
#else
    NixCreateFile();
#endif
    ReadFile();
	DequeueTest();
    return 0;
}


