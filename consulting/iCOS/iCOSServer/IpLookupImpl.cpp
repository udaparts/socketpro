#include "stdafx.h"
#include "IpLookupImpl.h"
#include "myspaserver.h"

namespace iCOS 
{
	std::vector<GEOLocation> CGeoIpPeer::m_vFakeLocation;
	void CGeoIpPeer::MakeFakeDataForTest()
	{
		GEOLocation loc;

		loc.CountryCode[0] = 'U';
		loc.CountryCode[1] = 'S';
		loc.RegionCode[0] = 'G';
		loc.RegionCode[1] = 'A';
		::wcscpy_s(loc.City, L"Atlanta");
		loc.Len = (unsigned int)::wcslen(loc.City) * sizeof(wchar_t);
		m_vFakeLocation.push_back(loc);

		loc.CountryCode[0] = 'C';
		loc.CountryCode[1] = 'N';
		loc.RegionCode[0] = '0';
		loc.RegionCode[1] = '3';
		::wcscpy_s(loc.City, L"Nanchang");
		loc.Len = (unsigned int)::wcslen(loc.City) * sizeof(wchar_t);
		m_vFakeLocation.push_back(loc);

		loc.CountryCode[0] = 'F';
		loc.CountryCode[1] = 'R';
		loc.RegionCode[0] = 'B';
		loc.RegionCode[1] = '2';
		::wcscpy_s(loc.City, L"Saint-nicolas-de-port");
		loc.Len = (unsigned int)::wcslen(loc.City) * sizeof(wchar_t);
		m_vFakeLocation.push_back(loc);

		loc.CountryCode[0] = 'H';
		loc.CountryCode[1] = 'K';
		loc.RegionCode[0] = '0';
		loc.RegionCode[1] = '0';
		::wcscpy_s(loc.City, L"Central District");
		loc.Len = (unsigned int)::wcslen(loc.City) * sizeof(wchar_t);
		m_vFakeLocation.push_back(loc);

		loc.CountryCode[0] = 'T';
		loc.CountryCode[1] = 'R';
		loc.RegionCode[0] = '7';
		loc.RegionCode[1] = '4';
		::wcscpy_s(loc.City, L"Dogan");
		loc.Len = (unsigned int)::wcslen(loc.City) * sizeof(wchar_t);
		m_vFakeLocation.push_back(loc);
	}

	SPA::CUCriticalSection CGeoIpPeer::m_cs;
	std::vector<IPBlock*> CGeoIpPeer::m_vGpuIpBlock;
	void CGeoIpPeer::LookupIps()
	{
		unsigned int n;
		unsigned short LOOKUP_VERSION;
		m_UQueue >> LOOKUP_VERSION;

		assert((m_UQueue.GetSize() % sizeof (CIndexIpCode)) == 0);
		
		unsigned int count = m_UQueue.GetSize() / sizeof (CIndexIpCode);
		CIndexIpCode *pIndexIpCode = (CIndexIpCode*) m_UQueue.GetBuffer();

		//use CScopeUQueueForIPBlock to avoid allocating and deallocating memory repeatedly
		CScopeUQueueForIPBlock suIPBlock;
		SPA::CUQueue &qIpBlock = *suIPBlock;

	#ifdef _DEBUG
		unsigned int maxAllowedIPBlocks = qIpBlock.GetMaxSize() / sizeof(IPBlock);
		assert(maxAllowedIPBlocks >= count);
	#endif
		
		IPBlock *pIPBlock = (IPBlock *)qIpBlock.GetBuffer();
		for (n = 0; n < count; ++n)
		{
			//copy ip address in unsigned int into IPBlock
			pIPBlock[n].TargetIP = pIndexIpCode[n].IpCode;
		}

		DoGPULookups(pIPBlock, count);

		//we are going to do inline replacement for 4 bytes (unsigned int ip --> Country and region codes)
		//Therefore, we avoid both deserialization and serialization.
		for (n=0; n<count; ++n)
		{
			GEOLocation *LocationPtr = pIPBlock[n].LocationPtr;

			//copy 4 bytes (CountryCode and RegionCode) back into IpCode 
			memcpy(&(pIndexIpCode[n].IpCode), &(LocationPtr->CountryCode[0]), sizeof(unsigned int));
		}

		//send ids and codes finally
		SendResult(idLookupGeoIp, m_UQueue.GetBuffer(), m_UQueue.GetSize());
	}

	void CGeoIpPeer::OnFastRequestArrive(unsigned short reqId, unsigned int len)
	{

	}

	int CGeoIpPeer::OnSlowRequestArrive(unsigned short reqId, unsigned int len)
	{
		switch (reqId)
		{
		case idLookupGeoIp:
			LookupIps();
			break;
		default:
			assert(false); //not implemented
			break;
		}
		return 0;
	}

}