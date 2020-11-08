#ifndef ___SOCKETPRO_DEFINES_TECHOD_I_H__
#define ___SOCKETPRO_DEFINES_TECHOD_I_H__

//defines for service CEchoSys
const unsigned int sidCEchoSys = ((unsigned int)SPA::tagServiceID::sidReserved + 3);
const unsigned int sidRouteSvs0 = ((unsigned int)SPA::tagServiceID::sidReserved + 120);
const unsigned int sidRouteSvs1 = (sidRouteSvs0 + 1);

const unsigned short idEchoMyStructCEchoSys = ((unsigned short)SPA::tagBaseRequestID::idReservedTwo + 1);
const unsigned short idEchoUQueueCEchoSys = (idEchoMyStructCEchoSys + 1);
const unsigned short idEchoComplex0CEchoSys = (idEchoUQueueCEchoSys + 1);

const unsigned short idREcho0 = (idEchoComplex0CEchoSys + 1);
const unsigned short idREcho1 = (idREcho0 + 1);
const unsigned short idRoutorClientCount = (idREcho1 + 1);
const unsigned short idCheckRouteeServiceId = (idRoutorClientCount + 1);

struct MyStruct {
    std::string AString;
    bool ABool;
    std::wstring WString;
    unsigned int AInt;

    bool operator ==(const MyStruct & ms) const {
        return (AString == ms.AString && ABool == ms.ABool && WString == ms.WString && AInt == ms.AInt);
    }
};

SPA::CUQueue& operator <<(SPA::CUQueue &q, const MyStruct &ms) {
    q << ms.AString << ms.ABool << ms.WString << ms.AInt;
    return q;
}

SPA::CUQueue& operator >>(SPA::CUQueue &q, MyStruct &ms) {
    q >> ms.AString >> ms.ABool >> ms.WString >> ms.AInt;
    return q;
}


#endif