#ifndef ___SOCKETPRO_DEFINES_REMOTESUM_I_H__
#define ___SOCKETPRO_DEFINES_REMOTESUM_I_H__

//defines for service RemSum
#define sidRemSum			(odUserServiceIDMin + 10)

#define idDoSumRemSum		(odUserRequestIDMin + 0)
#define idPauseRemSum		(idDoSumRemSum + 1)
#define idRedoSumRemSum		(idPauseRemSum + 1)
#define idReportProgress	(idRedoSumRemSum + 1)

#endif