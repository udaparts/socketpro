// MemPackTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../../include/membuffer.h"
#include <iostream>
#include <fstream>
using namespace std;

namespace MB {

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
        su << (float) 23.45 << "test ASCII" << L"TEST UNICODE" << 34567.23;
        return su;
    }

}

#ifdef WIN32_64
typedef CComVariant CMyVaraint;
typedef CComBSTR CMyString;
#else
typedef MB::CBVariant CMyVaraint;
typedef stl::wstring CMyString;
#endif

void TestMCOnSameOS() {
    std::string asciiTest;
    CMyString UStr;
    float f;
    double d;
    MB::CScopeUQueue su = MB::TestMovableConstructor();
    su >> f >> asciiTest >> UStr >> d;
    assert(f == (float) 23.45);
    assert(asciiTest == "test ASCII");
    assert(UStr == L"TEST UNICODE");
    assert(d == 34567.23);
}

void TestVariantOnSameOS() {
#ifdef USE_BOOST_VARIANT_WITHIN_MB
    {
        std::wstring testme(L"Test me");
        MB::CUQueue q;
        MB::CBVariant v, vStr, vIntOut, vbOut;
        MB::CBVariant vtStr(testme);
        MB::CBVariant vt(1234567890000);
        std::vector<unsigned short> vB(2);
        vB[0] = 245;
        vB[1] = 34569;
        std::vector<int> vInt(2);
        vInt[0] = 25;
        vInt[1] = -2456;
        MB::CBVariant vBInt(vInt);
        MB::CBVariant vBSrc(vB);

#ifdef WIN32_64
        CComVariant vtInt64;
        CComVariant vtBstr;
        CComVariant vtIntOut;
        CComVariant vtB;
#endif
        q << vt << vtStr << vBInt << vBSrc;
        unsigned int size = q.GetSize();
        q >> v >> vStr >> vIntOut >> vbOut;
#ifdef WIN32_64
        q.SetSize(size);
        q >> vtInt64 >> vtBstr >> vtIntOut >> vtB;
        assert(vtInt64.vt == VT_I8);
        assert(vtInt64.llVal == 1234567890000);
        assert(vtBstr.vt == VT_BSTR);
        assert(testme == vtBstr.bstrVal);
        assert(vtIntOut.vt == (VT_ARRAY | VT_I4));
        assert(vtIntOut.parray->rgsabound->cElements == 2);
        long *pLong;
        ::SafeArrayAccessData(vtIntOut.parray, (void**) &pLong);
        assert(pLong[0] == 25);
        assert(pLong[1] == -2456);
        ::SafeArrayUnaccessData(vtIntOut.parray);
        unsigned short *pShort;
        assert(vtB.vt == (VT_ARRAY | VT_UI2));
        assert(vtB.parray->rgsabound->cElements == 2);
        ::SafeArrayAccessData(vtB.parray, (void**) &pShort);
        assert(pShort[0] == 245);
        assert(pShort[1] == 34569);
        ::SafeArrayUnaccessData(vtB.parray);
#endif
        int64_t lData = boost::get<int64_t > (v);
        assert(lData == 1234567890000);
        std::wstring &str = boost::get<std::wstring > (vStr);
        assert(testme == str);
        std::vector<int> &vMyInt = boost::get<std::vector<int> >(vIntOut);
        assert(vMyInt == vInt);
        std::vector<unsigned short> &vMyB = boost::get < std::vector<unsigned short> >(vbOut);
        assert(vMyB == vB);
    }
#endif
}

