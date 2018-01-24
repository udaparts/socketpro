from spa import CUQueue, tagOperationSystem
from spa.clientside import CConnectionContext


class CConfig(object):
    _config_ = None

    def __init__(self):
        # master
        self.m_master_default_db = ''
        self.m_nMasterSessions = 2
        self.m_ccMaster = None
        self.m_master_queue_name = ''

        # slave
        self.m_slave_default_db = ''
        self.m_nSlaveSessions = 0
        self.m_vccSlave = []
        self.m_slave_queue_name = ''

        # middle tier server
        self.m_main_threads = 1
        self.m_nPort = 20911
        self.m_bNoIpV6 = False

        # secure communication between front and middle tier
        if CUQueue.DEFAULT_OS == tagOperationSystem.osWin:
            self.m_store_or_pfx = ''
        else:
            self.m_cert = ''  # in pem format
            self.m_key = '' # in pem format
        self.m_password_or_subject = ''

        # a list of cached tables for front applications
        self.m_vFrontCachedTable = []

    @staticmethod
    def getConfig():
        if CConfig._config_:
            return CConfig._config_
        config = CConfig()

        # load the following settings from a configuration file
        config.m_main_threads = 4

        # master
        config.m_master_default_db = 'sakila.db'
        config.m_nMasterSessions = 1  # one session enough
        config.m_ccMaster = CConnectionContext('localhost', 20901, 'root', 'Smash123')

        config.m_slave_default_db = 'sakila.db'
        cc = CConnectionContext('35.202.209.70', 20901, 'root', 'Smash123')
        config.m_vccSlave.append(cc)
        # treat master as last salve
        config.m_vccSlave.append(config.m_ccMaster)
        config.m_nSlaveSessions = 4
        config.m_slave_queue_name = 'db_sakila'

        # middle tier
        # test certificate and private key files are located at the directory ../socketpro/bin
        if CUQueue.DEFAULT_OS == tagOperationSystem.osWin:
            config.m_store_or_pfx = 'intermediate.pfx'
        else:
            config.m_cert = 'intermediate.cert.pem'
            config.m_key = 'intermediate.key.pem'
        config.m_password_or_subject = 'mypassword'

        # a list of cached tables for front applications
        config.m_vFrontCachedTable.append("actor")
        config.m_vFrontCachedTable.append("language")
        config.m_vFrontCachedTable.append("country")
        config.m_vFrontCachedTable.append("film_actor")

        CConfig._config_ = config
        return config
