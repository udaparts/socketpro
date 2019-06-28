#ifndef ___SOCKETPRO_SERVICES_IMPL_TECHOCIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_TECHOCIMPL_H__

#include "../include/aserverw.h"
using namespace SPA;
using namespace SPA::ServerSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../TEchoC_i.h"

//server implementation for service CEchoObject

class CEchoObjectPeer : public CClientPeer {
protected:

    virtual void OnSwitchFrom(unsigned int serviceId) {
        //initialize the object here
    }

    virtual void OnReleaseResource(bool closing, unsigned int info) {
        if (closing) {
            //closing the socket with error code = info
        } else {
            //switch to a new service with the service id = info
        }

        //release all of your resources here as early as possible
    }

    void EchoDateTime(const UVariant &dt, UVariant &dtRtn) {
        assert(SPA::Map2VarintType(dt) == VT_DATE);
        dtRtn = dt;
    }

    void EchoDateTimeArray(const UVariant &dtArr, UVariant &dtArrRtn) {
        assert(SPA::Map2VarintType(dtArr) == (VT_DATE | VT_ARRAY));
        dtArrRtn = dtArr;
    }

    void EchoEmpty(const UVariant &empty, /*out*/UVariant &EchoEmptyRtn) {
        assert(SPA::Map2VarintType(empty) == VT_EMPTY);
        EchoEmptyRtn = empty;
    }

    void EchoBool(const UVariant &b, /*out*/UVariant &EchoBoolRtn) {
        assert(SPA::Map2VarintType(b) == VT_BOOL);
        EchoBoolRtn = b;
    }

    void EchoInt8(const UVariant &c, /*out*/UVariant &EchoInt8Rtn) {
        assert(SPA::Map2VarintType(c) == VT_I1);
        EchoInt8Rtn = c;
    }

    void EchoUInt8(const UVariant &b, /*out*/UVariant &EchoUInt8Rtn) {
        assert(SPA::Map2VarintType(b) == VT_UI1);
        EchoUInt8Rtn = b;
    }

    void EchoInt16(const UVariant &s, /*out*/UVariant &EchoInt16Rtn) {
        assert(SPA::Map2VarintType(s) == VT_I2);
        EchoInt16Rtn = s;
    }

    void EchoUInt16(const UVariant &s, /*out*/UVariant &EchoUInt16Rtn) {
        assert(SPA::Map2VarintType(s) == VT_UI2);
        EchoUInt16Rtn = s;
    }

    void EchoInt32(const UVariant &data, /*out*/UVariant &EchoInt32Rtn) {
        assert(SPA::Map2VarintType(data) == VT_I4 || SPA::Map2VarintType(data) == VT_INT);
        EchoInt32Rtn = data;
    }

    void EchoUInt32(const UVariant &data, /*out*/UVariant &EchoUInt32Rtn) {
        assert(SPA::Map2VarintType(data) == VT_UI4 || SPA::Map2VarintType(data) == VT_UINT);
        EchoUInt32Rtn = data;
    }

    void EchoInt64(const UVariant &data, /*out*/UVariant &EchoInt64Rtn) {
        assert(SPA::Map2VarintType(data) == VT_I8);
        EchoInt64Rtn = data;
    }

    void EchoUInt64(const UVariant &data, /*out*/UVariant &EchoUInt64Rtn) {
        assert(SPA::Map2VarintType(data) == VT_UI8);
        EchoUInt64Rtn = data;
    }

    void EchoFloat(const UVariant &data, /*out*/UVariant &EchoFloatRtn) {
        assert(SPA::Map2VarintType(data) == VT_R4);
        EchoFloatRtn = data;
    }

    void EchoDouble(const UVariant &data, /*out*/UVariant &EchoDoubleRtn) {
        assert(SPA::Map2VarintType(data) == VT_R8);
        EchoDoubleRtn = data;
    }

    void EchoString(const UVariant &str, /*out*/UVariant &EchoStringRtn) {
        assert(SPA::Map2VarintType(str) == VT_BSTR);
        EchoStringRtn = str;
    }

    void EchoAString(const UVariant &str, /*out*/UVariant &EchoAStringRtn) {
        assert(SPA::Map2VarintType(str) == (VT_I1 | VT_ARRAY));
        EchoAStringRtn = str;
    }

