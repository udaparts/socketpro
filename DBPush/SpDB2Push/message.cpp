#include "stdafx.h"
#include "IsConnected.h"

#ifdef __cplusplus
extern "C"
#endif 

void SQL_API_FN SendUserMessage(SQLUDF_VARCHAR *strMessage, SQLUDF_VARCHAR *strUserID, SQLUDF_INTEGER *pOut)
{
	int res;
	CStringA str = strUserID;
	str = str.Trim();
	CComBSTR bstrUserID(str);
	CComVariant vtMessage(strMessage);
	{
		CAutoLock al(&g_csMySQLPush.m_sec);
		if(g_pClientSocket == NULL || !g_pClientSocket->IsConnected())
			res = 1;
		else
			res = g_pClientSocket->GetPush()->SendUserMessage(vtMessage, bstrUserID) ? 0 : 1;
	}
	if(pOut != NULL)
		*pOut = res;
}

#ifdef __cplusplus
extern "C"
#endif 
void SQL_API_FN Notify(SQLUDF_VARCHAR *strMessage, SQLUDF_VARCHAR *groups, SQLUDF_INTEGER *pOut)
{
	int res;
	int nPos = 0;
	CStringA str;
	CSimpleArray<unsigned long> aGroup;
	CStringA strGroups = groups;
	strGroups = strGroups.Trim(" {}[]()");
	str = strGroups.Tokenize(";|:,", nPos);
	while(nPos >= 0 || str.GetLength())
	{
		int nGroup = atoi(str);
		if(nGroup > 0)
			aGroup.Add(nGroup);
		str = strGroups.Tokenize(";|:,", nPos);
	}
	CComVariant vtMsg(strMessage);
	CAutoLock al(&g_csMySQLPush.m_sec);
	if(g_pClientSocket == NULL || !g_pClientSocket->IsConnected())
		res = 1;
	else
		res = g_pClientSocket->GetPush()->Broadcast(vtMsg, aGroup.GetData(), aGroup.GetSize()) ? 0 : 1;
	if(pOut != NULL)
		*pOut = res;
}