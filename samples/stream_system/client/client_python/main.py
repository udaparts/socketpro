from spa.clientside import *
from shared.sharedstruct import *
from client.client_python.webasynchandler import CWebAsyncHandler
import collections
from datetime import datetime
from concurrent.futures import TimeoutError

# CA file is located at the directory ../socketpro/bin
if CUQueue.DEFAULT_OS != tagOperationSystem.osWin:
    CClientSocket.SSL.SetVerifyLocation('ca.cert.pem')

with CMasterPool(CWebAsyncHandler, '', False) as sp:
    cc = CConnectionContext(input('Remote middle tier host: \n'), 20911, 'PythonUser', 'TooMuchSecret', tagEncryptionMethod.TLSv1)
    def ssl_server_authentication(pool, cs):
        cert = cs.UCert
        errCode, errMsg = cert.Verify()
        return errCode == 0
    sp.DoSslServerAuthentication = ssl_server_authentication
    # sp.QueueName = 'mpqueue'
    if not sp.StartSocketPool(cc, 1):
        input('No connection to a remote middle tier server, and press ENTER key to kill the demo ......')
        exit(-1)
    cache = sp.Cache
    handler = sp.Seek()
    # for example: payment_id between 1 and 49
    filter = input('Sakila.payment filter: ')
    vData = []
    vData.append(1)  # Google company id
    vData.append('Ted Cruz')
    vData.append(datetime.now())
    vData.append(1)  # Google company id
    vData.append('Donald Trump')
    vData.append(datetime.now())
    vData.append(2)  # Microsoft company id
    vData.append('Hillary Clinton')
    vData.append(datetime.now())

    with CScopeUQueue() as sb:
        try:
            fms = handler.sendRequest(idGetMasterSlaveConnectedSessions, sb)
            sb.SaveString(filter)
            fmma = handler.sendRequest(idQueryMaxMinAvgs, sb)
            sb.UQueue.SetSize(0)
            sb.SaveUInt(len(vData))
            for d in vData:
                sb.SaveObject(d)
            fue = handler.sendRequest(idUploadEmployees, sb)
            sb.UQueue.SetSize(0)
            res = fms.result()
            print('master connections: %d, slave connections: %d' % (res.LoadUInt(), res.LoadUInt())),
            res = fmma.result()
            ec = res.LoadInt()
            em = res.LoadString()
            mma = res.LoadByClass(CMaxMinAvg)
            print('QueryPaymentMaxMinAvgs')
            if ec:
                print('\terror code: %d, message: %s' % (ec, em))
            else:
                print('\tmax : %f, min: %f, avg: %f' % (mma.Max, mma.Min, mma.Avg))
            try:
                res = fue.result(5)
                ec = res.LoadInt()
                em = res.LoadString()
                print('UploadEmployees')
                if ec:
                    print('\tError code: %d, message = %s' % (ec, em))
                else:
                    vId = res.LoadByClass(CLongArray)
                    for d in vId:
                        print('\tLast id: ' + str(d))
            except TimeoutError as ex:
                print('\tThe request UploadEmployees not completed in 5 seconds')
            input('Press ENTER key to test server parallel processing ......\n')
            sb.UQueue.SetSize(0)
            sb.SaveString(filter)
            s_mma = CMaxMinAvg()
            start = datetime.now()
            fQ = collections.deque()
            cycles = 10000
            while cycles > 0:
                fQ.append(handler.sendRequest(idQueryMaxMinAvgs, sb))
                cycles -= 1
            count = len(fQ)
            print('QueryPaymentMaxMinAvgs')
            while len(fQ):
                res = fQ.popleft().result()
                ec = res.LoadInt()
                em = res.LoadString()
                mma = res.LoadByClass(CMaxMinAvg)
                if ec:
                    print('\terror code: %d, message: %s' % (ec, em))
                else:
                    s_mma.Avg += mma.Avg
                    s_mma.Max += mma.Max
                    s_mma.Min += mma.Min
            print('\tTime required: %f seconds for %d requests' % ((datetime.now() - start).total_seconds(), count))
            print('\tsum_max: %f, sum_min: %f, sum_avg: %f' % (s_mma.Max, s_mma.Min, s_mma.Avg))
            input('Press ENTER key to test server parallel processing and sequence returning ......\n')
            cycles = 0
            while cycles < 16000:
                cycles += 1
                sb.UQueue.SetSize(0)
                sb.SaveLong(cycles)
                fQ.append(handler.sendRequest(idGetRentalDateTimes, sb))
            prev_rental_id = 0
            print('GetRentalDateTimes')
            while len(fQ):
                res = fQ.popleft().result()
                dates = res.LoadByClass(CRentalDateTimes)
                ec = res.LoadInt()
                em = res.LoadString()
                if ec:
                    print('\terror code: %d, error message: %s' % (ec, em))
                elif dates.Rental.year <= 1900 and dates.Return.year <= 1900 and dates.LastUpdate.year <= 1900:
                    print('\trental_id: %d not available' % (dates.rental_id))
                elif prev_rental_id == 0 or dates.rental_id == prev_rental_id + 1:
                    pass # print('\trental_id=%d and dates (%s, %s, %s)' % (dates.rental_id, str(dates.Rental), str(dates.Return), str(dates.LastUpdate)))
                else:
                    print('\t****** returned out of order ******')
                prev_rental_id = dates.rental_id

        except (CServerError, CSocketError) as ex:
            print(ex)
        except Exception as ex:
            # invalid parameter, bad de-serialization, and so on
            print('Unexpected error: ' + str(ex))
    input('Press ENTER key to shutdown the demo application ......\n')
