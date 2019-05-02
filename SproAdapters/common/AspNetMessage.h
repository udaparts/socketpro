#pragma once

#include "jobmanagerinterface.h"

namespace SocketProAdapter
{
namespace ClientSide
{
	ref class CAsyncChatHandler : public CAsyncServiceHandler
	{
	public:
		CAsyncChatHandler() : CAsyncServiceHandler(sidChat)
		{
		}

		CAsyncChatHandler(CClientSocket ^cs) : CAsyncServiceHandler(sidChat, cs)
		{
		}

		CAsyncChatHandler(CClientSocket ^cs, IAsyncResultsHandler ^pAsyncResultsHandler) : CAsyncServiceHandler(sidChat, cs, pAsyncResultsHandler)
		{
		}	
	};

/// <summary>
/// An interface for integrating asp.net, ajax and silverlight with SocketPro HTTP push service.
/// </summary>
[CLSCompliantAttribute(true)]
public interface class IComet
{
	/// <summary>
	/// Join one or more chatting groups (Groups) with a given case-insensitive user id. 
	/// The parameter strSubscriberIpAddress indicates a subscriber ip address like '111.222.121.212'.
	/// The method returns a string containing a chat id if successful. Note that the method is thread-safe.
	/// </summary>
	String^ Enter(String ^strSubscriberIpAddress, String ^strUserId, array<long> ^Groups);
	
	/// <summary>
	/// Join one or more chatting groups (Groups) with a given case-insensitive user id. 
	/// The parameters page and strOnMessage indicate asp.net page and JavaScript function, respectively.
	/// A new JavaScript code snippet is created, and already registerred at startup block containing UHTTP default configuration code within asp.net page.
	/// On a browser side, it will kick off listening messages automatically after 350 ms.
	/// The method returns a string containing a chat id if successful. Note that the method is thread-safe.
	/// </summary>
	String^ Enter(System::Web::UI::Page ^page, String ^strUserId, array<long> ^Groups, String^ strOnMessage);

	/// <summary>
	/// Join one or more chatting groups (Groups) with a given case-insensitive user id. 
	/// The parameters page and strOnMessage indicate asp.net page and JavaScript function, respectively.
	/// A new JavaScript code snippet is created, and already registerred at startup block containing UHTTP default configuration code within asp.net page.
	/// On a browser side, it will kick off listening messages automatically in a given delay time (nDelayTime).
	/// The method returns a string containing a chat id if successful. Note that the method is thread-safe.
	/// </summary>
	String^ Enter(System::Web::UI::Page ^page, String ^strUserId, array<long> ^Groups, String^ strOnMessage, int nDelayTime);
	
	/// <summary>
	/// A version string like 1.1 by default.
	/// </summary>
	property String ^Version
	{
		String^ get();
		void set(String^ version);
	}
	
	/// <summary>
	/// A relative path to the file ujsonxml.js. It defaults to an empty string (web site root).
	/// </summary>
	property String ^UJson
	{
		String^ get();
		void set(String^ version);
	}

	/// <summary>
	/// A url address with case-sensitive chatting path or channel like http://www.somedomain.com:20901/UCHAT
	/// </summary>
	property String^ UrlToComet
	{
		String^ get();
		void set(String ^str);
	}

	/// <summary>
	/// A property for MS IE8 browser or later
	/// </summary>
	property bool IE8
	{
		bool get();
		void set(bool bIE);
	}
};

ref class CCometImpl : public IComet
{
public:
	CCometImpl();

private:
	String								^m_strUrlToComet;
	String								^m_strVersion;
	String								^m_strUJson;
	bool								m_bIE8;

public:
	virtual String^ Enter(String ^strSubscriberIpAddress, String ^strUserId, array<long> ^Groups);
	virtual String^ Enter(System::Web::UI::Page ^page, String ^strUserId, array<long> ^Groups, String^ strOnMessage);
	virtual String^ Enter(System::Web::UI::Page ^page, String ^strUserId, array<long> ^Groups, String^ strOnMessage, int nDelayTime);
	static String^ GetIpv4(String ^strIpAddr);
	static String^ GetJsonArray(array<long> ^myArray);

	property String ^Version
	{
		virtual String^ get()
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			return String::Copy(m_strVersion);
		}
		virtual void set(String^ version)
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			m_strVersion = "";
			if(version != nullptr && version->Length > 0)
				m_strVersion = String::Copy(version);
		}
	}

	property String ^UJson
	{
		virtual String^ get()
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			return m_strUJson;
		}

		virtual void set(String^ str)
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			m_strUJson = "";
			if(str != nullptr)
				m_strUJson = String::Copy(str);
			array<WCHAR> ^sep = {' ', '/', '\\', '?', '&'};
			m_strUJson = m_strUJson->Trim(sep);
		}
	}
	
	property String^ UrlToComet
	{
		virtual String^ get()
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			return String::Copy(m_strUrlToComet);
		}
		virtual void set(String ^str)
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			m_strUrlToComet = "";
			if(str != nullptr)
				m_strUrlToComet = String::Copy(str);
			array<WCHAR> ^sep = {' ', '/', '\\', '?', '&'};
			m_strUrlToComet = m_strUrlToComet->Trim(sep);
		}
	}

	property bool IE8
	{
		virtual bool get()
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			return m_bIE8;
		}
		virtual void set(bool bIE8)
		{
			CAutoLock	AutoLock(&g_cs.m_sec);
			m_bIE8 = bIE8;
		}
	}
};

/// <summary>
/// A sealed class simplifies the intergration between SocketProAdapter for JavaScript and ASP.NET/AJAX.
/// </summary>
public ref class AspNetPush sealed
{
private:
	static IComet ^m_comet = gcnew CCometImpl();

public:
	/// <summary>
	/// Create an extra COMET instance in case you want to assign a client to one of many available SocketPro HTTP push servers. 
	/// </summary>
	static IComet^ CreateInstance();
	
	/// <summary>
	/// An interface to a default COMET instance. In case you need extra one, use the method CreateInstance.
	/// </summary>
	static property IComet^ Comet
	{
		IComet^ get()
		{
			return m_comet;
		}
	}
};

}
}
