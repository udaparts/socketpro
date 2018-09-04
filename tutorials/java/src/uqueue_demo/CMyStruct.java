package uqueue_demo;

import SPA.CUQueue;

public class CMyStruct implements SPA.IUSerializer {

    private String NullString = null;
    private Object ObjectNull = null;
    private java.util.Date ADateTime = java.util.Calendar.getInstance().getTime();
    public double ADouble;
    public boolean ABool;
    public String UnicodeString;
    public byte[] AsciiString;
    public Object ObjBool = true;
    public Object ObjString;
    public Object objArrString;
    public Object objArrInt;

    @Override
    public int hashCode() {
        return super.hashCode();
    }

    @Override
    public boolean equals(Object obj) {
        CMyStruct ms;
        if (obj == null) {
            return false;
        }
        if (obj == this) {
            return true;
        }
        if (obj instanceof CMyStruct) {
            ms = (CMyStruct) obj;
        } else {
            return false;
        }
        return ADateTime.equals(ms.ADateTime)
                && ADouble == ms.ADouble
                && ABool == ms.ABool
                && UnicodeString.equals(ms.UnicodeString)
                && java.util.Arrays.equals(this.AsciiString, ms.AsciiString)
                && (boolean)ObjBool == (boolean)ms.ObjBool
                && ((String)ObjString).equals(ms.ObjString)
                && java.util.Arrays.equals((String[])objArrString, (String[])ms.objArrString)
                && java.util.Arrays.equals((int[])objArrInt, (int[])ms.objArrInt);
    }

    public static CMyStruct MakeOne() {
        CMyStruct msOriginal = new CMyStruct();
        int[] arrInt = {1, 76890};
        msOriginal.objArrInt = arrInt;
        String[] arrString = {"Hello", "world"};
        msOriginal.objArrString = arrString;
        msOriginal.ObjBool = true;
        msOriginal.ObjString = "test";
        msOriginal.UnicodeString = "Unicode";
        msOriginal.ABool = true;
        msOriginal.ADouble = 1234.567;
        String s = "ASCII";
        msOriginal.AsciiString = s.getBytes(java.nio.charset.Charset.forName("US-ASCII"));
        return msOriginal;
    }

    @Override
    public void LoadFrom(CUQueue UQueue) {
        NullString = UQueue.LoadString();
        ObjectNull = UQueue.LoadObject();
        ADateTime = UQueue.LoadDate();
        ADouble = UQueue.LoadDouble();
        ABool = UQueue.LoadBoolean();
        UnicodeString = UQueue.LoadString();
        AsciiString = UQueue.LoadBytes();
        ObjBool = UQueue.LoadObject();
        ObjString = UQueue.LoadObject();
        objArrString = UQueue.LoadObject();
        objArrInt = UQueue.LoadObject();
    }

    @Override
    public void SaveTo(CUQueue UQueue) {
        UQueue.Save(NullString) //4 bytes for length
                .Save(ObjectNull) //2 bytes for data type
                .Save(ADateTime) //8 bytes for long with accuracy to one micro-second
                .Save(ADouble) //8 bytes
                .Save(ABool) //1 byte
                .Save(UnicodeString) //4 bytes for string length + (length * 2) bytes for string data -- UTF16-lowendian
                .Save(AsciiString) //4 bytes for ASCII string length + length bytes for string data
                .Save(ObjBool) //2 bytes for data type + 2 bytes for variant bool
                .Save(ObjString) //2 bytes for data type + 4 bytes for string length + (length * 2) bytes for string data -- UTF16-lowendian
                .Save(objArrString) //2 bytes for data type + 4 bytes for array size + (4 bytes for string length + (length * 2) bytes for string data) * arraysize -- UTF16-lowendian
                .Save(objArrInt) //2 bytes for data type + 4 bytes for array size + arraysize * 4 bytes for int data
                ;
    }
}
