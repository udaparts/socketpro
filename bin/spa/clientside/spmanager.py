import json
import threading
from spa import *
from spa.clientside.ccoreloader import CCoreLoader as ccl
from spa.clientside import *

class SpManager(object):
    _cs_ = threading.Lock()
    _sp_config = None
    _mid_tier = False
    _pool_keys = {}

    @classmethod
    def _RepeatedHost(cls, hosts, cc):
        count = 0
        for key, value in hosts.items():
            if cc['Host'] == value['Host'] and cc['Port'] == value['Port']:
                count += 1
        return count > 1

    @classmethod
    def _Exist(cls, hosts, h):
        for key, value in hosts.items():
            if h == key:
                return True
        return False

    @classmethod
    def _CheckHostsError(cls, sc):
        hosts = sc['Hosts']
        if len(hosts) == 0:
            raise Exception('Hosts cannot be an empty dictionary')
        for key, value in hosts.items():
            if not key:
                raise Exception('Host key cannot be empty')
            value['Host'] = value['Host'].strip().lower()
            if not value['Host']:
                raise Exception('Host string cannot be empty')
            port = int(value['Port'])
            if port <= 0:
                raise Exception('Port number must be a positive integer')
            value['Port'] = port
            if 'EncrytionMethod' in value:
                em = int(value['EncrytionMethod'])
                if em < 0 or em > 1:
                    raise Exception('EncrytionMethod must be either 0 (not secure) or 1 (TLSv)')
                value['EncrytionMethod'] = em
            else:
                value['EncrytionMethod'] = 0
            if not 'UserId' in value:
                value['UserId'] = ''
            else:
                value['UserId'] = str(value['UserId']).strip()
            if not 'Password' in value:
                value['Password'] = ''
            if not 'Zip' in value:
                value['Zip'] = False
            else:
                value['Zip'] = not not value['Zip']
            if not 'V6' in value:
                value['V6'] = False
            else:
                value['V6'] = not not value['V6']
            if not 'AnyData' in value:
                value['AnyData'] = None
        for key, value in hosts.items():
            if SpManager._RepeatedHost(hosts, value):
                raise Exception('Connection context for host ' + key + ' duplicated')

    @classmethod
    def _CheckPoolsError(cls, hosts, pools, master=None, masterSvsId=0, defaultDb=None):
        svsId = 0
        if len(pools) == 0:
            raise Exception('Pools cannot be an empty dictionary')
        for key, value in pools.items():
            value['Pool'] = None
            if not key:
                raise Exception('Pool key cannot be empty')
            if masterSvsId:
                svsId = masterSvsId
                value['SvsId'] = svsId
            else:
                svsId = value['SvsId']
            if svsId == BaseServiceID.sidQueue:
                pass
            elif svsId == BaseServiceID.sidFile:
                pass
            elif svsId == BaseServiceID.sidODBC:
                pass
            elif svsId == CSqlite.sidSqlite:
                pass
            elif svsId == CMysql.sidMysql:
                pass
            elif svsId > BaseServiceID.sidReserved:
                pass
            else:
                raise Exception("User defined service id must be larger than " + BaseServiceID.sidReserved);
            if not 'RecvTimeout' in value or value['RecvTimeout'] <= 0:
                value['RecvTimeout'] = CClientSocket.DEFAULT_RECV_TIMEOUT
            if not 'ConnTimeout' in value or value['ConnTimeout'] <= 0:
                value['ConnTimeout'] = CClientSocket.DEFAULT_CONN_TIMEOUT
            if 'Master' in value:
                del value['Master']
            if not 'AutoConn' in value:
                value['AutoConn'] = True
            else:
                value['AutoConn'] = not not value['AutoConn']
            if not 'AutoMerge' in value:
                value['AutoMerge'] = False
            else:
                value['AutoMerge'] = not not value['AutoMerge']
            hs = value['Hosts']
            if len(hs) == 0:
                raise Exception('Hosts array for pool ' + key + ' cannot be empty')
            for h in hs:
                if not SpManager._Exist(hosts, h):
                    raise Exception('Host ' + h + ' cannot be found from Hosts')
            if not 'Threads' in value or not value['Threads']:
                value['Threads'] = 1
            else:
                value['Threads'] = int(value['Threads'])
            value['PoolType'] = 0
            if master:
                value['Master'] = master
                value['PoolType'] = 1
                if not 'DefaultDb' in value or not value['DefaultDb']:
                    value['DefaultDb'] = defaultDb
                if key in SpManager._pool_keys:
                    raise Exception('Slave pool key ' + key + ' duplicated')
                SpManager._pool_keys[key] = value
            elif 'DefaultDb' in value and value['DefaultDb'] and not master:
                value['PoolType'] = 2
                if svsId == BaseServiceID.sidQueue:
                    raise Exception('SocketPro server queue service has no support to either master or slave pool')
                elif svsId == BaseServiceID.sidFile:
                    raise Exception('SocketPro remote file service has no support to either master or slave pool')
                if 'Slaves' in value:
                    slaves = value['Slaves']
                    SpManager._CheckPoolsError(hosts, slaves, key, svsId, value['DefaultDb'])
            elif 'Slaves' in value:
                raise Exception('A regular pool cannot have slaves')

    @classmethod
    def _CheckErrors(cls, sc):
        SpManager._CheckHostsError(sc)
        pools = sc['Pools']
        for key, value in pools.items():
            SpManager._pool_keys[key] = value
        SpManager._CheckPoolsError(sc['Hosts'], pools)

    @classproperty
    def Config(clas):
        """
        :return: A dictionary containing SocketPro pools configuration
        """

        with SpManager._cs_:
            if SpManager._sp_config:
                SpManager._sp_config['WorkingDir'] = ccl.GetClientWorkDirectory().decode('latin-1')
                return SpManager._sp_config
            return None

    @classproperty
    def MidTier(clas):
        with SpManager._cs_:
            return SpManager._mid_tier

    @classproperty
    def Version(cls):
        """
        :return: Client core library version string
        """
        return ccl.GetUClientSocketVersion().decode('latin-1')

    @classproperty
    def Pools(cls):
        """
        :return: The number of running socket pools
        """
        return ccl.GetNumberOfSocketPools()

    @staticmethod
    def SetConfig(midTier=False, jsonConfig=None):
        """
        Set socket pools configuration from a JSON text file
        :param midTier: True if calling from a middle tier; Otherwise, false
        :param jsonConfig: >A file path to a JSON configuration text file, which defaults to sp_config.json at current directory
        :return: A dictionary containing SocketPro pools configuration
        """
        with SpManager._cs_:
            if SpManager._sp_config:
                return SpManager._sp_config
            if not jsonConfig:
                jsonConfig = 'sp_config.json'
            with open(jsonConfig, 'r') as jf:
                s = jf.read()
                start = s.find('{')
                s = s[start:]
                sc = json.loads(s)
                if 'CertStore' in sc and sc['CertStore']:
                    ccl.SetVerifyLocation(sc['CertStore'].encode('latin-1'))
                if 'WorkingDir' in sc and sc['WorkingDir']:
                    ccl.SetClientWorkDirectory(sc['WorkingDir'].encode('latin-1'))
                if 'QueuePassword' in sc and sc['QueuePassword']:
                    ccl.SetMessageQueuePassword(sc['QueuePassword'].encode('latin-1'))
                    sc['QueuePassword'] = 1
                else:
                    sc['QueuePassword'] = 0
                if 'KeysAllowed' in sc:
                    ka = []
                    for s in sc['KeysAllowed']:
                        ka.append(s.lower())
                    sc['KeysAllowed'] = ka
            SpManager._CheckErrors(sc)
            SpManager._sp_config = sc
            SpManager._sp_config['WorkingDir'] = ccl.GetClientWorkDirectory().decode('latin-1')
            return SpManager._sp_config

    @classmethod
    def _ToCC(cls, conn):
        cc = CConnectionContext(conn['Host'], conn['Port'], conn['UserId'], conn['Password'], conn['EncrytionMethod'], conn['Zip'], conn['V6'])
        if 'AnyData' in conn:
            cc.AnyData = conn['AnyData']
        return cc

    @staticmethod
    def SeekHandler(poolKey):
        pool = SpManager.GetPool(poolKey)
        return pool.Seek()

    @staticmethod
    def SeekHandlerByQueue(poolKey):
        pool = SpManager.GetPool(poolKey)
        return pool.SeekByQueue()

    @staticmethod
    def LockHandler(poolKey, timeout=0xffffffff):
        pool = SpManager.GetPool(poolKey)
        return pool.Lock(timeout)

    @staticmethod
    def GetPool(poolKey):
        pc = None
        with SpManager._cs_:
            if not SpManager._sp_config:
                SpManager.SetConfig()
            if poolKey not in SpManager._pool_keys:
                raise Exception('Pool key ' + poolKey + ' cannot be found from configuration')
            pc = SpManager._pool_keys[poolKey]
            if pc['Pool']:
                return pc['Pool']
            svsId = pc['SvsId']
            pt = pc['PoolType']
            pool = None
            if svsId == BaseServiceID.sidQueue:
                pool = CSocketPool(CAsyncQueue, pc['AutoConn'], pc['RecvTimeout'], pc['ConnTimeout'])
            elif svsId == BaseServiceID.sidFile:
                pool = CSocketPool(CStreamingFile, pc['AutoConn'], pc['RecvTimeout'], pc['ConnTimeout'])
            elif svsId == BaseServiceID.sidODBC:
                if pt == 0:
                    pool = CSocketPool(COdbc, pc['AutoConn'], pc['RecvTimeout'], pc['ConnTimeout'])
                elif pt == 2:
                    pool = CSqlMasterPool(COdbc, pc['DefaultDb'], SpManager._mid_tier, pc['RecvTimeout'], pc['AutoConn'], pc['ConnTimeout'])
                else:
                    pool = CSqlMasterPool.CSlavePool(COdbc, pc['DefaultDb'], pc['RecvTimeout'], pc['AutoConn'], pc['ConnTimeout'])
            elif svsId == CSqlite.sidSqlite:
                if pt == 0:
                    pool = CSocketPool(CSqlite, pc['AutoConn'], pc['RecvTimeout'], pc['ConnTimeout'])
                elif pt == 2:
                    pool = CSqlMasterPool(CSqlite, pc['DefaultDb'], SpManager._mid_tier, pc['RecvTimeout'], pc['AutoConn'], pc['ConnTimeout'])
                else:
                    pool = CSqlMasterPool.CSlavePool(CSqlite, pc['DefaultDb'], pc['RecvTimeout'], pc['AutoConn'], pc['ConnTimeout'])
            elif svsId == CMysql.sidMysql:
                if pt == 0:
                    pool = CSocketPool(CMysql, pc['AutoConn'], pc['RecvTimeout'], pc['ConnTimeout'])
                elif pt == 2:
                    pool = CSqlMasterPool(CMysql, pc['DefaultDb'], SpManager._mid_tier, pc['RecvTimeout'], pc['AutoConn'], pc['ConnTimeout'])
                else:
                    pool = CSqlMasterPool.CSlavePool(CMysql, pc['DefaultDb'], pc['RecvTimeout'], pc['AutoConn'], pc['ConnTimeout'])
            elif svsId > BaseServiceID.sidReserved:
                if pt == 0:
                    pool = CSocketPool(CCachedBaseHandler, pc['AutoConn'], pc['RecvTimeout'], pc['ConnTimeout'], svsId)
                elif pt == 2:
                    pool = CMasterPool(CCachedBaseHandler, pc['DefaultDb'], SpManager._mid_tier, pc['RecvTimeout'], pc['AutoConn'], pc['ConnTimeout'], svsId)
                else:
                    pool = CMasterPool.CSlavePool(CCachedBaseHandler, pc['DefaultDb'], pc['RecvTimeout'], pc['AutoConn'], pc['ConnTimeout'], svsId)
            else:
                raise Exception('Unexpected error')
            pool.QueueAutoMerge = pc['AutoMerge'];
            if 'Queue' in pc:
                pool.QueueName = pc['Queue'];
            pc['Pool'] =  pool

            def OnVerify(sp, cs):
                cert = cs.UCert
                errCode, errMsg = cert.Verify()
                if errCode == 0:
                    return True
                if 'KeysAllowed' in SpManager._sp_config:
                    str = cert.PublicKey
                    my_s = '';
                    for s in str:
                        my_s +=  format(s, '02x')
                    return (my_s in SpManager._sp_config['KeysAllowed'])
                return False
            pool.DoSslServerAuthentication = OnVerify
            sockets_per_thread = len(pc['Hosts'])
            threads = pc['Threads']
            # create a two-dimension matrix that contains connection contexts
            mcc = [[0 for i in range(sockets_per_thread)] for i in range(threads)]
            while threads > 0:
                threads -= 1
                j = 0
                for key in pc['Hosts']:
                    mcc[threads][j] = SpManager._ToCC(SpManager._sp_config['Hosts'][key])
                    j += 1
            if not pool.StartSocketPoolEx(mcc):
                raise Exception('There is no connection establised for pool ' + poolKey)
            return pool

"""
sc = SpManager.SetConfig(True, 'C:\\cyetest\\socketpro\\src\\njadapter\\sp_config.json')
pool = SpManager.GetPool('my_hello_world')
pool = 0
"""


