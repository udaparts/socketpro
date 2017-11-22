
from spa.serverside import RequestAttr, ServicetAttr


@ServicetAttr(234)
class CMyOne(object):
    @RequestAttr(2)
    def aFunction(self):
        print ("inside aFunction()")

    @RequestAttr(21, True)
    def aFunc(self):
        print("inside aFunc()")


obj0=CMyOne()
obj1=CMyOne()

# print("Finished decorating aFunction()")

# aFunction()