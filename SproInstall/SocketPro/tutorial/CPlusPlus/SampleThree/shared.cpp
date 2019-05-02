
#include "stdafx.h"
#include "shared.h"

CUQueue& operator << (CUQueue &UQueue, const CTestItem &item)
{
	//make sure compatibility among native and .NET codes
	UQueue << item.m_bNull;
	if(!item.m_bNull)
		UQueue << item.m_vtDT << item.m_lData << item.m_strUID;

	return UQueue;
}

CUQueue& operator >> (CUQueue &UQueue, CTestItem &item)
{
	//make sure compatibility among native and .NET codes
	UQueue >> item.m_bNull;
	if(!item.m_bNull)
		UQueue >> item.m_vtDT >> item.m_lData >> item.m_strUID;
	return UQueue;
}