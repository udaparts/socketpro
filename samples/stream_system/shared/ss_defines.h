#ifndef ___SOCKETPRO_DEFINES_STREAM_SYSTEM_I_H__
#define ___SOCKETPRO_DEFINES_STREAM_SYSTEM_I_H__

//defines for service HelloWorld
#define sidStreamSystem	(SPA::sidReserved + 121)

#define idChunkClientToServer (SPA::idReservedTwo + 1)
#define idChunkServerToClient (idChunkClientToServer + 1)
#define idStartBlobClientToServer (idChunkServerToClient + 1)
#define idStartBlobServerToClient (idStartBlobClientToServer + 1)
#define idEndBlobClientToServer (idStartBlobServerToClient + 1)
#define idEndBlobServerToClient (idEndBlobClientToServer + 1)

#define idStartBatch (SPA::idReservedTwo + 20)


#endif