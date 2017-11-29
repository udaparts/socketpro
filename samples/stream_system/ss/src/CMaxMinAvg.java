
import SPA.*;

public class CMaxMinAvg implements IUSerializer {

    public double Max = 0;
    public double Min = 0;
    public double Avg = 0;

    @Override
    public void LoadFrom(CUQueue q) {
        Max = q.LoadDouble();
        Min = q.LoadDouble();
        Avg = q.LoadDouble();
    }

    @Override
    public void SaveTo(CUQueue q) {
        q.Save(Max).Save(Min).Save(Avg);
    }
}
