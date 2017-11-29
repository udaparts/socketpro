
import SPA.*;
import java.sql.Timestamp;

public class CRentalDateTimes implements IUSerializer {

    public long rental_id = 0;
    public Timestamp Rental = new Timestamp(0);
    public Timestamp Return = new Timestamp(0);
    public Timestamp LastUpdate = new Timestamp(0);

    @Override
    public void LoadFrom(CUQueue q) {
        rental_id = q.LoadLong();
        Rental = q.LoadTimestamp();
        Return = q.LoadTimestamp();
        LastUpdate = q.LoadTimestamp();
    }

    @Override
    public void SaveTo(CUQueue q) {
        q.Save(rental_id).Save(Rental).Save(Return).Save(LastUpdate);
    }
}
