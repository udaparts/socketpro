
import SPA.CUQueue;
import SPA.IUSerializer;
import java.sql.Timestamp;

public class CRentalDateTimes implements IUSerializer {

    public long rental_id = 0;
    public Timestamp Rental = new Timestamp(0);
    public Timestamp Return = new Timestamp(0);
    public Timestamp LastUpdate = new Timestamp(0);

    @Override
    public void LoadFrom(CUQueue q) {

    }

    @Override
    public void SaveTo(CUQueue q) {

    }
}
