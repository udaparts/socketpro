#ifndef ___SOCKETPRO_DEFINES_TTHREE_I_H__
#define ___SOCKETPRO_DEFINES_TTHREE_I_H__

//defines for service CTThree
#define sidCTThree	(odUserServiceIDMin + 10)

#define idGetOneItemCTThree	(odUserRequestIDMin + 0)
#define idSendOneItemCTThree	(idGetOneItemCTThree + 1)
#define idGetManyItemsCTThree	(idSendOneItemCTThree + 1)
#define idSendManyItemsCTThree	(idGetManyItemsCTThree + 1)

#define idGetBatchItemsCTThree	(idSendManyItemsCTThree + 1)
#define idSendBatchItemsCTThree	(idGetBatchItemsCTThree + 1)

#endif