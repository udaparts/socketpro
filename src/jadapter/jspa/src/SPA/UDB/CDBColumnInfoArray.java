package SPA.UDB;

public class CDBColumnInfoArray extends java.util.ArrayList<CDBColumnInfo> implements SPA.IUSerializer {

    @Override
    public void LoadFrom(SPA.CUQueue UQueue) {
        clear();
        int size = UQueue.LoadInt();
        while (size > 0) {
            CDBColumnInfo obj = new CDBColumnInfo();
            obj.LoadFrom(UQueue);
            add(obj);
            --size;
        }
    }

    @Override
    public void SaveTo(SPA.CUQueue UQueue) {
        int size = size();
        UQueue.Save(size);
        for (int n = 0; n < size; ++n) {
            get(n).SaveTo(UQueue);
        }
    }
}
