
from spa.serverside import RequestAttr, tagThreadApartment, CSocketProService

CSocketProService._sa_ = []

class ServicetAttr(object):
    def __init__(self, ServiceID, ThreadApartment=tagThreadApartment.taNone):
        self.ServiceID = ServiceID
        self.ThreadApartment = ThreadApartment

    def __call__(self, original_func):
        CSocketProService._sa_.append([self.ServiceID, self.ThreadApartment, RequestAttr._ra_])
        RequestAttr._ra_ = []
        def wrapper(*args, **kwargs):
            original_func(*args, **kwargs)
        return wrapper
