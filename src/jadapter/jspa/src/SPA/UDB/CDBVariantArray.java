
package SPA.UDB;
import java.util.ArrayList;

public class CDBVariantArray extends ArrayList<Object> implements SPA.IUSerializer {

    @Override
    public void LoadFrom(SPA.CUQueue UQueue) {
        clear();
        int size = UQueue.LoadInt();
        while (size > 0) {
            Object obj = UQueue.LoadObject();
            add(obj);
            --size;
        }
    }

    @Override
    public void SaveTo(SPA.CUQueue UQueue) {
        int size = size();
        UQueue.Save(size);
        for (int n = 0; n < size; ++n) {
            UQueue.Save(get(n));
        }
    }
}
