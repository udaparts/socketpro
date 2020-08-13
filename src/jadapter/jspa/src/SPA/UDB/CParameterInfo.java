package SPA.UDB;

public class CParameterInfo implements SPA.IUSerializer {

    public tagParameterDirection Direction = tagParameterDirection.pdInput; //required
    public short DataType; //required! for example, VT_I4, VT_BSTR, VT_I1|VT_ARRAY (UTF8 string), ....
    public int ColumnSize; //-1 BLOB, string len or binary bytes; ignored for other data types
    public byte Precision; //datetime, decimal or numeric only
    public byte Scale; //datetime, decimal or numeric only
    public String ParameterName = ""; //may be optional, which depends on remote database system

    @Override
    public SPA.CUQueue LoadFrom(SPA.CUQueue q) {
        Direction = tagParameterDirection.forValue(q.LoadInt());
        DataType = q.LoadShort();
        ColumnSize = q.LoadInt();
        Precision = q.LoadByte();
        Scale = q.LoadByte();
        ParameterName = q.LoadString();
        return q;
    }

    @Override
    public SPA.CUQueue SaveTo(SPA.CUQueue q) {
        return q.Save(Direction.getValue()).Save(DataType).Save(ColumnSize).Save(Precision).Save(Scale).Save(ParameterName);
    }
}
