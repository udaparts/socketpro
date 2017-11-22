from spa.clientside import CConnectionContext, CClientSocket, UFuture

from webasynchandler import CWebAsyncHandler
from sharedstruct import *

# CA file is located at the directory ../socketpro/bin
CClientSocket.SSL.SetVerifyLocation('ca.cert.pem')

with CMasterPool(CWebAsyncHandler, '', False) as sp:
    cc = CConnectionContext(input('Remote host: '), 20911, 'PythonUser', 'TooMuchSecret', tagEncryptionMethod.TLSv1)

    def ssl_server_authentication(pool, cs):
        cert = cs.UCert
        errCode, errMsg = cert.Verify()
        return errCode == 0

    sp.DoSslServerAuthentication = ssl_server_authentication

    ok = sp.StartSocketPool(cc, 4, 1)
    if not ok:
        input('Failed in connecting to remote middle tier server, and press any key to close the application ......')
        exit(-1)
    cache = sp.Cache
    filter = input('Sakila.payment filter: ')
    handler = sp.Seek()
    call_index = handler.GetMasterSlaveConnectedSessions(
        lambda index, m, s: print('master connections: %d, slave connections: %d' % (m, s)),
        lambda index: print('Socket closed after sending call %d' % index))

    def dMma(index, mma, res, errMsg):
        if res != 0:
            print('QueryPaymentMaxMinAvgs error code: %d, error message: %s' %(res, errMsg))
        else:
            print('QueryPaymentMaxMinAvgs max : %f, min: %f, avg: %f' % (mma.Max, mma.Min, mma.Avg))

    call_index = handler.QueryPaymentMaxMinAvgs(filter, dMma, lambda index: print('Socket closed after sending call %d' % index))
    ok = handler.WaitAll()
    vData = []
    vData.append(1)  #Google company id
    vData.append('Ted Cruz')
    vData.append(datetime.datetime.now())
    vData.append(1)  # Google company id
    vData.append('Donald Trump')
    vData.append(datetime.datetime.now())
    vData.append(2)  # Microsoft company id
    vData.append('Hillary Clinton')
    vData.append(datetime.datetime.now())
    f = UFuture()
    def dUe(index, res, errMsg, vId):
        if res != 0:
            print('UploadEmployees Error code = %d, error message = %s' % (res, errMsg))
        else:
            for d in vId:
                print('Last id: ' + str(d))
        f.set(True)
    def dClosed(index):
        print('Socket closed after sending call %d' % index)
        f.set(True)
    call_index = handler.UploadEmployees(vData, dUe, dClosed)
    if call_index != 0:
        if not f.get(5):
            print('The above requests are not completed in 5 seconds')
        else:
            print('The above requests are completed in 5 seconds')
    else:
        print('Socket already closed before sending request')

    input('Press ENTER key to test random returning ......')
    sum_mma = CMaxMinAvg()
    def dQmma(index, mma, res, errMsg):
        if res != 0:
            print('QueryPaymentMaxMinAvgs call index: %d, error code: %d, error message: %s' % (index, res, errMsg))
        else:
            sum_mma.Avg += mma.Avg
            sum_mma.Max += mma.Max
            sum_mma.Min += mma.Min
            # print('QueryPaymentMaxMinAvgs call index = %d' % index)

    start = datetime.datetime.now()
    cycles = 10000
    while cycles > 0:
        call_index = handler.QueryPaymentMaxMinAvgs(filter, dQmma)
        cycles -= 1
    ok = handler.WaitAll()
    print('Time required: %f seconds for %d requests' % ((datetime.datetime.now() - start).total_seconds(), 10000))
    print('QueryPaymentMaxMinAvgs sum_max: %f, sum_min: %f, sum_avg: %f' % (sum_mma.Max, sum_mma.Min, sum_mma.Avg))

    input('Press ENTER key to test sequence returning ......')
    def dRdt(index, dates, res, errMsg):
        if res != 0:
            print('GetRentalDateTimes call index: %d, error code: %d, error message: %s' % (index, res, errMsg))
        elif dates.rental_id == 0:
            print('GetRentalDateTimes call index: %d, rental_id=%d not available' % (index, dates.rental_id))
        else:
            print('GetRentalDateTimes call index: %d rental_id=%d and dates (%s, %s, %s)' % (index, dates.rental_id, str(dates.Rental), str(dates.Return), str(dates.LastUpdate)))
    cycles = 0
    while cycles < 1000:
        call_index = handler.GetRentalDateTimes(cycles + 1, dRdt)
        cycles += 1
    ok = handler.WaitAll()

    input('Press ENTER key to shutdown the demo application ......')
    res = 0

    