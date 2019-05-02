
#include "udemo.h"
#include "mysamplesvs.h"

CMySampleService	g_MySampleService;
unsigned short		g_SlowRequestIDs[2];

bool WINAPI InitServerLibrary(int nParam)
{
	g_SlowRequestIDs[0] = idDoSomeWorkWithMSSQLServer;
	g_SlowRequestIDs[1] = idSleep;
	
	//make sure that service id is set here
	g_MySampleService.SetSvsID(sidMySampleService);

	//if you need more callbacks, refer to CBaseService::AddMe and set here

	return true;
}

void WINAPI UninitServerLibrary()
{
	g_MySampleService.RemoveMe();
}

unsigned short WINAPI GetNumOfServices()
{
	return 1;
}

unsigned long WINAPI GetAServiceID(unsigned short usIndex)
{
	if(usIndex >= GetNumOfServices())
		return 0;
	return sidMySampleService;
}

CSvsContext WINAPI GetOneSvsContext(unsigned long ulSvsID)
{
	if(ulSvsID == sidMySampleService)
		return g_MySampleService.m_SvsContext;
	CSvsContext	SvsContext;
	memset(&SvsContext, 0, sizeof(SvsContext));
	return SvsContext;
}

unsigned short WINAPI GetNumOfSlowRequests(unsigned long ulSvsID)
{
	if(ulSvsID != sidMySampleService)
		return 0;
	return sizeof(g_SlowRequestIDs)/sizeof(unsigned short);
}

unsigned short WINAPI GetOneSlowRequestID(unsigned long ulSvsID, unsigned long ulIndex)
{
	if(ulIndex >= GetNumOfSlowRequests(ulSvsID))
		return 0;
	return g_SlowRequestIDs[ulIndex];
}



