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
        return send<bool>(idEchoBoolCEchoBasic, b).get();
    }

    char EchoInt8(char c) {
        return send<char>(idEchoInt8CEchoBasic, c).get();
    }

    unsigned char EchoUInt8(unsigned char b) {
        return send<unsigned char>(idEchoUInt8CEchoBasic, b).get();
    }

    short EchoInt16(short s) {
        return send<short>(idEchoInt16CEchoBasic, s).get();
    }

    unsigned short EchoUInt16(unsigned short s) {
        return send<unsigned short>(idEchoUInt16CEchoBasic, s).get();
    }

    int EchoInt32(int data) {
        return send<int>(idEchoInt32CEchoBasic, data).get();
    }

    unsigned int EchoUInt32(unsigned int data) {
        return send<unsigned int>(idEchoUInt32CEchoBasic, data).get();
    }

    SPA::INT64 EchoInt64(SPA::INT64 data) {
        return send<SPA::INT64>(idEchoInt64CEchoBasic, data).get();
    }

    SPA::UINT64 EchoUInt64(SPA::UINT64 data) {
        return send<SPA::UINT64>(idEchoUInt64CEchoBasic, data).get();
    }

    float EchoFloat(float data) {
        return send<float>(idEchoFloatCEchoBasic, data).get();
    }

    double EchoDouble(double data) {
        return send<double>(idEchoDoubleCEchoBasic, data).get();
    }

    std::wstring EchoString(const wchar_t* str) {
        return send<std::wstring>(idEchoStringCEchoBasic, str).get();
    }

    std::string EchoAString(const char* str) {
        return send<std::string>(idEchoAStringCEchoBasic, str).get();
    }

    DECIMAL EchoDecimal(DECIMAL dec) {
        return send<DECIMAL>(idEchoDecimalCEchoBasic, dec).get();
    }

    wchar_t EchoWChar(wchar_t wc) {
        return send<wchar_t>(idEchoWCharCEchoBasic, wc).get();
    }

    GUID EchoGuid(const GUID &guid) {
        return send<GUID>(idEchoGuidCEchoBasic, guid).get();
    }

    CY EchoCy(const CY &cy) {
        return send<CY>(idEchoCyCEchoBasic, cy).get();
    }

    SPA::UDateTime EchoDateTime(const SPA::UDateTime &dt) {
        return send<SPA::UDateTime>(idEchoDateTime, dt).get();
    }
};
#endif
