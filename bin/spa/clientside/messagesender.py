
from ctypes import Structure, c_wchar_p, c_char_p, c_ushort, c_uint, c_bool, c_void_p

"""
struct CMessageSender {
            const wchar_t* UserId;
            const char* IpAddress;
            unsigned short Port;
            unsigned int ServiceId;
            bool SelfMessage;
        };
"""

class CMessageSender(Structure):
    _fields_ = [("UserId", c_wchar_p),
                ("IpAddress", c_char_p),
                ("Port", c_ushort),
                ("ServiceId", c_uint),
                ("SelfMessage", c_bool)]

USocket_Client_Handle = c_void_p

class CSender(object):
    def __init__(self, sender):
        self.UserId = sender.contents.UserId
        self.IpAddress = sender.contents.IpAddress.decode('utf-8')
        self.Port = sender.contents.Port
        self.ServiceId = sender.contents.ServiceId
        self.SelfMessage = sender.contents.SelfMessage