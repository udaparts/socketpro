#ifndef _UDAPARTS_ASYNC_DB2_HANDLER_H_
#define _UDAPARTS_ASYNC_DB2_HANDLER_H_

#include "odbcbase.h"
#include "db2/udb2.h"

namespace SPA {
    namespace ClientSide {
        class CDb2 : public CBaseOdbc {
            CDb2(const CDb2& db) = delete;
            CDb2& operator=(const CDb2& db) = delete;

        public:
            CDb2(CClientSocket* cs) : CBaseOdbc(Db2::sidDB2, cs) {
            }
        };
        typedef COdbcBase CDb2Base;
        typedef CSocketPool<CDb2> CDb2Pool;
    } //namespace ClientSide
} //namespace SPA

#endif
