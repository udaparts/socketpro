using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace CloudClientSharedForUnitTest
{
    public class TestStructure : SocketProAdapter.IUSerializer
    {
        public int n;
        public string str;

        public override string ToString()
        {
            string s = "{n = " + n.ToString();
            s += (", str = " + str + "}");
            return s;
        }

        #region IUSerializer Members

        public int LoadFrom(SocketProAdapter.CUQueue UQueue)
        {
            int start = UQueue.GetSize();
            UQueue.Pop(out n);
            UQueue.Load(out str);
            return start - UQueue.GetSize();
        }

        public void SaveTo(SocketProAdapter.CUQueue UQueue)
        {
            UQueue.Push(n);
            UQueue.Save(str);
        }

        #endregion
    }
}
