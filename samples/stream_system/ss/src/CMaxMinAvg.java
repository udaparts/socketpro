
import SPA.*;

public class CMaxMinAvg implements IUSerializer {

    public double Max = 0;
    public double Min = 0;
    public double Avg = 0;

    @Override
    public CUQueue LoadFrom(CUQueue q) {
        Max = q.LoadDouble();
        Min = q.LoadDouble();
        Avg = q.LoadDouble();
        return q;
    }

    @Override
    public CUQueue SaveTo(CUQueue q) {
        q.Save(Max).Save(Min).Save(Avg);
        return q;
    }
}
