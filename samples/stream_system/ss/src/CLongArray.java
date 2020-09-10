
import SPA.*;
import java.util.ArrayList;

public class CLongArray extends ArrayList<Long> implements IUSerializer {

    @Override
    public CUQueue LoadFrom(CUQueue q) {

        clear();
        int size = q.LoadInt();
        while (size > 0) {
            long n = q.LoadLong();
            add(n);
            --size;
        }
        return q;
    }

    @Override
    public CUQueue SaveTo(CUQueue q) {
        q.Save(size());
        for (long n : this) {
            q.Save(n);
        }
        return q;
    }
}
