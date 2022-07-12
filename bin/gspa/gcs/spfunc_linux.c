#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint64_t USocket_Client_Handle;
typedef struct {wchar_t* UserId; char* IpAddress; unsigned short Port; unsigned int ServiceId; bool SelfMessage;} CMessageSender;
typedef struct {char* Issuer;char* Subject;char* NotBefore;char* NotAfter;bool Validity;char* SigAlg;char* CertPem;char* SessionInfo;unsigned int PKSize;unsigned char* PublicKey;unsigned int AlgSize;unsigned char* Algorithm;unsigned int SNSize;unsigned char* SerialNumber;} CertInfo;

void spc_go(uint32_t pid, int spe, USocket_Client_Handle handle);
void socketclosed_go(USocket_Client_Handle handle, int ec);
void socketconnected_go(USocket_Client_Handle handle, int ec);
void handshakecompleted_go(USocket_Client_Handle handle, int ec);
void requestprocessed_go(USocket_Client_Handle handler, unsigned short requestId, unsigned int len);
void baserequestprocessed_go(USocket_Client_Handle handler, unsigned short requestId);
void serverexception_go(USocket_Client_Handle handler, unsigned short requestId, const wchar_t *errMessage, const char* errWhere, int errCode);
void allrequestsprocessed_go(USocket_Client_Handle handler, unsigned short lastRequestId);
void postprocessing_go(USocket_Client_Handle handler, unsigned int hint, uint64_t data);
void enter_go(USocket_Client_Handle handler, CMessageSender *sender, const unsigned int *pGroup, unsigned int count);
void exit_go(USocket_Client_Handle handler, CMessageSender *sender, const unsigned int *pGroup, unsigned int count);
void speak_go(USocket_Client_Handle handler, CMessageSender *sender, const unsigned int *pGroup, unsigned int count, const unsigned char *message, unsigned int size);
void usermessage_go(USocket_Client_Handle handler, CMessageSender *sender, const unsigned char *message, unsigned int size);
void usermessageex_go(USocket_Client_Handle handler, CMessageSender *sender, const unsigned char *message, unsigned int size);
void speakex_go(USocket_Client_Handle handler, CMessageSender *sender, const unsigned int *pGroup, unsigned int count, const unsigned char *message, unsigned int size);
bool cvc_go(bool verified, int depth, int ec, const char *em, CertInfo *ci);

void spc_cgo(uint32_t pid, int spe, USocket_Client_Handle handle) {
	spc_go(pid, spe, handle);
}

void socketclosed_cgo(USocket_Client_Handle handle, int ec) {
	socketclosed_go(handle, ec);
}

void socketconnected_cgo(USocket_Client_Handle handle, int ec) {
	socketconnected_go(handle, ec);
}

void handshakecompleted_cgo(USocket_Client_Handle handle, int ec) {
	handshakecompleted_go(handle, ec);
}

void requestprocessed_cgo(USocket_Client_Handle handler, unsigned short requestId, unsigned int len) {
	requestprocessed_go(handler, requestId, len);
}

void baserequestprocessed_cgo(USocket_Client_Handle handler, unsigned short requestId) {
	baserequestprocessed_go(handler, requestId);
}

void serverexception_cgo(USocket_Client_Handle handler, unsigned short requestId, const wchar_t *errMessage, const char* errWhere, int errCode) {
	serverexception_go(handler, requestId, errMessage, errWhere, errCode);
}

void allrequestsprocessed_cgo(USocket_Client_Handle handler, unsigned short lastRequestId) {
	allrequestsprocessed_go(handler, lastRequestId);
}

void postprocessing_cgo(USocket_Client_Handle handler, unsigned int hint, uint64_t data) {
	postprocessing_go(handler, hint, data);
}

void enter_cgo(USocket_Client_Handle handler, CMessageSender *sender, const unsigned int *pGroup, unsigned int count) {
	enter_go(handler, sender, pGroup, count);
}

void exit_cgo(USocket_Client_Handle handler, CMessageSender *sender, const unsigned int *pGroup, unsigned int count) {
	exit_go(handler, sender, pGroup, count);
}

void speak_cgo(USocket_Client_Handle handler, CMessageSender *sender, const unsigned int *pGroup, unsigned int count, const unsigned char *message, unsigned int size) {
	speak_go(handler, sender, pGroup, count, message, size);
}

void usermessage_cgo(USocket_Client_Handle handler, CMessageSender *sender, const unsigned char *message, unsigned int size) {
	usermessage_go(handler, sender, message, size);
}

void usermessageex_cgo(USocket_Client_Handle handler, CMessageSender *sender, const unsigned char *message, unsigned int size) {
	usermessageex_go(handler, sender, message, size);
}

void speakex_cgo(USocket_Client_Handle handler, CMessageSender *sender, const unsigned int *pGroup, unsigned int count, const unsigned char *message, unsigned int size) {
	speakex_go(handler, sender, pGroup, count, message, size);
}

bool cvc_cgo(bool verified, int depth, int ec, const char *em, CertInfo *ci) {
	return cvc_go(verified, depth, ec, em, ci);
}

