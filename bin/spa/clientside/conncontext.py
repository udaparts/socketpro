from spa import tagEncryptionMethod

class CConnectionContext(object):

    #constructor arguments default to arbitrary inputs as hints
    def __init__(self, host='localhost', port=20901, userId=u'SocketPro', password=u'PassOne', em=tagEncryptionMethod.NoEncryption, zip=False, v6=False):
        self.Host = host
        self.Port = port
        self.UserId = userId
        self._Password_ = password
        self.EncrytionMethod = em
        self.Zip = zip
        self.V6 = v6
        self.AnyData = None
