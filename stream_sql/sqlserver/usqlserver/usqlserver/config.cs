using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

public static class Config
{
    internal static string m_WorkingDirectory = "C:\\ProgramData\\MSSQL\\";
    public static string WorkingDirectory
    {
        get
        {
            return m_WorkingDirectory;
        }
    }

    internal static string m_store_or_pfx = "";
    public static string StoreOrPfx
    {
        get
        {
            return m_store_or_pfx;
        }
    }

    internal static string m_subject_or_password = "";
    public static string SubjectOrPassword
    {
        get
        {
            return m_subject_or_password;
        }
    }

    internal static uint m_Param = 1;
    public static uint Param
    {
        get
        {
            return m_Param;
        }
    }
}

