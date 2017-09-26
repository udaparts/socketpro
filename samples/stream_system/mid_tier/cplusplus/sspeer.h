#pragma once

class CSSPeer : public CClientPeer {
public:
    CSSPeer();
    ~CSSPeer();

protected:
    virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len);
    virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len);

private:


private:
    CSSPeer(const CSSPeer &p);
    CSSPeer& operator=(const CSSPeer &p);
};
