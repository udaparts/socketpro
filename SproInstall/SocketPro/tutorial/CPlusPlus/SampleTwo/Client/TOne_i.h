#ifndef ___SOCKETPRO_DEFINES_TONE_I_H__
#define ___SOCKETPRO_DEFINES_TONE_I_H__

//defines for service CTOne
#define sidCTOne	(odUserServiceIDMin + 20)

#define idQueryCountCTOne	(odUserRequestIDMin + 0)
#define idQueryGlobalCountCTOne	(idQueryCountCTOne + 1)
#define idQueryGlobalFastCountCTOne	(idQueryGlobalCountCTOne + 1)
#define idSleepCTOne	(idQueryGlobalFastCountCTOne + 1)
#define idEchoCTOne	(idSleepCTOne + 1)



#endif