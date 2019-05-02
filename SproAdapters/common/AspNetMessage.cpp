#include "StdAfx.h"
#include "AspNetMessage.h"
using namespace System::Web::Script;
using namespace System::Web::Script::Serialization;

namespace SocketProAdapter
{
namespace ClientSide
{

CCometImpl::CCometImpl()
{
	m_strVersion = "1.1";
	m_strUrlToComet = "";
	m_strUJson = "";
	m_bIE8 = false;
}

String^ CCometImpl::GetJsonArray(array<long> ^myArray)
{
	String ^str = "[";
	if(myArray != nullptr)
	{
		int n;
		for(n=0; n<myArray->Length; n++)
		{
			if(n>0)
				str += ",";
			str += (myArray[n]).ToString();
		}
	}
	str += "]";
	return str;
}

String^ CCometImpl::GetIpv4(String ^strIpAddr)
{
	String ^str = "";
	for each (IPAddress ^IPA in Dns::GetHostAddresses(strIpAddr))
    {
		if (IPA->AddressFamily == System::Net::Sockets::AddressFamily::InterNetwork)
        {
            str = IPA->ToString();
            return str;
        }
    }
	for each (IPAddress ^IPA in Dns::GetHostAddresses(Dns::GetHostName()))
    {
		if (IPA->AddressFamily == System::Net::Sockets::AddressFamily::InterNetwork)
        {
            str = IPA->ToString();
            return str;
        }
    }
	return strIpAddr;
}
String^ CCometImpl::Enter(String ^strSubscriberIpAddress, String ^strUserId, array<long> ^Groups)
{
	String ^strHttp;
	String ^strDomain;
	int nPort;
	String ^strPath = "";
	String ^str;
	String ^strQuery;
	WebResponse ^wr;
	String ^strHost = strSubscriberIpAddress;
	array<WCHAR> ^sep = {'.'};
	array<String^> ^dots = strHost->Split(sep);
	if(dots->Length != 4)
		strHost = GetIpv4(strHost);
	{
		bool bSSL;
		CAutoLock	AutoLock(&g_cs.m_sec);
		str = m_strUrlToComet->ToLower();
		int nSep = str->IndexOf("://");
		if(nSep < 4 || nSep > 5 || str->IndexOf("http") != 0)
			throw gcnew Exception("Url address to a remote COMET server not available!");
		bSSL = (str->IndexOf("https://") == 0);
		nSep = str->IndexOf('/', 7 + (bSSL ? 1 : 0));
		if(nSep != -1)
			strPath =  m_strUrlToComet->Substring(nSep + 1);
		str = str->Substring(0, nSep);
		str = str->Substring(7 + (bSSL ? 1 : 0));
		nSep = str->IndexOf(':');
		if(nSep == -1)
		{
			nPort = bSSL ? 443 : 80;
			strDomain = str;
		}
		else
		{
			strDomain = str->Substring(0, nSep);
			nPort = int::Parse(str->Substring(nSep + 1));
		}
		strHttp = bSSL ? "https:" : "http:";
		strQuery = String::Format(	"{0}?UJS_DATA={{\"method\":\"enter\",\"parameters\":{{\"groups\":{1},\"userId\":\"{2}\"}},\"id\":{3},\"version\":{4}}}&BrowserIpAddr={5}",
			m_strUrlToComet, GetJsonArray(Groups), strUserId, "0", Version, strHost);
	}
	HttpWebRequest ^hwr = (HttpWebRequest^)WebRequest::Create(strQuery);
	TCHAR strFileName[1024] = {0};
	::GetModuleFileName(NULL, strFileName, sizeof(strFileName)/sizeof(TCHAR));
    hwr->Method = "GET";
	hwr->UserAgent = gcnew String(strFileName);
	hwr->ContentType = "application/json; charset=utf-8";
	wr = hwr->GetResponse();
	StreamReader ^stream = gcnew StreamReader(wr->GetResponseStream());
	str = stream->ReadToEnd();
	stream->Close();
	wr->Close();
	return str;
}

String^ CCometImpl::Enter(System::Web::UI::Page ^page, String ^strUserId, array<long> ^Groups, String^ strOnMessage, int nDelayTime)
{
	String ^strHttp;
	String ^strDomain;
	int nPort;
	String ^strPath = "";
	String ^str;
	String ^strQuery;
	WebResponse ^wr;
	CRet ^ret;
	if(nDelayTime < 0)
		nDelayTime = 350;
	String ^strHost = page->Request->UserHostAddress;
	if(strHost == "127.0.0.1" || strHost == IPAddress::IPv6Loopback->ToString() || strHost->ToLower() == "localhost")
		strHost = Dns::GetHostName();
	array<WCHAR> ^sep = {'.'};
	array<String^> ^dots = strHost->Split(sep);
	if(dots->Length != 4)
		strHost = GetIpv4(strHost);
	{
		bool bSSL;
		CAutoLock	AutoLock(&g_cs.m_sec);
		str = m_strUrlToComet->ToLower();
		int nSep = str->IndexOf("://");
		if(nSep < 4 || nSep > 5 || str->IndexOf("http") != 0)
			throw gcnew Exception("Url address to a remote COMET server not available!");
		bSSL = (str->IndexOf("https://") == 0);
		nSep = str->IndexOf('/', 7 + (bSSL ? 1 : 0));
		if(nSep != -1)
			strPath =  m_strUrlToComet->Substring(nSep + 1);
		str = str->Substring(0, nSep);
		str = str->Substring(7 + (bSSL ? 1 : 0));
		nSep = str->IndexOf(':');
		if(nSep == -1)
		{
			nPort = bSSL ? 443 : 80;
			strDomain = str;
		}
		else
		{
			strDomain = str->Substring(0, nSep);
			nPort = int::Parse(str->Substring(nSep + 1));
		}
		strHttp = bSSL ? "https:" : "http:";
		strQuery = String::Format(	"{0}?UJS_DATA={{\"method\":\"enter\",\"parameters\":{{\"groups\":{1},\"userId\":\"{2}\"}},\"id\":{3},\"version\":{4}}}&BrowserIpAddr={5}",
			m_strUrlToComet, GetJsonArray(Groups), strUserId, "0", Version, strHost);
	}

	ScriptManager ^sm = ScriptManager::GetCurrent(page);
	if(sm == nullptr)
	{
		if(m_strUJson->Length > 0)
			str = String::Format("{0}//{1}:{2}/{3}/ujsonxml.js", strHttp, strDomain, nPort.ToString(), m_strUJson);
		else
			str = String::Format("{0}//{1}:{2}/ujsonxml.js", strHttp, strDomain, nPort.ToString());
		if(m_bIE8)
			str += "?IE8Mode=1";
		ScriptManager::RegisterClientScriptInclude(page, page->GetType(), "ucomet_include_block", str);
	}
/*	else
	{
		String ^strInclude = String::Format("document.write(\"<script type='text/javascript' src='{0}'>\");", str);
		strInclude += "document.write('</' + 'script>\n');";
		ScriptManager::RegisterClientScriptBlock(page, page->GetType(), "ucomet_include_block", strInclude, true);
	}*/

	HttpWebRequest ^hwr = (HttpWebRequest^)WebRequest::Create(strQuery);
    hwr->Method = "GET";
	hwr->UserAgent = page->Request->UserAgent;
	hwr->ContentType = "application/json; charset=utf-8";
	wr = hwr->GetResponse();
	StreamReader ^stream = gcnew StreamReader(wr->GetResponseStream());
	str = stream->ReadToEnd();
	stream->Close();
	wr->Close();
	String ^strMe = String::Copy(str);
	JavaScriptSerializer ^jss = gcnew JavaScriptSerializer();
	ret = jss->Deserialize<CRet^>(str);
	if(ret->ret->ToString()->Length != 38)
		throw gcnew Exception(str);
	String ^strTemp = page->Request->UserAgent->ToLower();
	strQuery = String::Format("UChat.subscribe({0})", nDelayTime);
	str = String::Format("<script type='text/javascript'>UHTTP.defaultConfig.chatId='{0}';UChat.onMessage={1};{2};</script>\n", ret->ret->ToString(), strOnMessage, strQuery);
	ScriptManager::RegisterStartupScript(page, page->GetType(), "ucomet_enter_startup", str, false);
	return strMe;
}

String^ CCometImpl::Enter(System::Web::UI::Page ^page, String ^strUserId, array<long> ^Groups, String^ strOnMessage)
{
	return Enter(page, strUserId, Groups, strOnMessage, 350);
}

IComet^ AspNetPush::CreateInstance()
{
	return gcnew CCometImpl();
}

}
}
