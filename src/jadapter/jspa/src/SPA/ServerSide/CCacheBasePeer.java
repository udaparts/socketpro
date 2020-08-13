package SPA.ServerSide;

import SPA.*;
import SPA.UDB.*;

public abstract class CCacheBasePeer extends CClientPeer {

    public CCacheBasePeer() {
    }

    public class CachedTableResult implements IUSerializer {

        @Override
        public CUQueue LoadFrom(CUQueue UQueue) {
            ms = tagManagementSystem.forValue(UQueue.LoadInt());
            res = UQueue.LoadInt();
            errMsg = UQueue.LoadString();
            return UQueue;
        }

        @Override
        public CUQueue SaveTo(CUQueue UQueue) {
            return UQueue.Save(ms.getValue()).Save(res).Save(errMsg);
        }
        public tagManagementSystem ms = tagManagementSystem.msUnknown;
        public int res = 0;
        public String errMsg = "";
    }

    protected abstract CachedTableResult GetCachedTables(String defaultDb, int flags, long index);

    public boolean SendMeta(CDBColumnInfoArray meta, long index) {
        try (CScopeUQueue q = new CScopeUQueue()) {
            meta.SaveTo(q.getUQueue());
            q.Save(index);
            //A client expects a rowset meta data and call index
            int ret = SendResult(DB_CONSTS.idRowsetHeader, q);
            return (ret != REQUEST_CANCELED && ret != SOCKET_NOT_FOUND);
        }
    }

    protected boolean SendRows(CUQueue q, boolean transferring) {
        boolean batching = (getBytesBatched() >= DB_CONSTS.DEFAULT_RECORD_BATCH_SIZE);
        if (batching) {
            CommitBatching();
        }
        int ret = SendResult(transferring ? DB_CONSTS.idTransferring : DB_CONSTS.idEndRows, q.getIntenalBuffer(), q.GetSize());
        q.SetSize(0);
        if (batching) {
            StartBatching();
        }
        return (ret != REQUEST_CANCELED && ret != SOCKET_NOT_FOUND);
    }

    protected boolean SendBlob(CUQueue qBuffer) {
        CUQueue q = CScopeUQueue.Lock();
        short data_type = qBuffer.LoadShort();
        int bytes = qBuffer.LoadInt();
        /* 10 = sizeof (short) + sizeof (int) + sizeof (int) extra 4 bytes for string null termination*/
        q.Save(bytes + 10).Save(data_type).Save(bytes);
        int ret = SendResult(DB_CONSTS.idStartBLOB, q);
        CScopeUQueue.Unlock(q);
        if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
            return false;
        }
        byte[] b = new byte[DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE];
        while (qBuffer.GetSize() > DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE) {
            ret = qBuffer.PopBytes(b, DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE);
            ret = SendResult(DB_CONSTS.idChunk, b, DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE);
            if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                return false;
            }
        }
        ret = qBuffer.PopBytes(b, DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE);
        ret = SendResult(DB_CONSTS.idEndBLOB, b, ret);
        return (ret != REQUEST_CANCELED && ret != SOCKET_NOT_FOUND);
    }

    public boolean SendRows(CDBVariantArray vData) {
        int len = 0;
        CUQueue q = CScopeUQueue.Lock();
        for (Object vt : vData) {
            if (vt instanceof String) {
                String s = (String) vt;
                if (s.length() > DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                    if (q.GetSize() > 0 && !SendRows(q, true)) {
                        CScopeUQueue.Unlock(q);
                        return false;
                    }
                    q.Save(vt);
                    if (!SendBlob(q)) {
                        CScopeUQueue.Unlock(q);
                        return false;
                    }
                    q.SetSize(0);
                } else {
                    q.Save(vt);
                }
            } else if (vt instanceof byte[]) {
                byte[] s = (byte[]) vt;
                if (s.length > DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE * 2) {
                    if (q.GetSize() > 0 && !SendRows(q, true)) {
                        CScopeUQueue.Unlock(q);
                        return false;
                    }
                    q.Save(vt);
                    if (!SendBlob(q)) {
                        CScopeUQueue.Unlock(q);
                        return false;
                    }
                    q.SetSize(0);
                } else {
                    q.Save(vt);
                }
            } else {
                q.Save(vt);
            }
        }
        len = SendResult(DB_CONSTS.idEndRows, q);
        CScopeUQueue.Unlock(q);
        return (len != REQUEST_CANCELED && len != SOCKET_NOT_FOUND);
    }
}
