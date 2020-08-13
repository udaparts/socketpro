package SPA.UDB;

public class CDBColumnInfoArray extends java.util.ArrayList<CDBColumnInfo> implements SPA.IUSerializer {

    @Override
    public SPA.CUQueue LoadFrom(SPA.CUQueue UQueue) {
        clear();
        int size = UQueue.LoadInt();
        while (size > 0) {
            CDBColumnInfo obj = new CDBColumnInfo();
            obj.LoadFrom(UQueue);
            add(obj);
            --size;
        }
        return UQueue;
    }

    @Override
    public SPA.CUQueue SaveTo(SPA.CUQueue UQueue) {
        int size = size();
        UQueue.Save(size);
        for (int n = 0; n < size; ++n) {
            get(n).SaveTo(UQueue);
        }
        return UQueue;
    }
}
