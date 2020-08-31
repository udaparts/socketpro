#ifndef ___SOCKETPRO_CLIENT_HANDLER_TECHOC_H__
#define ___SOCKETPRO_CLIENT_HANDLER_TECHOC_H__

#include "../include/aclientw.h"
using namespace SPA;
using namespace SPA::ClientSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../TEchoC_i.h"

//client handler for service CEchoObject

class CEchoObject : public CAsyncServiceHandler {
public:

    CEchoObject(CClientSocket *pClientSocket)
    : CAsyncServiceHandler(sidCEchoObject, pClientSocket) {
    }

public:

    UVariant EchoEmpty(const UVariant &empty) {
        return async<UVariant>(idEchoEmptyCEchoObject, empty).get();
    }

    UVariant EchoBool(const UVariant &b) {
        return async<UVariant>(idEchoBoolCEchoObject, b).get();
    }

    UVariant EchoInt8(const UVariant &c) {
        return async<UVariant>(idEchoInt8CEchoObject, c).get();
    }

    UVariant EchoUInt8(const UVariant &b) {
        return async<UVariant>(idEchoUInt8CEchoObject, b).get();
    }

    UVariant EchoInt16(const UVariant &s) {
        return async<UVariant>(idEchoInt16CEchoObject, s).get();
    }

    UVariant EchoUInt16(const UVariant &s) {
        return async<UVariant>(idEchoUInt16CEchoObject, s).get();
    }

    UVariant EchoInt32(const UVariant &data) {
        return async<UVariant>(idEchoInt32CEchoObject, data).get();
    }

    UVariant EchoUInt32(const UVariant &data) {
        return async<UVariant>(idEchoUInt32CEchoObject, data).get();
    }

    UVariant EchoInt64(const UVariant &data) {
        return async<UVariant>(idEchoInt64CEchoObject, data).get();
    }

    UVariant EchoUInt64(const UVariant &data) {
        return async<UVariant>(idEchoUInt64CEchoObject, data).get();
    }

    UVariant EchoFloat(const UVariant &data) {
        return async<UVariant>(idEchoFloatCEchoObject, data).get();
    }

    UVariant EchoDouble(const UVariant &data) {
        return async<UVariant>(idEchoDoubleCEchoObject, data).get();
    }

    UVariant EchoString(const UVariant &str) {
        return async<UVariant>(idEchoStringCEchoObject, str).get();
    }

    UVariant EchoAString(const UVariant &str) {
        return async<UVariant>(idEchoAStringCEchoObject, str).get();
    }

    UVariant EchoDecimal(const UVariant &dec) {
        return async<UVariant>(idEchoDecimalCEchoObject, dec).get();
    }

    UVariant EchoBoolArray(const UVariant &bArr) {
        return async<UVariant>(idEchoBoolArrayCEchoObject, bArr).get();
    }

    UVariant EchoUInt8Array(const UVariant &bArr) {
        return async<UVariant>(idEchoUInt8ArrayCEchoObject, bArr).get();
    }

    UVariant EchoInt16Array(const UVariant &shortArr) {
        return async<UVariant>(idEchoInt16ArrayCEchoObject, shortArr).get();
    }

    UVariant EchoUInt16Array(const UVariant &ushortArr) {
        return async<UVariant>(idEchoUInt16ArrayCEchoObject, ushortArr).get();
    }

    UVariant EchoInt32Array(const UVariant &intArr) {
        return async<UVariant>(idEchoInt32ArrayCEchoObject, intArr).get();
    }

    UVariant EchoUInt32Array(const UVariant &uintArr) {
        return async<UVariant>(idEchoUInt32ArrayCEchoObject, uintArr).get();
    }

    UVariant EchoInt64Array(const UVariant &longArr) {
        return async<UVariant>(idEchoInt64ArrayCEchoObject, longArr).get();
    }

    UVariant EchoUInt64Array(const UVariant &ulongArr) {
        return async<UVariant>(idEchoUInt64ArrayCEchoObject, ulongArr).get();
    }

    UVariant EchoFloatArray(const UVariant &fArr) {
        return async<UVariant>(idEchoFloatArrayCEchoObject, fArr).get();
    }

    UVariant EchoDoubleArray(const UVariant &dArr) {
        return async<UVariant>(idEchoDoubleArrayCEchoObject, dArr).get();
    }

    UVariant EchoStringArray(const UVariant &strArr) {
        return async<UVariant>(idEchoStringArrayCEchoObject, strArr).get();
    }

    UVariant EchoDecimalArray(const UVariant &decArr) {
        return async<UVariant>(idEchoDecimalArrayCEchoObject, decArr).get();
    }

    UVariant EchoUUID(const UVariant &clsid) {
        return async<UVariant>(idEchoUUIDCEchoObject, clsid).get();
    }

    UVariant EchoUUIDArray(const UVariant &clsidArr) {
        return async<UVariant>(idEchoUUIDArrayCEchoObject, clsidArr).get();
    }

    UVariant EchoCY(const UVariant &cy) {
        return async<UVariant>(idEchoCYCEchoObject, cy).get();
    }

    UVariant EchoCYArray(const UVariant &cyArray) {
        return async<UVariant>(idEchoCYArrayCEchoObject, cyArray).get();
    }

    UVariant EchoDateTime(const UVariant &dt) {
        return async<UVariant>(idEchoDateTimeCEchoObject, dt).get();
    }

    UVariant EchoDateTimeArray(const UVariant &dtArr) {
        return async<UVariant>(idEchoDateTimeArrayCEchoObject, dtArr).get();
    }
};
#endif