void SmokeTestOnSameMachine() {
    int n;
    unsigned short us = 25;
    unsigned short usOut;
    unsigned int un = 101;
    unsigned int unOut;
    std::string strOut, s;
    std::wstring ws(L"该宣言甫一出现，立即引起了民众的一片叫好与赞赏"), ws0, wsOut;
    MB::CUQueue mc;
    MB::CUQueue mcOut;
    mc << "test";
    {
        MB::CMbTest mbTest;
        MB::CMbTest mbTestOut;
        mbTest.n = 25;
        mbTest.s = "test structure";
        MB::CScopeUQueue sc;
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

void DoOsSpecificTest() {
#ifdef WIN32_64
    //window-specific unit testings
    {
        MB::CMbTest mbTest;
        MB::CMbTest mbTestOut;
        mbTest.n = 25;
        mbTest.s = "test structure from me";
        CAtlStringW wAtlStr(L"WTLwSTRMe"), wOut;
        std::wstring swOut;
        CAtlStringA aAtlStr("ASCIIString"), aOut;
        std::string saOut;
        CComBSTR bstr(L"MyBSTR"), bstrOut;
        CComVariant vtStr(L"MyVTStringTest"), vtOut;

        MB::CScopeUQueue su;
        su << wAtlStr << mbTest << aAtlStr << vtStr << bstr;
        unsigned int size = su->GetSize();
        su >> wOut >> mbTestOut >> aOut >> vtOut >> bstrOut;
        assert(wOut == wAtlStr);
        assert(mbTest.n == mbTestOut.n);
        assert(mbTest.s == mbTestOut.s);
        assert(aOut == aAtlStr);
        assert(vtStr == vtOut);
        assert(bstr == bstrOut);
        assert(su->GetSize() == 0);
        su->SetSize(size);

        su >> swOut >> mbTestOut >> saOut >> vtStr >> wAtlStr;
        assert(swOut == LPCWSTR(wOut));
        assert(mbTest.n == mbTestOut.n);
        assert(mbTest.s == mbTestOut.s);
        assert(saOut == LPCSTR(aOut));
        assert(su->GetSize() == 0);
        assert(vtStr == vtOut);
        assert(wAtlStr == bstrOut);
        assert(su->GetSize() == 0);
    }
#else

#endif
}

void CreateFile() {
    std::string aStr("This is a UNIT test from UCOMM"), aStrOut;
    CMyString ws(L"该宣言甫一出现，立即引起了民众的一片叫好与赞赏"), wsOut;
    CMyVaraint vtInt64(1234567890000), vtInt64Out;
    CMyVaraint vtWstrArray, vtWstrArrayOut;
    CMyVaraint vtUShort, vtUShortOut;
    CMyVaraint vtBool, vtBoolOut;
    CMyVaraint vtDate, vtDateOut;
    CMyVaraint vtDateArray, vtDateArrayOut;

#ifdef WIN32_64
    ofstream outfile("win2nix.data", ios::out | ios::binary | ios::trunc);
    MB::CScopeUQueue su(false, MB::osLinux); //win --> nix
    SYSTEMTIME st;
    ::GetLocalTime(&st);
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
    vtBool.vt = (VT_ARRAY | VT_BOOL);
    vtBool.parray = pSafeArray;

    sab[0].cElements = 2;
    pSafeArray = ::SafeArrayCreate(VT_UI2, 1, sab);
    unsigned short *pUShort;
    ::SafeArrayAccessData(pSafeArray, (void**) &pUShort);
    pUShort[0] = 25;
    pUShort[1] = 2456;
    ::SafeArrayUnaccessData(pSafeArray);
    vtUShort.vt = (VT_ARRAY | VT_UI2);
    vtUShort.parray = pSafeArray;

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
    pBstr[0] = ::SysAllocString(L"笔者并非女士，然每至此节，却都会情不自禁想起一个人和一篇文章");
    pBstr[1] = ::SysAllocString(L"女同志的结婚永远使人注意，而不会使人满意的");
    ::SafeArrayUnaccessData(pSafeArray);
    vtWstrArray.vt = (VT_ARRAY | VT_BSTR);
    vtWstrArray.parray = pSafeArray;

#else
    std::vector<bool> vBool(3);
    vB[0] = true;
    vB[1] = false;
    vB[2] = true;
    std::vector<unsigned short> vShort(2);
    vInt[0] = 25;
    vInt[1] = 2456;
    CMyVaraint vtShort(vShort), vtShortOut;
    CMyVaraint vtBool(vBool), vtBoolOut;
    CMyVaraint vtDate;
    CMyVaraint vtDateArray;

    std::vector<std::wstring> vWString(3);
    vWString[1] = ws;
    vWString[2] = L"MY立即引起了民众的一片叫好与赞赏MY";
    vtWstrArray = vWString;

    MB::CScopeUQueue su(false, MB::osWin); //nix --> win
    ofstream outfile("nix2win.dat", ios::out | ios::binary | ios::trunc);
#endif
    su << vtBool << vtInt64 << ws << vtUShort << aStr << vtDate << vtWstrArray << vtDateArray;


    outfile.write((const char*) su->GetBuffer(), su->GetSize());
}

int main(int argc, char* argv[]) {
    CreateFile();
    SmokeTestOnSameMachine();
    TestMCOnSameOS();
    TestVariantOnSameOS();
    DoOsSpecificTest();
    return 0;
}

