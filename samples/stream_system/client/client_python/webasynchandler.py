from spa.clientside import CCachedBaseHandler
from shared.sharedstruct import *

class CWebAsyncHandler(CCachedBaseHandler):

    def __init__(self):
        super(CWebAsyncHandler, self).__init__(sidStreamSystem)
