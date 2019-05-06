
#pragma once
#include <deque>
#include "../include/membuffer.h"
#ifdef WIN32_64
#include <unordered_map>
#else
#include <map>
#endif
#include "../core_shared/pinc/mqfile.h"

class CResIndexImpl {
public:
    CResIndexImpl(unsigned short reqId, void *session);
    CResIndexImpl(unsigned short reqId, void *session, const MQ_FILE::QAttr &qa);
    ~CResIndexImpl();

public:

    inline unsigned short GetReqId() const {
        return m_uReqId;
    }

    inline void SetHead(bool isHead) {
        m_bHead = isHead;
    }

    bool IsAllCollected(bool *partial = nullptr) const;

    inline bool IsSent() const {
        return m_pQ == nullptr;
    }
    unsigned int SendReturnData(unsigned short usReqId, const unsigned char *pBuffer, unsigned int size);
    SPA::UINT64 SendAll();
    SPA::UINT64 SendPartial();

private:
    CResIndexImpl(const CResIndexImpl &rii);
    CResIndexImpl& operator=(const CResIndexImpl &rii);

private:
    unsigned short m_uReqId;
    void *m_pSession;
    MQ_FILE::QAttr m_qa;
    bool m_bHead;
    std::deque<SPA::CUQueue *> m_deq;
    SPA::CUQueue *m_pQ;
};

#ifdef WIN32_64
typedef std::unordered_map<SPA::UINT64, std::shared_ptr<CResIndexImpl>> CMapIndex;
#else
typedef std::map<SPA::UINT64, std::shared_ptr<CResIndexImpl>> CMapIndex;
#endif

bool IsTooMany(const CMapIndex &mi);
