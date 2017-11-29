
# from spa.serverside import ServicetAttr

class RequestAttr(object):
    _ra_ = []
    def __init__(self, RequestID, SlowRequest=False):
        self.RequestID = RequestID
        self.SlowRequest = SlowRequest

    def __call__(self, original_func):
        def wrapper(*args, **kwargs):
            original_func(*args, **kwargs)
        RequestAttr._ra_.append([self.RequestID, self.SlowRequest, wrapper])
        return wrapper
