#include "stdafx.h"
#include "IsConnected.h"

my_bool SendUserMessage_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	if(args->arg_count != 2 || args->arg_type[0] != STRING_RESULT || args->arg_type[1] != STRING_RESULT)
	{
		strcpy(message, "Function SendUserMessage requires two input strings for message and receiver's user id.");
        return 7;
	}
	CStringA str = args->args[1];
	if(str.IsEmpty())
	{
		strcpy(message, "Function SendUserMessage requires a valid receiver's user id for the 2nd argument.");
        return 3;
	}
	return (char)0;
}

void SendUserMessage_deinit(UDF_INIT *initid)
{

}

longlong SendUserMessage(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
	CStringA str = args->args[1];
	str = str.Trim();
	CComBSTR bstrUserID(str);
	CComVariant vtMessage(args->args[0]);
	CAutoLock al(&g_csMySQLPush.m_sec);
	if(g_pClientSocket == NULL || !g_pClientSocket->IsConnected())
		return 1;
	return g_pClientSocket->GetPush()->SendUserMessage(vtMessage, bstrUserID) ? 0 : 1;
}


my_bool Notify_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	if(args->arg_count != 2 || args->arg_type[0] != STRING_RESULT || args->arg_type[1] != STRING_RESULT)
	{
		strcpy(message, "Function Notify requires two input strings for message and chat groups.");
        return 7;
	}
	return (char)0;
}

void Notify_deinit(UDF_INIT *initid)
{

}

longlong Notify(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
	int nPos = 0;
	CStringA str;
	CSimpleArray<unsigned long> aGroup;
	CStringA strGroups = args->args[1];
	strGroups = strGroups.Trim(" {}[]()");
	str = strGroups.Tokenize(";|:,", nPos);
	while(nPos >= 0 || str.GetLength())
	{
		int nGroup = atoi(str);
		if(nGroup > 0)
			aGroup.Add(nGroup);
		str = strGroups.Tokenize(";|:,", nPos);
	}
	CComVariant vtMsg(args->args[0]);
	CAutoLock al(&g_csMySQLPush.m_sec);
	if(g_pClientSocket == NULL || !g_pClientSocket->IsConnected())
		return 1;
	return g_pClientSocket->GetPush()->Broadcast(vtMsg, aGroup.GetData(), aGroup.GetSize()) ? 0 : 1;
}