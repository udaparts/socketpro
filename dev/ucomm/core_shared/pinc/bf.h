// BF.h: interface for the CBF class.
//
//////////////////////////////////////////////////////////////////////

//this implementation is slower in comparsion to the other one

#if !defined(__SOCKET_PRO_BLOW_FISH__)
#define __SOCKET_PRO_BLOW_FISH__

namespace SPA{

struct BLOWFISH_CTX
{
  unsigned int P[16 + 2];
  unsigned int S[4][256];
};

class CBF  
{
public:
	CBF(unsigned char *strKey, unsigned char nSize);

public:
	// Encrypt/Decrypt Buffer in Place
	void Encrypt(unsigned char* buf, unsigned int nSize);
	void Decrypt(unsigned char* buf, unsigned int nSize);

private:
	void Blowfish_Init(unsigned char *key, int keyLen);
	void Blowfish_Encrypt(unsigned int *xl, unsigned int *xr);
	void Blowfish_Decrypt(unsigned int *xl, unsigned int *xr);
	inline unsigned int F(unsigned int x);

private:
	BLOWFISH_CTX	m_ctx;
	static unsigned int ORIG_P[16 + 2];
	static unsigned int ORIG_S[4][256];
};

}; //namespace SPA

#endif // !defined(__SOCKET_PRO_BLOW_FISH__)
