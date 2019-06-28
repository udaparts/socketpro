#ifndef ___SOCKETPRO_CLIENT_HANDLER_TECHOB_H__
#define ___SOCKETPRO_CLIENT_HANDLER_TECHOB_H__

#include "../include/aclientw.h" 

using namespace SPA;
using namespace SPA::ClientSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../TEchoB_i.h"

//client handler for service CEchoBasic

class CEchoBasic : public CAsyncServiceHandler {
public:

    CEchoBasic(CClientSocket *pClientSocket = nullptr)
    : CAsyncServiceHandler(sidCEchoBasic, pClientSocket) {
    }

public:

    bool EchoBool(bool b) {
        bool EchoBoolRtn;
        bool bProcessRy = ProcessR1(idEchoBoolCEchoBasic, b, EchoBoolRtn);
        return EchoBoolRtn;
    }

    char EchoInt8(char c) {
        char EchoInt8Rtn;
        bool bProcessRy = ProcessR1(idEchoInt8CEchoBasic, c, EchoInt8Rtn);
        return EchoInt8Rtn;
    }

    unsigned char EchoUInt8(unsigned char b) {
        unsigned char EchoUInt8Rtn;
        bool bProcessRy = ProcessR1(idEchoUInt8CEchoBasic, b, EchoUInt8Rtn);
        return EchoUInt8Rtn;
    }

    short EchoInt16(short s) {
        short EchoInt16Rtn;
        bool bProcessRy = ProcessR1(idEchoInt16CEchoBasic, s, EchoInt16Rtn);
        return EchoInt16Rtn;
    }

    unsigned short EchoUInt16(unsigned short s) {
        unsigned short EchoUInt16Rtn;
        bool bProcessRy = ProcessR1(idEchoUInt16CEchoBasic, s, EchoUInt16Rtn);
        return EchoUInt16Rtn;
    }

    int EchoInt32(int data) {
        int EchoInt32Rtn;
        bool bProcessRy = ProcessR1(idEchoInt32CEchoBasic, data, EchoInt32Rtn);
        return EchoInt32Rtn;
    }

    unsigned int EchoUInt32(unsigned int data) {
        unsigned int EchoUInt32Rtn;
        bool bProcessRy = ProcessR1(idEchoUInt32CEchoBasic, data, EchoUInt32Rtn);
        return EchoUInt32Rtn;
    }

    SPA::INT64 EchoInt64(SPA::INT64 data) {
        SPA::INT64 EchoInt64Rtn;
        bool bProcessRy = ProcessR1(idEchoInt64CEchoBasic, data, EchoInt64Rtn);
        return EchoInt64Rtn;
    }

    SPA::UINT64 EchoUInt64(SPA::UINT64 data) {
        SPA::UINT64 EchoUInt64Rtn;
        bool bProcessRy = ProcessR1(idEchoUInt64CEchoBasic, data, EchoUInt64Rtn);
        return EchoUInt64Rtn;
    }

    float EchoFloat(float data) {
        float EchoFloatRtn;
        bool bProcessRy = ProcessR1(idEchoFloatCEchoBasic, data, EchoFloatRtn);
        return EchoFloatRtn;
    }

    double EchoDouble(double data) {
        double EchoDoubleRtn;
        bool bProcessRy = ProcessR1(idEchoDoubleCEchoBasic, data, EchoDoubleRtn);
        return EchoDoubleRtn;
    }

    std::wstring EchoString(const wchar_t* str) {
        std::wstring EchoStringRtn;
        bool bProcessRy = ProcessR1(idEchoStringCEchoBasic, str, EchoStringRtn);
        assert(bProcessRy);
        return EchoStringRtn;
    }

    std::string EchoAString(const char* str) {
        std::string EchoAStringRtn;
        bool bProcessRy = ProcessR1(idEchoAStringCEchoBasic, str, EchoAStringRtn);
        return EchoAStringRtn;
    }

    DECIMAL EchoDecimal(DECIMAL dec) {
        DECIMAL EchoDecimalRtn;
        bool bProcessRy = ProcessR1(idEchoDecimalCEchoBasic, dec, EchoDecimalRtn);
        return EchoDecimalRtn;
    }

    wchar_t EchoWChar(wchar_t wc) {
        wchar_t EchoWCharRtn;
        bool bProcessRy = ProcessR1(idEchoWCharCEchoBasic, wc, EchoWCharRtn);
        return EchoWCharRtn;
    }

    GUID EchoGuid(const GUID &guid) {
        GUID EchoGuidRtn;
        bool bProcessRy = ProcessR1(idEchoGuidCEchoBasic, guid, EchoGuidRtn);
        return EchoGuidRtn;
    }

    CY EchoCy(const CY &cy) {
        CY EchoCyRtn;
        bool bProcessRy = ProcessR1(idEchoCyCEchoBasic, cy, EchoCyRtn);
        return EchoCyRtn;
    }

    SPA::UDateTime EchoDateTime(const SPA::UDateTime &dt) {
        SPA::UDateTime EchoDateTimeRtn;
        bool bProcessRy = ProcessR1(idEchoDateTime, dt, EchoDateTimeRtn);
        return EchoDateTimeRtn;
    }
};
#endif
