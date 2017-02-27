

using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;

public static class spmssqlpush
{
    [DllImport("udbubase.dll")]
    private static extern int SetSocketProConnectionString([MarshalAs(UnmanagedType.LPWStr)]string conn);

    [DllImport("udbubase.dll")]
    private static unsafe extern ulong GetSocketProConnections(uint* groups, uint count);

    [DllImport("udbubase.dll")]
    private static unsafe extern ulong NotifySocketProDatabaseEvent(uint* groups, uint count, int eventType, [MarshalAs(UnmanagedType.LPWStr)]string queryFilter, uint* index, uint size);

    public static int SetConnectionString(string connectionString)
    {
        return SetSocketProConnectionString(connectionString);
    }

    public static string NotifyDatabaseEvent(int eventType, string queryFilter, string groups)
    {
        if (queryFilter == null)
            queryFilter = "";
        if (groups == null)
            groups = "";
        string[] grps = groups.Split(',');
        List<uint> v = new List<uint>();
        foreach (string s in grps)
        {
            uint n;
            if (uint.TryParse(s, out n))
            {
                v.Add(n);
            }
        }
        ulong ret;
        uint count = 1024;
        uint[] index = new uint[count];
        unsafe
        {
            fixed (uint* p = index)
            {
                uint[] vG = v.ToArray();
                fixed (uint* g = vG)
                {
                    ret = NotifySocketProDatabaseEvent(g, (uint)v.Count, eventType, queryFilter, p, count);
                }
            }
        }
        uint suc = (uint)ret;
        uint fail = (uint)(ret >> 32);
        if (count > suc + fail)
            count = suc + fail;
        string str = "";
        for (uint n = 0; n < count; ++n)
        {
            str += ((index[n] != 0) ? '0' : '1');
        }
        return str;
    }

    public static string GetConnections()
    {
        ulong ret;
        uint count = 1024;
        uint[] index = new uint[count];
        unsafe
        {
            fixed (uint* p = index)
            {
                ret = GetSocketProConnections(p, count);
            }
        }
        uint suc = (uint)ret;
        uint fail = (uint)(ret >> 32);
        if (count > suc + fail)
            count = suc + fail;
        string s = "";
        for (uint n = 0; n < count; ++n)
        {
            s += ((index[n] != 0) ? '0' : '1');
        }
        return s;
    }
}

