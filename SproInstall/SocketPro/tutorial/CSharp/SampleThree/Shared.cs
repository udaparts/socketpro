using System;
using SocketProAdapter;

namespace SampleThreeShared
{
	class CDefines
	{
		//define your own service id starting from 0x10000001
		//udaparts reserves service ids from 0x00000000 through 0x10000000
		public const int sidSThreeSvs = 0x1F322311;
		
		//define your request ids starting from 2001
		//udaparts reserves service ids from 0x0000 through 0x2000
		public const short idSendOneItemToServer = 0x2001;
		public const short idSendAFewItemsToServer = 0x2002;
		public const short idSendALotItemsToServer = 0x2003;
		public const short idGetOneItemFromServer = 0x2004;
		public const short idGetAFewItemsFromServer = 0x2005;
		//slow request
		public const short idGetALotItemsFromServer = 0x2006;
		
		public const short idStartToSendALotItemsToServer = 0x2007;
		public const short idStartToGetALotItemsFromServer = 0x2008;
		
		//slow request
		public const short idSendingALotItemsToServer = 0x2009;
		public const short idGetingALotItemsFromServer = 0x200A;

		public const short idPushItemsIntoGlobalStack = 0x200B;
		public const short idPopItemsFromGlobalStack = 0x200C;
		
		//slow request
		public const short idSendItemsToServerByDotNetSerializer = 0x200D;
		//slow request
		public const short idGetItemsFromServerByDotNetSerializer = 0x200E;
	}

	class CTestItem
	{
		public CTestItem()
		{
			m_dt = System.DateTime.Now;
		}
		
		public void PushMe(CUQueue UQueue)
		{
			UQueue.Push(m_dt);
			UQueue.Push(m_lData);

			//Pushing string in obejct makes code simpler.
			//Orherwise, you have to push a int indicating its length first
			UQueue.Push((object)m_strUID);
		}

		public int PopMe(CUQueue UQueue)
		{
			int nLen;
			object ob = null;
			nLen = UQueue.Pop(ref m_dt);
			nLen += UQueue.Pop(ref m_lData);
			
			//Pop an object first, and then unbox into a string
			nLen += UQueue.Pop(ref ob);
			m_strUID = (string)ob;

			return nLen;
		}

		public DateTime	m_dt;
		public long		m_lData;
		public string	m_strUID;
	}
}