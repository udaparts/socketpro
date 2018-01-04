
from spa import BaseServiceID, tagBaseRequestID

class hwConst:
    sidHelloWorld = BaseServiceID.sidReserved + 1
    idSayHelloHelloWorld = tagBaseRequestID.idReservedTwo + 1
    idSleepHelloWorld = idSayHelloHelloWorld + 1
    idEchoHelloWorld = idSleepHelloWorld + 1

class piConst:
    sidPi = BaseServiceID.sidReserved + 5
    sidPiWorker = sidPi + 1
    idComputePi = tagBaseRequestID.idReservedTwo + 1