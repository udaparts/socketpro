using System;
using SocketProAdapter;

class Program
{
    static void Save(CMyStruct ms, CUQueue q)
    {
        ms.SaveTo(q);
    }

    static CMyStruct Load(CUQueue q)
    {
        CMyStruct ms = new CMyStruct();
        ms.LoadFrom(q);
        return ms;
    }

    static void Main(string[] args)
    {
        using (CScopeUQueue su = new CScopeUQueue())
        {
            CMyStruct msOriginal = CMyStruct.MakeOne();

            msOriginal.SaveTo(su.UQueue);

            CMyStruct ms = Load(su.UQueue);
            System.Diagnostics.Debug.Assert(su.UQueue.GetSize() == 0);

            //check if both msOriginal and ms are equal in value.
        }
    }
}

