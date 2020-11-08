#ifndef ___SOCKETPRO_DEFINES_TONE_I_H__
#define ___SOCKETPRO_DEFINES_TONE_I_H__

//defines for service CTOne
#define sidCTOne	((unsigned int)SPA::tagServiceID::sidReserved + 30)

#define idQueryCountCTOne	((unsigned short)SPA::tagBaseRequestID::idReservedTwo + 1)
#define idQueryGlobalCountCTOne	(idQueryCountCTOne + 1)
#define idQueryGlobalFastCountCTOne	(idQueryGlobalCountCTOne + 1)
#define idSleepCTOne	(idQueryGlobalFastCountCTOne + 1)
#define idEchoCTOne	(idSleepCTOne + 1)
#define idEchoExCTOne	(idEchoCTOne + 1)


#endif