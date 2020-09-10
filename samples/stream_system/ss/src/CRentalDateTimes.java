
import SPA.*;
import java.sql.Timestamp;

public class CRentalDateTimes implements IUSerializer {

    public long rental_id = 0;
    public Timestamp Rental = new Timestamp(0);
    public Timestamp Return = new Timestamp(0);
    public Timestamp LastUpdate = new Timestamp(0);
    private final static long ZERO_TICK = 0;

    @Override
    public CUQueue LoadFrom(CUQueue q) {
        rental_id = q.LoadLong();
        Rental = q.LoadTimestamp();
        Return = q.LoadTimestamp();
        LastUpdate = q.LoadTimestamp();
        return q;
    }

    @Override
    public CUQueue SaveTo(CUQueue q) {
        q.Save(rental_id);
        if (Rental == null) {
            q.Save(ZERO_TICK);
        } else {
            q.Save(Rental);
        }
        if (Return == null) {
            q.Save(ZERO_TICK);
        } else {
            q.Save(Return);
        }
        if (LastUpdate == null) {
            q.Save(ZERO_TICK);
        } else {
            q.Save(LastUpdate);
        }
        return q;
    }
}
