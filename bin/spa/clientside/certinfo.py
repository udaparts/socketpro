
from ctypes import Structure, c_char_p, c_bool, c_uint, POINTER, c_ubyte

"""
struct CertInfo {
    const char* Issuer;
    const char* Subject;
    const char* NotBefore;
    const char* NotAfter;
    bool Validity;
    const char* SigAlg;
    const char* CertPem;
    const char* SessionInfo;
    unsigned int PKSize;
    const unsigned char* PublicKey;
    unsigned int AlgSize;
    const unsigned char* Algorithm;
    unsigned int SNSize;
    const unsigned char* SerialNumber;
};
"""

class CertInfo(Structure):
    _fields_ = [("Issuer", c_char_p),
                ("Subject", c_char_p),
                ("NotBefore", c_char_p),
                ("NotAfter", c_char_p),
                ("Validity", c_bool),
                ("SigAlg", c_char_p),
                ("CertPem", c_char_p),
                ("SessionInfo", c_char_p),
                ("PKSize", c_uint),
                ("PublicKey", POINTER(c_ubyte)),
                ("AlgSize", c_uint),
                ("Algorithm", POINTER(c_ubyte)),
                ("SNSize", c_uint),
                ("SerialNumber", POINTER(c_ubyte))]

PCertInfo=POINTER(CertInfo)