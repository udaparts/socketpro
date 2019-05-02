using System;
using SocketProAdapter;

namespace SampleThreeShared
{
	public class CTestItem : IUSerializer
	{
		public CTestItem()
		{
			m_dt = System.DateTime.Now;
		}
		public void SaveTo(CUQueue UQueue)
		{
            //make sure that both native and .NET codes are compatible to each of other
			object obj = m_dt;
			UQueue.Push(obj); //save datetime as native VARIANT datetime
			UQueue.Push(m_lData);
            UQueue.Save(m_strUID);
		}
		public int LoadFrom(CUQueue UQueue)
		{
			int nLen;
			object obj = null;
            //make sure that both native and .NET codes are compatible to each of other
			nLen = UQueue.Pop(out obj);
			m_dt = (DateTime)obj;
			nLen += UQueue.Pop(out m_lData);
            nLen += UQueue.Load(out m_strUID);
			return nLen;
		}
		public DateTime	m_dt;
		public long		m_lData;
		public string	m_strUID;
	}
}