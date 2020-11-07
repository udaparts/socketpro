#ifndef ___SOCKETPRO_DEFINES_HW_I_H__
#define ___SOCKETPRO_DEFINES_HW_I_H__

//defines for service HelloWorld
#define sidHelloWorld	((unsigned int)SPA::tagServiceID::sidReserved + 1)

#define idSayHello	((unsigned short)SPA::tagBaseRequestID::idReservedTwo + 1)
#define idSleep	(idSayHello + 1)
#define idEcho (idSleep + 1)

static std::string ToString(const unsigned int *groups, int count) {
    int n;
    if (!groups)
        count = 0;
    std::string s = "[";
    for (n = 0; n < count; ++n) {
        if (n != 0)
            s += ", ";
        s += std::to_string((SPA::UINT64)(groups[n]));
        ++n;
    }
    s += "]";
    return s;
}

#endif