package SPA.UDB;

import java.util.ArrayList;

public class CPosArray extends ArrayList<Integer> implements SPA.IUSerializer {

    @Override
    public void LoadFrom(SPA.CUQueue UQueue) {
        clear();
        int size = UQueue.LoadInt();
        while (size > 0) {
            int data = UQueue.LoadInt();
            add(data);
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
