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
        return async<bool>(idEchoBoolCEchoBasic, b).get();
    }

    char EchoInt8(char c) {
        return async<char>(idEchoInt8CEchoBasic, c).get();
    }

    unsigned char EchoUInt8(unsigned char b) {
        return async<unsigned char>(idEchoUInt8CEchoBasic, b).get();
    }

    short EchoInt16(short s) {
        return async<short>(idEchoInt16CEchoBasic, s).get();
    }

    unsigned short EchoUInt16(unsigned short s) {
        return async<unsigned short>(idEchoUInt16CEchoBasic, s).get();
    }

    int EchoInt32(int data) {
        return async<int>(idEchoInt32CEchoBasic, data).get();
    }

    unsigned int EchoUInt32(unsigned int data) {
        return async<unsigned int>(idEchoUInt32CEchoBasic, data).get();
    }

    SPA::INT64 EchoInt64(SPA::INT64 data) {
        return async<SPA::INT64>(idEchoInt64CEchoBasic, data).get();
    }

    SPA::UINT64 EchoUInt64(SPA::UINT64 data) {
        return async<SPA::UINT64>(idEchoUInt64CEchoBasic, data).get();
    }

    float EchoFloat(float data) {
        return async<float>(idEchoFloatCEchoBasic, data).get();
    }

    double EchoDouble(double data) {
        return async<double>(idEchoDoubleCEchoBasic, data).get();
    }

    std::wstring EchoString(const wchar_t* str) {
        return async<std::wstring>(idEchoStringCEchoBasic, str).get();
    }

    std::string EchoAString(const char* str) {
        return async<std::string>(idEchoAStringCEchoBasic, str).get();
    }

    DECIMAL EchoDecimal(DECIMAL dec) {
        return async<DECIMAL>(idEchoDecimalCEchoBasic, dec).get();
    }

    wchar_t EchoWChar(wchar_t wc) {
        return async<wchar_t>(idEchoWCharCEchoBasic, wc).get();
    }

    GUID EchoGuid(const GUID &guid) {
        return async<GUID>(idEchoGuidCEchoBasic, guid).get();
    }

    CY EchoCy(const CY &cy) {
        return async<CY>(idEchoCyCEchoBasic, cy).get();
    }

    SPA::UDateTime EchoDateTime(const SPA::UDateTime &dt) {
        return async<SPA::UDateTime>(idEchoDateTime, dt).get();
    }
};
#endif
