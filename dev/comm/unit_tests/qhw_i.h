#ifndef ___SOCKETPRO_DEFINES_QHW_I_H__
#define ___SOCKETPRO_DEFINES_QHW_I_H__

//defines for service HWReceiver
#define sidHWReceiver	(sidReserved + 124)

#define idDoDequeueHWReceiver	(SPA::idReservedTwo + 1)


//defines for service HWSender
#define sidHWSender	(sidReserved + 123)

#define idSayHelloWordHWSender	(idDoDequeueHWReceiver + 1)

#define idHWMessage (idSayHelloWordHWSender + 1)

#endif