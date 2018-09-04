using System;
using SocketProAdapter;

public class CMyStruct : IUSerializer
{
    string NullString = null;
    object ObjectNull = null;
    DateTime ADateTime = DateTime.UtcNow;
    public double ADouble;
    public bool ABool;
    public string UnicodeString;
    public byte[] AsciiString;
    public object ObjBool = true;
    public object ObjString;
    public object objArrString;
    public object objArrInt;

    public static CMyStruct MakeOne()
    {
        CMyStruct msOriginal = new CMyStruct();
        int[] arrInt = { 1, 76890 };
        msOriginal.objArrInt = arrInt;
        string[] arrString = { "Hello", "world" };
        msOriginal.objArrString = arrString;
        msOriginal.ObjBool = true;
        msOriginal.ObjString = "test";
        msOriginal.UnicodeString = "Unicode";
        msOriginal.ABool = true;
        msOriginal.ADouble = 1234.567;
        msOriginal.AsciiString = System.Text.ASCIIEncoding.ASCII.GetBytes("ASCII");
        return msOriginal;
    }

    //make sure both serialization and de-serialization match against each other.
    public void LoadFrom(CUQueue UQueue)
    {
        UQueue.Load(out NullString)
            .Load(out ObjectNull)
            .Load(out ADateTime)
            .Load(out ADouble)
            .Load(out ABool)
            .Load(out UnicodeString) //UTF16-lowendian
            .Load(out AsciiString)
            .Load(out ObjBool)
            .Load(out ObjString) //UTF16-lowendian
            .Load(out objArrString) //UTF16-lowendian
            .Load(out objArrInt)
            ;
    }

    //make sure both serialization and de-serialization match against each other.
    public void SaveTo(CUQueue UQueue)
    {
        UQueue.Save(NullString) //4 bytes for length
            .Save(ObjectNull) //2 bytes for data type
            .Save(ADateTime) //8 bytes for ulong with accuracy to 1 micro-second
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
