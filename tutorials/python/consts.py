from spa import BaseServiceID, tagBaseRequestID


class hwConst:
    sidHelloWorld = BaseServiceID.sidReserved + 1
    idSayHello = tagBaseRequestID.idReservedTwo + 1
    idSleep = idSayHello + 1
    idEcho = idSleep + 1


class piConst:
    sidPi = BaseServiceID.sidReserved + 5
    sidPiWorker = sidPi + 1
    idComputePi = tagBaseRequestID.idReservedTwo + 1