    void EchoDecimal(const UVariant &dec, /*out*/UVariant &EchoDecimalRtn) {
        assert(SPA::Map2VarintType(dec) == VT_DECIMAL);
        EchoDecimalRtn = dec;
    }

    void EchoBoolArray(const UVariant &bArr, /*out*/UVariant &EchoBoolArrayRtn) {
        assert(SPA::Map2VarintType(bArr) == (VT_BOOL | VT_ARRAY));
        EchoBoolArrayRtn = bArr;
    }

    void EchoUInt8Array(const UVariant &bArr, /*out*/UVariant &EchoUInt8ArrayRtn) {
        assert(SPA::Map2VarintType(bArr) == (VT_UI1 | VT_ARRAY));
        EchoUInt8ArrayRtn = bArr;
    }

    void EchoInt16Array(const UVariant &shortArr, /*out*/UVariant &EchoInt16ArrayRtn) {
        assert(SPA::Map2VarintType(shortArr) == (VT_I2 | VT_ARRAY));
        EchoInt16ArrayRtn = shortArr;
    }

    void EchoUInt16Array(const UVariant &ushortArr, /*out*/UVariant &EchoUInt16ArrayRtn) {
        assert(SPA::Map2VarintType(ushortArr) == (VT_UI2 | VT_ARRAY));
        EchoUInt16ArrayRtn = ushortArr;
    }

    void EchoInt32Array(const UVariant &intArr, /*out*/UVariant &EchoInt32ArrayRtn) {
        assert(SPA::Map2VarintType(intArr) == (VT_I4 | VT_ARRAY) || SPA::Map2VarintType(intArr) == (VT_INT | VT_ARRAY));
        EchoInt32ArrayRtn = intArr;
    }

    void EchoUInt32Array(const UVariant &uintArr, /*out*/UVariant &EchoUInt32ArrayRtn) {
        assert(SPA::Map2VarintType(uintArr) == (VT_UI4 | VT_ARRAY) || SPA::Map2VarintType(uintArr) == (VT_UINT | VT_ARRAY));
        EchoUInt32ArrayRtn = uintArr;
    }

    void EchoInt64Array(const UVariant &longArr, /*out*/UVariant &EchoInt64ArrayRtn) {
        assert(SPA::Map2VarintType(longArr) == (VT_I8 | VT_ARRAY));
        EchoInt64ArrayRtn = longArr;
    }

    void EchoUInt64Array(const UVariant &ulongArr, /*out*/UVariant &EchoUInt64ArrayRtn) {
        assert(SPA::Map2VarintType(ulongArr) == (VT_UI8 | VT_ARRAY));
        EchoUInt64ArrayRtn = ulongArr;
    }

    void EchoFloatArray(const UVariant &fArr, /*out*/UVariant &EchoFloatArrayRtn) {
        assert(SPA::Map2VarintType(fArr) == (VT_R4 | VT_ARRAY));
        EchoFloatArrayRtn = fArr;
    }

    void EchoDoubleArray(const UVariant &dArr, /*out*/UVariant &EchoDoubleArrayRtn) {
        assert(SPA::Map2VarintType(dArr) == (VT_R8 | VT_ARRAY));
        EchoDoubleArrayRtn = dArr;
    }

    void EchoStringArray(const UVariant &strArr, /*out*/UVariant &EchoStringArrayRtn) {
        assert(SPA::Map2VarintType(strArr) == (VT_BSTR | VT_ARRAY));
        EchoStringArrayRtn = strArr;
    }

    void EchoDecimalArray(const UVariant &decArr, /*out*/UVariant &EchoDecimalArrayRtn) {
        assert(SPA::Map2VarintType(decArr) == (VT_DECIMAL | VT_ARRAY));
        EchoDecimalArrayRtn = decArr;
    }

    void EchoCY(const UVariant &cy, /*out*/UVariant &EchoCYRtn) {
        assert(SPA::Map2VarintType(cy) == VT_CY || SPA::Map2VarintType(cy) == VT_R8);
        EchoCYRtn = cy;
    }

    void EchoCYArray(const UVariant &cyArray, /*out*/UVariant &EchoCYArrayRtn) {
        assert(SPA::Map2VarintType(cyArray) == (VT_CY | VT_ARRAY) || SPA::Map2VarintType(cyArray) == (VT_R8 | VT_ARRAY));
        EchoCYArrayRtn = cyArray;
    }

