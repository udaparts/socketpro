#pragma once

#include "../../shared/tablecache.h"

class CMasterPool : public CAsyncSQLPool {
public:
    CMasterPool();

protected:
    virtual void OnSocketPoolEvent(tagSocketPoolEvent spe, const PHandler &handler);

private:
    CTableCache m_cache;
};

