#ifndef __SAMPLE_THREE_SHARED_H__
#define __SAMPLE_THREE_SHARED_H__

#include "sprowrap.h"
using namespace SocketProAdapter;
	
class CTestItem
{
public:
	CTestItem() : m_bNull(false), m_lData(0)
	{
		SYSTEMTIME	sysTime;
		::GetSystemTime(&sysTime);
		m_vtDT.vt = VT_DATE;
		::SystemTimeToVariantTime(&sysTime, &m_vtDT.date);
	}

public:
	bool			m_bNull;
	CComVariant		m_vtDT;
	LONGLONG		m_lData;
	CComBSTR		m_strUID;
};

//you must override the following operators for complex structures
CUQueue& operator << (CUQueue &UQueue, const CTestItem &item);
CUQueue& operator >> (CUQueue &UQueue, CTestItem &item);

#endif