
#include "../../../include/aserverw.h"
#include "../IpLookup_i.h"
#include "icosdefines.h"

#ifndef ___SOCKETPRO_SERVICES_IMPL_IPLOOKUPIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_IPLOOKUPIMPL_H__

using namespace SPA;
using namespace SPA::ServerSide;

/* **** including all of defines, service id(s) and request id(s) ***** */

namespace iCOS 
{
	//server implementation for service GeoIp
	class CGeoIpPeer : public CClientPeer
	{
	public:
		static void MakeFakeDataForTest();
		static void DestroyGpuIpBlockPool();

	protected:
		virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len);
		virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len);

	private:
		void LookupIps();
		void DoGPULookups(IPBlock *pIPBlock, unsigned int count);
		static IPBlock *LockGpuIpBlock();
		static void Recycle(IPBlock *pGpuIpBlock);

	private:
		static SPA::CUCriticalSection m_cs;
		static std::vector<IPBlock*> m_vGpuIpBlock;
		

	private:
		//this is for fake testing only
		static std::vector<GEOLocation> m_vFakeLocation;
	};

}

#endif