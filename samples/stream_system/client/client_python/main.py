from spa.clientside import CConnectionContext, CClientSocket, UFuture

from sharedstruct import *
from webasynchandler import CWebAsyncHandler

# CA file is located at the directory ../socketpro/bin
CClientSocket.SSL.SetVerifyLocation('ca.cert.pem')

with CMasterPool(CWebAsyncHandler, '', False) as sp:
    cc = CConnectionContext(input('Remote middle tier host: '), 20911, 'PythonUser', 'TooMuchSecret', tagEncryptionMethod.TLSv1)

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
    ok = handler.GetMasterSlaveConnectedSessions(
        lambda m, s: print('master connections: %d, slave connections: %d' % (m, s)),
        lambda h, canceled: print('Connection closed after sending call'))

    def dMma(mma, res, errMsg):
        if res != 0:
            print('QueryPaymentMaxMinAvgs error code: %d, error message: %s' %(res, errMsg))
        else:
            print('QueryPaymentMaxMinAvgs max : %f, min: %f, avg: %f' % (mma.Max, mma.Min, mma.Avg))

    ok = handler.QueryPaymentMaxMinAvgs(filter, dMma, lambda h, canceled: print('Socket closed after sending call'))
    ok = handler.WaitAll()

    vData = []
    vData.append(1)  # Google company id
    vData.append('Ted Cruz')
    vData.append(datetime.datetime.now())
    vData.append(1)  # Google company id
    vData.append('Donald Trump')
    vData.append(datetime.datetime.now())
    vData.append(2)  # Microsoft company id
    vData.append('Hillary Clinton')
    vData.append(datetime.datetime.now())
    f = UFuture()
    def dUe(res, errMsg, vId):
        if res != 0:
            print('UploadEmployees Error code = %d, error message = %s' % (res, errMsg))
        else:
            for d in vId:
                print('Last id: ' + str(d))
        f.set(True)
    def dDiscarded(h, canceled):
        print('Connection closed after sending call')
        f.set(True)
    call_index = handler.UploadEmployees(vData, dUe, dDiscarded)
    if call_index != 0:
        if not f.get(5):
            print('The above requests are not completed in 5 seconds')
        else:
            print('The above requests are completed in 5 seconds')
    else:
        print('Socket already closed before sending request')

    input('Press ENTER key to test requests parallel processing and fault tolerance at server side ......')
    sum_mma = CMaxMinAvg()
    def dQmma(mma, res, errMsg):
        if res != 0:
            print('QueryPaymentMaxMinAvgs call error code: %d, error message: %s' % (res, errMsg))
        else:
            sum_mma.Avg += mma.Avg
            sum_mma.Max += mma.Max
            sum_mma.Min += mma.Min

    start = datetime.datetime.now()
    cycles = 10000
    while cycles > 0:
        ok = handler.QueryPaymentMaxMinAvgs(filter, dQmma)
        if not ok:
            print('Connection closed')
            break
        cycles -= 1
    ok = handler.WaitAll()
    print('Time required: %f seconds for %d requests' % ((datetime.datetime.now() - start).total_seconds(), 10000))
    print('QueryPaymentMaxMinAvgs sum_max: %f, sum_min: %f, sum_avg: %f' % (sum_mma.Max, sum_mma.Min, sum_mma.Avg))

    input('Press ENTER key to test requests server parallel processing, fault tolerance and sequence returning ......')
    handler.prev_rental_id = 0
    def dRdt(dates, res, errMsg):
        if res != 0:
            print('GetRentalDateTimes call error code: %d, error message: %s' % (res, errMsg))
            handler.prev_rental_id = 0
        elif dates.rental_id == 0:
            print('GetRentalDateTimes call rental_id=%d not available' % (dates.rental_id))
            handler.prev_rental_id = 0
        elif handler.prev_rental_id == 0 or dates.rental_id == handler.prev_rental_id + 1:
            print('GetRentalDateTimes call rental_id=%d and dates (%s, %s, %s)' % (dates.rental_id, str(dates.Rental), str(dates.Return), str(dates.LastUpdate)))
        else:
            print('****** GetRentalDateTimes returned out of order ******')
    cycles = 0
    while cycles < 1000:
        ok = handler.GetRentalDateTimes(cycles + 1, dRdt)
        if not ok:
            print('Connection closed')
            break
        cycles += 1
    ok = handler.WaitAll()
    input('Press ENTER key to shutdown the demo application ......')
    res = 0
