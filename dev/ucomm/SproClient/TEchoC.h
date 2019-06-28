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
        UVariant EchoEmptyRtn;
        bool bProcessRy = ProcessR1(idEchoEmptyCEchoObject, empty, EchoEmptyRtn);
        return EchoEmptyRtn;
    }

    UVariant EchoBool(const UVariant &b) {
        UVariant EchoBoolRtn;
        bool bProcessRy = ProcessR1(idEchoBoolCEchoObject, b, EchoBoolRtn);
        return EchoBoolRtn;
    }

    UVariant EchoInt8(const UVariant &c) {
        UVariant EchoInt8Rtn;
        bool bProcessRy = ProcessR1(idEchoInt8CEchoObject, c, EchoInt8Rtn);
        return EchoInt8Rtn;
    }

    UVariant EchoUInt8(const UVariant &b) {
        UVariant EchoUInt8Rtn;
        bool bProcessRy = ProcessR1(idEchoUInt8CEchoObject, b, EchoUInt8Rtn);
        return EchoUInt8Rtn;
    }

    UVariant EchoInt16(const UVariant &s) {
        UVariant EchoInt16Rtn;
        bool bProcessRy = ProcessR1(idEchoInt16CEchoObject, s, EchoInt16Rtn);
        return EchoInt16Rtn;
    }

    UVariant EchoUInt16(const UVariant &s) {
        UVariant EchoUInt16Rtn;
        bool bProcessRy = ProcessR1(idEchoUInt16CEchoObject, s, EchoUInt16Rtn);
        return EchoUInt16Rtn;
    }

    UVariant EchoInt32(const UVariant &data) {
        UVariant EchoInt32Rtn;
        bool bProcessRy = ProcessR1(idEchoInt32CEchoObject, data, EchoInt32Rtn);
        return EchoInt32Rtn;
    }

    UVariant EchoUInt32(const UVariant &data) {
        UVariant EchoUInt32Rtn;
        bool bProcessRy = ProcessR1(idEchoUInt32CEchoObject, data, EchoUInt32Rtn);
        return EchoUInt32Rtn;
    }

    UVariant EchoInt64(const UVariant &data) {
        UVariant EchoInt64Rtn;
        bool bProcessRy = ProcessR1(idEchoInt64CEchoObject, data, EchoInt64Rtn);
        return EchoInt64Rtn;
    }

    UVariant EchoUInt64(const UVariant &data) {
        UVariant EchoUInt64Rtn;
        bool bProcessRy = ProcessR1(idEchoUInt64CEchoObject, data, EchoUInt64Rtn);
        return EchoUInt64Rtn;
    }

    UVariant EchoFloat(const UVariant &data) {
        UVariant EchoFloatRtn;
        bool bProcessRy = ProcessR1(idEchoFloatCEchoObject, data, EchoFloatRtn);
        return EchoFloatRtn;
    }

    UVariant EchoDouble(const UVariant &data) {
        UVariant EchoDoubleRtn;
        bool bProcessRy = ProcessR1(idEchoDoubleCEchoObject, data, EchoDoubleRtn);
        return EchoDoubleRtn;
    }

    UVariant EchoString(const UVariant &str) {
        UVariant EchoStringRtn;
        bool bProcessRy = ProcessR1(idEchoStringCEchoObject, str, EchoStringRtn);
        return EchoStringRtn;
    }

    UVariant EchoAString(const UVariant &str) {
        UVariant EchoAStringRtn;
        bool bProcessRy = ProcessR1(idEchoAStringCEchoObject, str, EchoAStringRtn);
        return EchoAStringRtn;
    }

    UVariant EchoDecimal(const UVariant &dec) {
        UVariant EchoDecimalRtn;
        bool bProcessRy = ProcessR1(idEchoDecimalCEchoObject, dec, EchoDecimalRtn);
        return EchoDecimalRtn;
    }

    UVariant EchoBoolArray(const UVariant &bArr) {
        UVariant EchoBoolArrayRtn;
        bool bProcessRy = ProcessR1(idEchoBoolArrayCEchoObject, bArr, EchoBoolArrayRtn);
        return EchoBoolArrayRtn;
    }

    UVariant EchoUInt8Array(const UVariant &bArr) {
        UVariant EchoUInt8ArrayRtn;
        bool bProcessRy = ProcessR1(idEchoUInt8ArrayCEchoObject, bArr, EchoUInt8ArrayRtn);
        return EchoUInt8ArrayRtn;
    }

    UVariant EchoInt16Array(const UVariant &shortArr) {
        UVariant EchoInt16ArrayRtn;
        bool bProcessRy = ProcessR1(idEchoInt16ArrayCEchoObject, shortArr, EchoInt16ArrayRtn);
        return EchoInt16ArrayRtn;
    }

    UVariant EchoUInt16Array(const UVariant &ushortArr) {
        UVariant EchoUInt16ArrayRtn;
        bool bProcessRy = ProcessR1(idEchoUInt16ArrayCEchoObject, ushortArr, EchoUInt16ArrayRtn);
        return EchoUInt16ArrayRtn;
    }

    UVariant EchoInt32Array(const UVariant &intArr) {
        UVariant EchoInt32ArrayRtn;
        bool bProcessRy = ProcessR1(idEchoInt32ArrayCEchoObject, intArr, EchoInt32ArrayRtn);
        return EchoInt32ArrayRtn;
    }

    UVariant EchoUInt32Array(const UVariant &uintArr) {
        UVariant EchoUInt32ArrayRtn;
        bool bProcessRy = ProcessR1(idEchoUInt32ArrayCEchoObject, uintArr, EchoUInt32ArrayRtn);
        return EchoUInt32ArrayRtn;
    }

    UVariant EchoInt64Array(const UVariant &longArr) {
        UVariant EchoInt64ArrayRtn;
        bool bProcessRy = ProcessR1(idEchoInt64ArrayCEchoObject, longArr, EchoInt64ArrayRtn);
        return EchoInt64ArrayRtn;
    }

    UVariant EchoUInt64Array(const UVariant &ulongArr) {
        UVariant EchoUInt64ArrayRtn;
        bool bProcessRy = ProcessR1(idEchoUInt64ArrayCEchoObject, ulongArr, EchoUInt64ArrayRtn);
        return EchoUInt64ArrayRtn;
    }

    UVariant EchoFloatArray(const UVariant &fArr) {
        UVariant EchoFloatArrayRtn;
        bool bProcessRy = ProcessR1(idEchoFloatArrayCEchoObject, fArr, EchoFloatArrayRtn);
        return EchoFloatArrayRtn;
    }

    UVariant EchoDoubleArray(const UVariant &dArr) {
        UVariant EchoDoubleArrayRtn;
        bool bProcessRy = ProcessR1(idEchoDoubleArrayCEchoObject, dArr, EchoDoubleArrayRtn);
        return EchoDoubleArrayRtn;
    }

    UVariant EchoStringArray(const UVariant &strArr) {
        UVariant EchoStringArrayRtn;
        bool bProcessRy = ProcessR1(idEchoStringArrayCEchoObject, strArr, EchoStringArrayRtn);
        return EchoStringArrayRtn;
    }

    UVariant EchoDecimalArray(const UVariant &decArr) {
        UVariant EchoDecimalArrayRtn;
        bool bProcessRy = ProcessR1(idEchoDecimalArrayCEchoObject, decArr, EchoDecimalArrayRtn);
        return EchoDecimalArrayRtn;
    }

    UVariant EchoUUID(const UVariant &clsid) {
        UVariant EchoUUIDRtn;
        bool bProcessRy = ProcessR1(idEchoUUIDCEchoObject, clsid, EchoUUIDRtn);
        return EchoUUIDRtn;
    }

    UVariant EchoUUIDArray(const UVariant &clsidArr) {
        UVariant EchoUUIDArrayRtn;
        bool bProcessRy = ProcessR1(idEchoUUIDArrayCEchoObject, clsidArr, EchoUUIDArrayRtn);
        return EchoUUIDArrayRtn;
    }

    UVariant EchoCY(const UVariant &cy) {
        UVariant EchoCYRtn;
        bool bProcessRy = ProcessR1(idEchoCYCEchoObject, cy, EchoCYRtn);
        return EchoCYRtn;
    }

    UVariant EchoCYArray(const UVariant &cyArray) {
        UVariant EchoCYArrayRtn;
        bool bProcessRy = ProcessR1(idEchoCYArrayCEchoObject, cyArray, EchoCYArrayRtn);
        return EchoCYArrayRtn;
    }

    UVariant EchoDateTime(const UVariant &dt) {
        UVariant EchoDateTimeRtn;
        bool bProcessRy = ProcessR1(idEchoDateTimeCEchoObject, dt, EchoDateTimeRtn);
        return EchoDateTimeRtn;
    }

    UVariant EchoDateTimeArray(const UVariant &dtArr) {
        UVariant EchoDateTimeArrayRtn;
        bool bProcessRy = ProcessR1(idEchoDateTimeArrayCEchoObject, dtArr, EchoDateTimeArrayRtn);
        return EchoDateTimeArrayRtn;
    }
};
#endif
