using System.Collections.Generic;

namespace SocketProAdapter
{
    namespace ClientSide
    {
        internal class UDelegate<TDel> : List<TDel>
        {
            public UDelegate(object cs)
            {
                if (cs == null)
                {
                    cs = new object();
                }
                m_cs = cs;
            }

            public void add(TDel del)
            {
                if (del == null)
                {
                    return;
                }
                lock (m_cs)
                {
                    if (IndexOf(del) == -1)
                    {
                        Add(del);
                    }
                }
            }

            public void remove(TDel del)
            {
                if (del == null)
                {
                    return;
                }
                lock (m_cs)
                {
                    int index = IndexOf(del);
                    if (index != -1)
                    {
                        RemoveAt(index);
                    }
                }
            }
            private object m_cs;
        }
    }
}
