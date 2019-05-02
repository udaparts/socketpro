#include "stdafx.h"
#include "IsConnected.h"
#include "MyConnectionContext.h"

extern CMyConnectionContext		g_cc;


my_bool IsConnected_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	StartPush();
	return (char)0;
}

void IsConnected_deinit(UDF_INIT *initid)
{

}

longlong IsConnected(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
	CAutoLock al(&g_csMySQLPush.m_sec);
	if(g_pClientSocket == NULL)
		return 0;
	return g_pClientSocket->IsConnected() ? 1 : 0;
}

my_bool Disconnect_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	return (char)0;
}

void Disconnect_deinit(UDF_INIT *initid)
{

}

longlong Disconnect(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
	CAutoLock al(&g_csMySQLPush.m_sec);
	if(g_pClientSocket == NULL)
		return 0;
	Stop();
	return 0;
}

my_bool Connect_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	if(args->arg_count != 1 || args->arg_type[0] != STRING_RESULT)
	{
		strcpy(message, "Connect() requires one SocketPro server conection string.");
        return 7;
	}
	return (char)0;
}

void Connect_deinit(UDF_INIT *initid)
{

}

longlong Connect(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
	int nPos = 0;
	CStringA strPart;
	CStringA str = args->args[0];
	if(str.GetLength() == 0)
	{
		CAutoLock al(&g_csMySQLPush.m_sec);
		if(g_pClientSocket == NULL)
			return 1;
		return g_pClientSocket->IsConnected() ? 0 : 1;
	}
	CMyConnectionContext cc;
	CSimpleMap<CStringA, CStringA> map;
	strPart = str.Tokenize(";", nPos);
	while(nPos >= 0 || strPart.GetLength())
	{
		int nFind = strPart.Find('=');
		if(nFind != -1)
		{
			CStringA key = strPart.Left(nFind).Trim().MakeLower();
			CStringA value = strPart.Mid(nFind + 1).Trim();
			if(key.GetLength() && value.GetLength())
				map.Add(key, value);
		}
		if(nPos < 0)
			break;
		strPart = str.Tokenize(";", nPos);
	}
	
	nPos = map.FindKey("host");
	if (nPos != -1)
		cc.m_strHost = map.GetValueAt(nPos).MakeLower();
	nPos = map.FindKey("server");
	if (nPos != -1)
		cc.m_strHost = map.GetValueAt(nPos).MakeLower();

	nPos = map.FindKey("uid");
	if (nPos != -1)
		cc.m_strUID = map.GetValueAt(nPos).MakeLower();
	nPos = map.FindKey("userid");
	if (nPos != -1)
		cc.m_strUID = map.GetValueAt(nPos).MakeLower();

	nPos = map.FindKey("pwd");
	if (nPos != -1)
		cc.m_strPassword= map.GetValueAt(nPos);
	nPos = map.FindKey("password");
	if (nPos != -1)
		cc.m_strPassword = map.GetValueAt(nPos);

	nPos = map.FindKey("zip");
	if (nPos != -1)
	{
		str = map.GetValueAt(nPos).MakeLower();
		if(str == "true")
			cc.m_bZip = true;
		else
			cc.m_bZip = atoi(str) ? true : false;
	}
	
	nPos = map.FindKey("port");
	if (nPos != -1)
	{
		str = map.GetValueAt(nPos);
		cc.m_nPort = atoi(str);
	}

	nPos = map.FindKey("encryption");
	if (nPos != -1)
	{
		str = map.GetValueAt(nPos);
		cc.m_EncrytionMethod = (short)atoi(str);
	}
	
	if(cc.m_EncrytionMethod != NoEncryption)
	{
		nPos = map.FindKey("strict");
		if (nPos != -1)
		{
			str = map.GetValueAt(nPos);
			cc.m_bStrict = (atoi(str) != 0);
		}

		nPos = map.FindKey("verify");
		if (nPos != -1)
			cc.m_strLocation = map.GetValueAt(nPos);
	}

	CAutoLock al(&g_csMySQLPush.m_sec);
	StartPush();
	if(g_hThread == NULL)
		return 6;
	if(g_pClientSocket == NULL)
		return 5;
	if(!g_pClientSocket->IsConnected())
		g_cc = cc;
	else if(g_cc.m_EncrytionMethod != cc.m_EncrytionMethod || g_cc.m_strHost != cc.m_strHost ||
		g_cc.m_nPort != cc.m_nPort || g_cc.m_strPassword != cc.m_strPassword || g_cc.m_strUID != cc.m_strUID)
	{
		g_pClientSocket->Disconnect();
		g_cc = cc;
	}
	else if(g_cc.m_bZip != cc.m_bZip)
	{
		g_pClientSocket->GetIUSocket()->put_ZipIsOn(cc.m_bZip ? VARIANT_TRUE : VARIANT_FALSE);
        g_pClientSocket->GetIUSocket()->TurnOnZipAtSvr(cc.m_bZip ? VARIANT_TRUE : VARIANT_FALSE);
        g_cc = cc;
	}
	if(g_pClientSocket->IsConnected())
		return 0;
	bool bSuc = ::ResetEvent(g_hEvent) ? true : false;
	int nIndex = 0;
	while (!::PostThreadMessage(g_dwThreadId, WM_MYSQL_PUSH_CONNECT, 0, 0))
    {
        Sleep(1);
        nIndex++;
        if (nIndex > 5)
            break;
    }
    if (nIndex > 5)
        return 4;
	::WaitForSingleObject(g_hEvent, 61000);
	if(g_cc.m_VerifyCode != 0)
		return g_cc.m_VerifyCode;
	return g_pClientSocket->IsConnected() ? 0 : 1;
}