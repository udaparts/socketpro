
import SPA.CUQueue;
import SPA.IUSerializer;
import java.util.ArrayList;

public class CLongArray extends ArrayList<Long> implements IUSerializer {

    @Override
    public void LoadFrom(CUQueue q) {

        clear();
        int size = q.LoadInt();
        while (size > 0) {
            long n = q.LoadLong();
            add(n);
            --size;
        }
    }

    @Override
    public void SaveTo(CUQueue q) {
        q.Save(size());
        for (long n : this) {
            q.Save(n);
        }
    }
}
