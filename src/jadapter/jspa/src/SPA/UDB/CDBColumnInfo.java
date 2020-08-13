package SPA.UDB;

public class CDBColumnInfo implements SPA.IUSerializer {

    public final static int FLAG_NOT_NULL = 0x1;
    public final static int FLAG_UNIQUE = 0x2;
    public final static int FLAG_PRIMARY_KEY = 0x4;
    public final static int FLAG_AUTOINCREMENT = 0x8;
    public final static int FLAG_NOT_WRITABLE = 0x10;
    public final static int FLAG_ROWID = 0x20;
    public final static int FLAG_XML = 0x40;
    public final static int FLAG_JSON = 0x80;
    public final static int FLAG_CASE_SENSITIVE = 0x100;
    public final static int FLAG_IS_ENUM = 0x200;
    public final static int FLAG_IS_SET = 0x400;
    public final static int FLAG_IS_UNSIGNED = 0x800;
    public final static int FLAG_IS_BIT = 0x1000;

    public String DBPath = "";
    public String TablePath = "";
    public String DisplayName = "";
    public String OriginalName = "";
    public String DeclaredType = "";
    public String Collation = "";
    public int ColumnSize = 0;
    public int Flags = 0;
    public short DataType = SPA.tagVariantDataType.sdVT_NULL;
    public byte Precision = 0;
    public byte Scale = 0;

    @Override
    public SPA.CUQueue LoadFrom(SPA.CUQueue q) {
        DBPath = q.LoadString();
        TablePath = q.LoadString();
        DisplayName = q.LoadString();
        OriginalName = q.LoadString();
        DeclaredType = q.LoadString();
        Collation = q.LoadString();
        ColumnSize = q.LoadInt();
        Flags = q.LoadInt();
        DataType = q.LoadShort();
        Precision = q.LoadByte();
        Scale = q.LoadByte();
        return q;
    }

    @Override
    public SPA.CUQueue SaveTo(SPA.CUQueue q) {
        q.Save(DBPath).Save(TablePath).Save(DisplayName).Save(OriginalName).Save(DeclaredType).Save(Collation);
        return q.Save(ColumnSize).Save(Flags).Save(DataType).Save(Precision).Save(Scale);
    }
}
