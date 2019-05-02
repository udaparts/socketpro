#include "stdafx.h"
#include "poimpl.h"

unsigned long CTOnePeer::m_uGlobalCount = 0;
unsigned long CTOnePeer::m_uGlobalFastCount = 0;
CComAutoCriticalSection	CTOnePeer::m_cs;

