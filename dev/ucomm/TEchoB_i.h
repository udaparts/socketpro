#ifndef ___SOCKETPRO_DEFINES_TECHOB_I_H__
#define ___SOCKETPRO_DEFINES_TECHOB_I_H__

//defines for service CEchoBasic
#define sidCEchoBasic	((unsigned int)SPA::tagServiceID::sidReserved + 1)

#define idEchoBoolCEchoBasic	((unsigned short)SPA::tagBaseRequestID::idReservedTwo + 1)
#define idEchoInt8CEchoBasic	(idEchoBoolCEchoBasic + 1)
#define idEchoUInt8CEchoBasic	(idEchoInt8CEchoBasic + 1)
#define idEchoInt16CEchoBasic	(idEchoUInt8CEchoBasic + 1)
#define idEchoUInt16CEchoBasic	(idEchoInt16CEchoBasic + 1)
#define idEchoInt32CEchoBasic	(idEchoUInt16CEchoBasic + 1)
#define idEchoUInt32CEchoBasic	(idEchoInt32CEchoBasic + 1)
#define idEchoInt64CEchoBasic	(idEchoUInt32CEchoBasic + 1)
#define idEchoUInt64CEchoBasic	(idEchoInt64CEchoBasic + 1)
#define idEchoFloatCEchoBasic	(idEchoUInt64CEchoBasic + 1)
#define idEchoDoubleCEchoBasic	(idEchoFloatCEchoBasic + 1)
#define idEchoStringCEchoBasic	(idEchoDoubleCEchoBasic + 1)
#define idEchoAStringCEchoBasic	(idEchoStringCEchoBasic + 1)
#define idEchoDecimalCEchoBasic	(idEchoAStringCEchoBasic + 1)
#define idEchoWCharCEchoBasic	(idEchoDecimalCEchoBasic + 1)
#define idEchoGuidCEchoBasic	(idEchoWCharCEchoBasic + 1)
#define idEchoCyCEchoBasic	(idEchoGuidCEchoBasic + 1)
#define idEchoDateTime  (idEchoCyCEchoBasic + 1)


#endif