    void EchoUUID(const UVariant &clsid, /*out*/UVariant &EchoUUIDRtn) {
        assert(SPA::Map2VarintType(clsid) == VT_CLSID);
        EchoUUIDRtn = clsid;
    }

    void EchoUUIDArray(const UVariant &clsidArr, /*out*/UVariant &EchoUUIDArrayRtn) {
        assert(SPA::Map2VarintType(clsidArr) == (VT_CLSID | VT_ARRAY));
        EchoUUIDArrayRtn = clsidArr;
    }

    virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len) {
        BEGIN_SWITCH(reqId)
        M_I1_R1(idEchoDateTimeCEchoObject, EchoDateTime, UVariant, UVariant)
        M_I1_R1(idEchoDateTimeArrayCEchoObject, EchoDateTimeArray, UVariant, UVariant)
        M_I1_R1(idEchoEmptyCEchoObject, EchoEmpty, UVariant, UVariant)
        M_I1_R1(idEchoBoolCEchoObject, EchoBool, UVariant, UVariant)
        M_I1_R1(idEchoInt8CEchoObject, EchoInt8, UVariant, UVariant)
        M_I1_R1(idEchoUInt8CEchoObject, EchoUInt8, UVariant, UVariant)
        M_I1_R1(idEchoInt16CEchoObject, EchoInt16, UVariant, UVariant)
        M_I1_R1(idEchoUInt16CEchoObject, EchoUInt16, UVariant, UVariant)
        M_I1_R1(idEchoInt32CEchoObject, EchoInt32, UVariant, UVariant)
        M_I1_R1(idEchoUInt32CEchoObject, EchoUInt32, UVariant, UVariant)
        M_I1_R1(idEchoInt64CEchoObject, EchoInt64, UVariant, UVariant)
        M_I1_R1(idEchoUInt64CEchoObject, EchoUInt64, UVariant, UVariant)
        M_I1_R1(idEchoFloatCEchoObject, EchoFloat, UVariant, UVariant)
        M_I1_R1(idEchoDoubleCEchoObject, EchoDouble, UVariant, UVariant)
        M_I1_R1(idEchoStringCEchoObject, EchoString, UVariant, UVariant)
        M_I1_R1(idEchoAStringCEchoObject, EchoAString, UVariant, UVariant)
        M_I1_R1(idEchoDecimalCEchoObject, EchoDecimal, UVariant, UVariant)
        M_I1_R1(idEchoBoolArrayCEchoObject, EchoBoolArray, UVariant, UVariant)
        M_I1_R1(idEchoUInt8ArrayCEchoObject, EchoUInt8Array, UVariant, UVariant)
        M_I1_R1(idEchoInt16ArrayCEchoObject, EchoInt16Array, UVariant, UVariant)
        M_I1_R1(idEchoUInt16ArrayCEchoObject, EchoUInt16Array, UVariant, UVariant)
        M_I1_R1(idEchoInt32ArrayCEchoObject, EchoInt32Array, UVariant, UVariant)
        M_I1_R1(idEchoUInt32ArrayCEchoObject, EchoUInt32Array, UVariant, UVariant)
        M_I1_R1(idEchoInt64ArrayCEchoObject, EchoInt64Array, UVariant, UVariant)
        M_I1_R1(idEchoUInt64ArrayCEchoObject, EchoUInt64Array, UVariant, UVariant)
        M_I1_R1(idEchoFloatArrayCEchoObject, EchoFloatArray, UVariant, UVariant)
        M_I1_R1(idEchoDoubleArrayCEchoObject, EchoDoubleArray, UVariant, UVariant)
        M_I1_R1(idEchoStringArrayCEchoObject, EchoStringArray, UVariant, UVariant)
        M_I1_R1(idEchoDecimalArrayCEchoObject, EchoDecimalArray, UVariant, UVariant)
        M_I1_R1(idEchoUUIDCEchoObject, EchoUUID, UVariant, UVariant)
        M_I1_R1(idEchoUUIDArrayCEchoObject, EchoUUIDArray, UVariant, UVariant)
        M_I1_R1(idEchoCYCEchoObject, EchoCY, UVariant, UVariant)
        M_I1_R1(idEchoCYArrayCEchoObject, EchoCYArray, UVariant, UVariant)
        END_SWITCH
    }

    virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
        BEGIN_SWITCH(reqId)
        END_SWITCH
        return 0;
    }

};


#endif