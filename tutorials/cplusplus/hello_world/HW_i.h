#ifndef ___SOCKETPRO_DEFINES_HW_I_H__
#define ___SOCKETPRO_DEFINES_HW_I_H__

//defines for service HelloWorld
#define sidHelloWorld	((unsigned int)SPA::tagServiceID::sidReserved + 1)

#define idSayHello ((unsigned short)tagBaseRequestID::idReservedTwo + 1)
#define idSleep (idSayHello + 1)
#define idEcho (idSleep + 1)

#endif