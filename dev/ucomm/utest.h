#ifndef	__UMB_TEST_INCLUDE_HEADER__
#define __UMB_TEST_INCLUDE_HEADER__

#define sidTestService 	((unsigned int)SPA::tagServiceID::sidReserved + 20)
#define idSleep		((unsigned short)SPA::tagBaseRequestID::idReservedTwo + 1)
#define idEcho		(idSleep + 1)
#define idOpenDb	(idEcho + 1)
#define idBadRequest	(idOpenDb + 1)
#define idDoRequest0	(idBadRequest + 1)
#define idDoRequest1	(idDoRequest0 + 1)
#define idDoRequest2	(idDoRequest1 + 1)
#define idDoRequest3	(idDoRequest2 + 1)
#define idDoRequest4	(idDoRequest3 + 1)
#define idDequeue	(idDoRequest4 + 1)
#define idDoIdle	(idDequeue + 1)

#endif