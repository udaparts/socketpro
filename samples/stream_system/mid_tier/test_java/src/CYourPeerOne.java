
import SPA.ServerSide.CCacheBasePeer;
import SPA.ClientSide.CAsyncDBHandler;
import SPA.ClientSide.CSqlite;

public class CYourPeerOne extends CCacheBasePeer {

    @Override
    protected CachedTableResult GetCachedTables(String defaultDb, int flags, boolean rowset, long index) {
        CachedTableResult res = this.new CachedTableResult();
        //CSqlMasterPool<CSqlite> Master = CYourServer.Master;
        do {
            if (!rowset) {
                res.errMsg = "Client side doesn't ask for rowsets";
                break;
            }
            CConfig config = CConfig.getConfig();
            if (config.m_vFrontCachedTable.isEmpty()) {
                break;
            }
            if ((flags & CAsyncDBHandler.ENABLE_TABLE_UPDATE_MESSAGES) == CAsyncDBHandler.ENABLE_TABLE_UPDATE_MESSAGES) {
                if (!getPush().Subscribe(CAsyncDBHandler.CACHE_UPDATE_CHAT_GROUP_ID, CAsyncDBHandler.STREAMING_SQL_CHAT_GROUP_ID)) {
                    res.errMsg = "Failed in subscribing for table events"; //warning message
                }
            }
            String sql = "";
            for (String s : config.m_vFrontCachedTable) {
                if (sql.length() != 0) {
                    sql += ";";
                }
                sql += "SELECT * FROM " + s;
            }
            CSqlite handler = CYourServer.Master.Lock(); //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
            if (handler == null) {
                res.res = -1;
                res.errMsg = "No connection to a master database";
                break;
            }
            res.ms = handler.getDBManagementSystem();

        } while (false);

        return res;
    }

}
