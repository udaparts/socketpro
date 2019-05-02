#include "stdafx.h"
#include "IsConnected.h"

int SendUserMessage(char *strMsg, char *strUserId)
{
	CStringA str = strUserId;
	str = str.Trim();
	CComBSTR bstrUserID(str);
	CComVariant vtMessage(strMsg);
	CAutoLock al(&g_csMySQLPush.m_sec);
	if(g_pClientSocket == NULL || !g_pClientSocket->IsConnected())
		return 1;
	return g_pClientSocket->GetPush()->SendUserMessage(vtMessage, bstrUserID) ? 0 : 1;
}


int Notify(char *strMsg, char *groups)
{
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
	CComVariant vtMsg(strMsg);
	CAutoLock al(&g_csMySQLPush.m_sec);
	if(g_pClientSocket == NULL || !g_pClientSocket->IsConnected())
		return 1;
	return g_pClientSocket->GetPush()->Broadcast(vtMsg, aGroup.GetData(), aGroup.GetSize()) ? 0 : 1;
}