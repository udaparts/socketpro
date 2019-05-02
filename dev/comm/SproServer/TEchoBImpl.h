#ifndef ___SOCKETPRO_SERVICES_IMPL_TECHOBIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_TECHOBIMPL_H__

#include "../../include/aserverw.h" 

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../TEchoB_i.h"

using namespace SPA; //SocketProAdapter --> SPA
using namespace SPA::ServerSide;

//server implementation for service CEchoBasic

class CEchoBasicPeer : public CClientPeer {
protected:

    virtual void OnSwitchFrom(unsigned int ulServiceID) {
        //initialize the object here
    }

    virtual void OnReleaseResource(bool bClosing, unsigned int ulInfo) {
        if (bClosing) {
            //closing the socket with error code = ulInfo
        } else {
            //switch to a new service with the service id = ulInfo
        }

        //release all of your resources here as early as possible
    }

    void EchoBool(bool b, /*out*/bool &EchoBoolRtn) {
        EchoBoolRtn = b;
    }

    void EchoDateTime(const SPA::UDateTime &dt, SPA::UDateTime &dtRtn) {
        dtRtn = dt;
    }

    void EchoInt8(char c, /*out*/char &EchoInt8Rtn) {
        EchoInt8Rtn = c;
    }

    void EchoUInt8(unsigned char b, /*out*/unsigned char &EchoUInt8Rtn) {
        EchoUInt8Rtn = b;
    }

    void EchoInt16(short s, /*out*/short &EchoInt16Rtn) {
        EchoInt16Rtn = s;
    }

    void EchoUInt16(unsigned short s, /*out*/unsigned short &EchoUInt16Rtn) {
        EchoUInt16Rtn = s;
    }

    void EchoInt32(int data, /*out*/int &EchoInt32Rtn) {
        EchoInt32Rtn = data;
    }

    void EchoUInt32(unsigned int data, /*out*/unsigned int &EchoUInt32Rtn) {
        EchoUInt32Rtn = data;
    }

    void EchoInt64(SPA::INT64 data, /*out*/SPA::INT64 &EchoInt64Rtn) {
        EchoInt64Rtn = data;
    }

    void EchoUInt64(SPA::UINT64 data, /*out*/SPA::UINT64 &EchoUInt64Rtn) {
        EchoUInt64Rtn = data;
    }

    void EchoFloat(float data, /*out*/float &EchoFloatRtn) {
        EchoFloatRtn = data;
    }

    void EchoDouble(double data, /*out*/double &EchoDoubleRtn) {
        EchoDoubleRtn = data;
    }

    void EchoString(const std::wstring &str, /*out*/std::wstring &EchoStringRtn) {
        if (str.empty())
            std::cout << "empty wchar string" << std::endl;
        EchoStringRtn = str;
    }

    void EchoAString(const std::string &str, /*out*/std::string &EchoAStringRtn) {
        EchoAStringRtn = str;
    }

    void EchoDecimal(DECIMAL dec, /*out*/DECIMAL &EchoDecimalRtn) {
        EchoDecimalRtn = dec;
    }

    void EchoWChar(wchar_t wc, /*out*/wchar_t &EchoWCharRtn) {
        EchoWCharRtn = wc;
    }

    void EchoGuid(const GUID &guid, /*out*/GUID &EchoGuidRtn) {
        EchoGuidRtn = guid;
    }

    void EchoCy(const CY &cy, /*out*/CY &EchoCyRtn) {
        EchoCyRtn = cy;
    }

    virtual void OnFastRequestArrive(unsigned short usRequestID, unsigned int ulLen) {
        BEGIN_SWITCH(usRequestID)
        M_I1_R1(idEchoBoolCEchoBasic, EchoBool, bool, bool)
        M_I1_R1(idEchoInt8CEchoBasic, EchoInt8, char, char)
        M_I1_R1(idEchoUInt8CEchoBasic, EchoUInt8, unsigned char, unsigned char)
        M_I1_R1(idEchoInt16CEchoBasic, EchoInt16, short, short)
        M_I1_R1(idEchoUInt16CEchoBasic, EchoUInt16, unsigned short, unsigned short)
        M_I1_R1(idEchoInt32CEchoBasic, EchoInt32, int, int)
        M_I1_R1(idEchoUInt32CEchoBasic, EchoUInt32, unsigned int, unsigned int)
        M_I1_R1(idEchoInt64CEchoBasic, EchoInt64, SPA::INT64, SPA::INT64)
        M_I1_R1(idEchoUInt64CEchoBasic, EchoUInt64, SPA::UINT64, SPA::UINT64)
        M_I1_R1(idEchoFloatCEchoBasic, EchoFloat, float, float)
        M_I1_R1(idEchoDoubleCEchoBasic, EchoDouble, double, double)
        M_I1_R1(idEchoStringCEchoBasic, EchoString, std::wstring, std::wstring)
        M_I1_R1(idEchoAStringCEchoBasic, EchoAString, std::string, std::string)
        M_I1_R1(idEchoDecimalCEchoBasic, EchoDecimal, DECIMAL, DECIMAL)
        M_I1_R1(idEchoWCharCEchoBasic, EchoWChar, wchar_t, wchar_t)
        M_I1_R1(idEchoGuidCEchoBasic, EchoGuid, GUID, GUID)
        M_I1_R1(idEchoCyCEchoBasic, EchoCy, CY, CY)
        M_I1_R1(idEchoDateTime, EchoDateTime, SPA::UDateTime, SPA::UDateTime)
        END_SWITCH
    }

    virtual int OnSlowRequestArrive(unsigned short usRequestID, unsigned int ulLen) {
        BEGIN_SWITCH(usRequestID)
        END_SWITCH
        return 0; //S_OK --> 0
    }

};

#endif