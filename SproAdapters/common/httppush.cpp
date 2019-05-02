#include "StdAfx.h"
#include "httppush.h"
using namespace System::Web::Script;
using namespace System::Web::Script::Serialization;

namespace SocketProAdapter
{
namespace ServerSide
{
	array<long>^ CHttpPushPeer::ConvertObjectIntoIntArray(Object^ groups)
	{
		List<long> lst;
		array<Object^>^ objGroups = dynamic_cast<array<Object^>^>(groups);
		if(objGroups)
		{
			for each (Object^ obj in objGroups)
			{
				lst.Add(int::Parse(obj->ToString()));
			}
		}
		else
		{
			int n = 0;
			int nGroup = 1;
			int nGroups = int::Parse(groups->ToString());
			while(n<32)
			{
				if((nGroups & nGroup) == nGroup)
					lst.Add(nGroup);
				n++;
				nGroup <<= 1;
			}
		}
		return lst.ToArray();
	}

	CRet^ CHttpPushPeer::HandleJSCall(String ^strMethod, String^ strVersion, String ^strId, Dictionary<String^, Object^> ^mapParams, String ^strRequest)
	{
		String ^strAgent = "";
		CRet ^ret = gcnew CRet();
		ret->method = strMethod;
		ret->ret = "UnknownRequest";
		bool b = false;
		bool bCrossSite = Params->ContainsKey("UJS_CB");
		if (Headers->ContainsKey("User-Agent"))
			strAgent = Headers["User-Agent"]->ToLower();
		bool bSelfMessage = ((bCrossSite && (strAgent->IndexOf("firefox") != -1)) || (strAgent->IndexOf("msie ") != -1 && m_dAgentVersion < 7.5));
		if(strMethod == "enter")
		{
            if(Params->ContainsKey("BrowserIpAddr"))
                m_strBrowserIpAddress = Params["BrowserIpAddr"];
			if(IsOk(mapParams))
			{
				String ^strUserId;
				array<long> ^GroupIds;
				if(mapParams->ContainsKey("userId"))
					strUserId = mapParams["userId"]->ToString();
				if(mapParams->ContainsKey("groups"))
					GroupIds = ConvertObjectIntoIntArray(mapParams["groups"]);
				ret->ret = HttpPush->Enter(GroupIds, strUserId, m_nLeaseTime, m_strBrowserIpAddress);
				b = (ret->ret->ToString()->Length == 38);
				if(b)
					UserID = strUserId;
			}
			else
			{
				ret->ret = "Chatting not allowed";
				b = false;
			}
		}
		else if(strMethod == "exit")
		{
			String ^strChatId = "";
			if(mapParams->ContainsKey("chatId"))
				strChatId = mapParams["chatId"]->ToString();
			if (bSelfMessage) //FireFox, Flock, and MSIE!
				SendSelfMessage(strChatId, nullptr);
			b = HttpPush->Exit(strChatId);
			ret->ret = b;
			b = false;
		}
		else if(strMethod == "speak")
		{
			String ^strMsg; //????
			if(mapParams->ContainsKey("message"))
				strMsg = mapParams["message"]->ToString();
			String ^strChatId = "";
			if(mapParams->ContainsKey("chatId"))
				strChatId = mapParams["chatId"]->ToString();
			array<long> ^GroupIds;
			if(mapParams->ContainsKey("groups"))
				GroupIds = ConvertObjectIntoIntArray(mapParams["groups"]);
			if (bSelfMessage) //FireFox, Flock, and MSIE!
				SendSelfMessage(strChatId, nullptr);
			b = HttpPush->Speak(strChatId, strMsg, GroupIds);
			ret->ret = b;
		}
		else if(strMethod == "sendUserMessage")
		{
			String ^strMsg; //????
			if(mapParams->ContainsKey("message"))
				strMsg = mapParams["message"]->ToString();
			String ^strUID; //????
			if(mapParams->ContainsKey("userId"))
				strUID = mapParams["userId"]->ToString();
			String ^strChatId = "";
			if(mapParams->ContainsKey("chatId"))
				strChatId = mapParams["chatId"]->ToString();
			if (bSelfMessage) //FireFox, Flock, and MSIE!
				SendSelfMessage(strChatId, nullptr);
			b = HttpPush->SendUserMessage(strChatId, strUID, strMsg);
			ret->ret = b;
		}
		else if(strMethod == "listen")
		{
			String ^strChatId = "";
			if(mapParams->ContainsKey("chatId"))
				strChatId = mapParams["chatId"]->ToString();
			b = HttpPush->HTTPSubscribe(strChatId, m_nListeningTimeout, m_strJSCB);
			ret->ret = b;
		}
		else
		{
			ATLASSERT(FALSE);
		}
		if(!b)
			SetResponseHeader("Connection", "close");
		return ret;
	}

	String^ CHttpPushPeer::GetBrowserVersion(String ^str)
	{
		if(str != nullptr && str->Length >=3)
		{
			int nNum;
			bool bDot = false;
			array<wchar_t> ^sep = {'.', ' '};
			array<String^> ^lst = str->Split(sep);
			str = "";
			for each(String ^strNum in lst)
			{
				try
				{
					nNum = int::Parse(strNum);
					str += strNum;
					if(!bDot)
					{
						str += ".";
						bDot = true;
					}
				}
				catch(...)
				{
					break;
				}
			}
			return str;
		}
		return "0.0";
	}

	bool CHttpPushPeer::HasNativeJson(String^ strUserAgent)
	{
		int nIndex;
		m_dAgentVersion = 0.0;
		if(strUserAgent == nullptr) return false;
		strUserAgent = strUserAgent->ToLower();
		do
		{
			nIndex = strUserAgent->IndexOf("msie ");
			if(nIndex != -1)
			{
				String ^str = strUserAgent->Substring(nIndex + 5, 3);
				str = GetBrowserVersion(str);
				m_dAgentVersion = double::Parse(str);
				if(m_dAgentVersion > 7.5 && Params->ContainsKey("IE8Mode"))
					return true;
				return false;
			}
			nIndex = strUserAgent->IndexOf("firefox/");
			if(nIndex != -1)
			{
				String ^str = strUserAgent->Substring(nIndex + 8);
				str = GetBrowserVersion(str);
				m_dAgentVersion = double::Parse(str);
				if(m_dAgentVersion < 3.51)
					return false;
				return true;
			}
			nIndex = strUserAgent->IndexOf("chrome/");
			if(nIndex != -1)
			{
				String ^str = strUserAgent->Substring(nIndex + 7);
				str = GetBrowserVersion(str);
				m_dAgentVersion = double::Parse(str);
				if(m_dAgentVersion < 3.019)
					return false;
				return true;
			}
			nIndex = strUserAgent->IndexOf("safari/");
			if(nIndex != -1)
			{
				String ^str = strUserAgent->Substring(nIndex + 7);
				str = GetBrowserVersion(str);
				m_dAgentVersion = double::Parse(str);
				if(m_dAgentVersion < 4.01)
					return false;
				return true;
			}
		}while(false);
		return false;
		
	}

	bool CHttpPushPeer::DoCallback(String ^strChatId, String ^strMsg)
	{
		return SendSelfMessage(strChatId, strMsg);
	}

	bool CHttpPushPeer::SendSelfMessage(String ^strChatId, String ^strMsg)
    {
        String ^strUserID = nullptr;
        String ^strIpAddr = nullptr;
		array<long> ^GroupIds;
        int nLeaseTimeOut = 0;
        int nMessages = 0;
        int nGroupIds = 0;
        int nTimeout = 0;
		if(!CSocketProServer::HttpPush::GetHTTPChatContext(strChatId, strUserID, strIpAddr, nLeaseTimeOut, GroupIds, nTimeout, nMessages))
            return false;
		//for FireFox and Flock
		if(strMsg == nullptr || strMsg->Length == 0)
			strMsg = "__UCOMET_MESSAGE__";
        return HttpPush->SendUserMessage(strChatId, strUserID, strMsg);
    }

	void CHttpPushPeer::OnAfterProcessingHttpPushRequest(String ^strMethod, Object ^result)
	{

	}

	bool CHttpPushPeer::IsFile(String ^%fileExt)
	{
		String ^strPath = PathName->ToLower();
		CAutoLock	AutoLock(&g_cs.m_sec);
		for each(String ^strExt in m_lstFileExtension)
		{
			int nIndex  = strPath->LastIndexOf("." + strExt);
			if(nIndex > 0 && (nIndex + strExt->Length + 1) == strPath->Length)
			{
				fileExt = strExt;
				return true;
			}
		}
		return false;
	}

	bool CHttpPushPeer::OnDownloading(String ^strFile)
	{
		return true;
	}

	void CHttpPushPeer::HandleRequest(short sRequestID)
	{
		int res;
		String ^fileExt = nullptr;
		String ^strRequest = nullptr;
		String ^str;
		switch(sRequestID)
		{
		case idGet:
			if(PathName == "/" || IsFile(fileExt))
			{
				int nIndexUJS = -1;
				if(fileExt == "html" || fileExt == "htm" || fileExt == "mhtml" || fileExt == "mht")
					SetResponseHeader("Content-Type", "text/html");
				str = PathName->ToLower();
				fileExt = "." + fileExt;
				if(fileExt == ".js")
				{
					nIndexUJS = str->IndexOf("ujsonxml.js");
					SetResponseHeader("Connection", "close");
					SetResponseHeader("Content-Type", "text/javascript");
				}
				if(nIndexUJS != -1)
				{
					System::Text::Encoding ^encoding = gcnew System::Text::UTF8Encoding();
					CScopeUQueue	tempUQueue;
					{
						bool bHasJson = false;
						if (Headers->ContainsKey("User-Agent") && HasNativeJson(Headers["User-Agent"]))
							bHasJson = true;
						CAutoLock	AutoLock(&g_cs.m_sec);
						if(m_su->UQueue->GetSize() == 0)
						{
							String ^strFile = PathName->Trim('\\', '/');
							strFile = strFile->Replace('/', '\\');
							try
							{
								array<BYTE> ^buffer = gcnew array<BYTE>(40960);
								FileStream ^fs = gcnew FileStream(strFile, FileMode::Open, FileAccess::Read);
								int nRead = fs->Read(buffer, 0, 40960);
								buffer[nRead] = 0;
								char *str = "if(!window.SocketProAdapter)";
								pin_ptr<BYTE> pByte = &buffer[0];
								char *strStart = reinterpret_cast<char*>( pByte );
								char *strPos = strstr(strStart, str);
								unsigned long nPos = (unsigned long)(strPos - strStart);
								m_json->UQueue->Push(buffer, nPos, 0);
								m_su->UQueue->Push(buffer, nRead);
								m_su->UQueue->Discard((int)nPos);
								m_su->UQueue->SetHeadPosition();
								fs->Close();
							}
							catch(Exception ^myError)
							{
								CRet ^retObject = gcnew CRet();
								retObject->method = "download_ujsonxml";
								OnProcessingHttpPushRequestException(retObject->method, myError);
								array<WCHAR> ^sep = {'\r', '\n', ' '};
								retObject->ret =myError->Message->Trim(sep);
								JavaScriptSerializer ^jss = gcnew JavaScriptSerializer();
								str = jss->Serialize(retObject);
								m_UQueue->SetSize(0);
								m_UQueue->Push(encoding->GetBytes(str));
								int res = SendResult(sRequestID, m_UQueue);
								return;
							}
						}
						if(bHasJson)
						{
							char *json = "window.UJSON=JSON;";
							tempUQueue.UQueue->GetInternalUQueue()->Push(json);
						}
						else
						{
							tempUQueue.UQueue->Push(m_json->UQueue->GetBuffer(), m_json->UQueue->GetSize());
						}
						tempUQueue.UQueue->Push(m_su->UQueue->GetBuffer(), m_su->UQueue->GetSize());
					}
					str = "\nUHTTP.defaultConfig={pathname:'";
					str += PushPath;
					String ^strHost = "";
					if(Headers->ContainsKey("Host"))
						strHost = Headers["Host"];
					array<WCHAR> ^sep = {':'};
					array<String^> ^lstHost = strHost->Split(sep);
					String ^strPort = nullptr;
					if(lstHost->Length == 2)
						strPort = lstHost[1];
					if(CSocketProServer::Config::DefaultEncryptionMethod != USOCKETLib::tagEncryptionMethod::NoEncryption && CSocketProServer::Config::DefaultEncryptionMethod != USOCKETLib::tagEncryptionMethod::BlowFish)
					{
						str += "',protocol:'https',port:";
						if(strPort == nullptr)
							strPort = "443";
					}
					else
					{
						str += "',protocol:'http',port:";
						if(strPort == nullptr)
							strPort = "80";
					}
					str += strPort;
					str += ",hostname:'";
					str += lstHost[0];
					str += "'};";
					tempUQueue.UQueue->Push(encoding->GetBytes(str));
					SendResult(sRequestID, tempUQueue.UQueue);
				}
				else
				{
					String ^strFile;
					if(PathName == "/")
						strFile = Default;
					else
					{
						strFile = PathName->Trim('/', '\\');
						strFile = strFile->Replace('/', '\\');
					}
					try
					{
						if((strFile->IndexOf("_sp.xml") == -1) && OnDownloading(strFile))
						{
							FileStream ^fs = gcnew FileStream(strFile, FileMode::Open, FileAccess::Read, FileShare::Read);
							array<BYTE> ^buffer = gcnew array<BYTE>(10240);
							__int64 len = fs->Length;
							SetResponseHeader("Content-Length", len.ToString());
							int nRead = fs->Read(buffer, 0, 10240);
							while(nRead > 0)
							{
								SendReturnData(sRequestID, buffer, nRead);
								nRead = fs->Read(buffer, 0, 10240);
							}
							fs->Close();
						}
						else
						{
							SetResponseHeader("Connection", "close");
							SetResponseCode(403); //403 Forbidden 
							SendResult(sRequestID);
						}
					}
					catch(...)
					{
						SetResponseHeader("Connection", "close");
						SetResponseCode(404); //404 Not Found
                        SendResult(sRequestID);
					}
				}
				return;
			}
		case idPost:
			if(String::Compare(PathName, PushPath, false) == 0)
			{
				CRet ^retObject;
				Object ^jsObject;
				System::Text::Encoding ^encoding = gcnew System::Text::UTF8Encoding();
				String ^strMethod = "UnknownRequest";
				bool bBad = false;
				m_strJSCB = "";
				String ^strAgent = "";
				if(Headers->ContainsKey("User-Agent"))
					strAgent = Headers["User-Agent"]->ToLower();
				bool bJSON = HasNativeJson(strAgent);
				if(strAgent->IndexOf("opera") != -1 || (strAgent->IndexOf("msie ") != -1 && m_dAgentVersion < 7.5))
					SetResponseHeader("Connection", "close");
				JavaScriptSerializer ^jss = gcnew JavaScriptSerializer();
				try
				{
					if(sRequestID == idGet && Params->ContainsKey("UJS_DATA"))
					{
						strRequest = Params["UJS_DATA"];
						if(Params->ContainsKey("UJS_CB")) //cross-site callback
							m_strJSCB = Params["UJS_CB"];
					}
					else
					{
						array<BYTE>^ bytes;
						m_UQueue->Pop(bytes);
						strRequest = encoding->GetString(bytes);
					}
					jsObject = jss->DeserializeObject(strRequest);
					Dictionary<String^, Object^> ^mapParameters = (Dictionary<String^, Object^>^)jsObject;
					if(mapParameters->ContainsKey("method"))
						strMethod = mapParameters["method"]->ToString();
					String ^strId = "";
					if(mapParameters->ContainsKey("id"))
						strId = mapParameters["id"]->ToString();
					String ^strVersion = "";
					if(mapParameters->ContainsKey("version"))
						strVersion = mapParameters["version"]->ToString();
					Dictionary<String^, Object^> ^map = gcnew Dictionary<String^, Object^>();
					if(mapParameters->ContainsKey(PARAMS))
						map = (Dictionary<String^, Object^> ^)(mapParameters[PARAMS]);
					OnBeforeProcessingHttpPushRequest(strMethod, strVersion, strId, map);
					SetResponseHeader("Content-Type", "application/json");
					retObject = HandleJSCall(strMethod, strVersion, strId, map, strRequest);
					OnAfterProcessingHttpPushRequest(strMethod, retObject->ret);
				}
				catch(Exception ^myErr)
				{
					SetResponseHeader("Connection", "close");
					bBad = true;
					retObject = gcnew CRet();
					retObject->method = strMethod;
					OnProcessingHttpPushRequestException(strMethod, myErr);
					retObject->ret = myErr->Message;
				}
				String ^strRet = retObject->ret->ToString();
				if(bBad || strMethod != "listen" || strRet == "False" || strRet->Length == 0)
				{
					str = jss->Serialize(retObject);
					m_UQueue->SetSize(0);
                    if (m_strJSCB->Length > 0)
                        m_UQueue->Push(encoding->GetBytes(m_strJSCB));
					m_UQueue->Push(encoding->GetBytes(str));
					if (m_strJSCB->Length > 0)
						m_UQueue->Push(encoding->GetBytes(");"));
					res = SendResult(sRequestID, m_UQueue);
				}
			}
			else
			{
				Object ^jsObject;
				String ^strMethod = nullptr;
				array<BYTE>^ bytes;
				m_strJSCB = "";
				bool bOk = false;
				CRet ^retObject = gcnew CRet();
				Dictionary<String^, Object^> ^map = nullptr;
				System::Text::Encoding ^encoding = gcnew System::Text::UTF8Encoding();
				JavaScriptSerializer ^jss = gcnew JavaScriptSerializer();
				String ^strAgent = "";
				if(Headers->ContainsKey("User-Agent"))
					strAgent = Headers["User-Agent"]->ToLower();
				if(sRequestID == idGet && Params->ContainsKey("UJS_DATA"))
				{
					strRequest = Params["UJS_DATA"];
					if(Params->ContainsKey("UJS_CB")) //cross-site callback
						m_strJSCB = Params["UJS_CB"];
				}
				else
				{
					m_UQueue->Pop(bytes);
					try
					{
						strRequest = encoding->GetString(bytes);
					}
					catch(...)
					{
						m_UQueue->Push(bytes);
						return;
					}
				}
				SetResponseHeader("Content-Type", "application/json");
				bool bSelfMessage = ((m_strJSCB->Length > 0 && (strAgent->IndexOf("firefox") != -1)) || (strAgent->IndexOf("msie ") != -1 && m_dAgentVersion < 7.5));
				String ^strId = nullptr;
				String ^strVersion = nullptr;
				try
				{
					jsObject = jss->DeserializeObject(strRequest);
					Dictionary<String^, Object^> ^mapParameters = (Dictionary<String^, Object^>^)jsObject;
					if(mapParameters->ContainsKey("method"))
					{
						strMethod = mapParameters["method"]->ToString();
						retObject->method = strMethod;
					}
					if(mapParameters->ContainsKey("id"))
						strId = mapParameters["id"]->ToString();
					if(mapParameters->ContainsKey("version"))
						strVersion = mapParameters["version"]->ToString();
					map = gcnew Dictionary<String^, Object^>();
					if(mapParameters->ContainsKey(PARAMS))
					{
						map = (Dictionary<String^, Object^> ^)(mapParameters[PARAMS]);
					}
					if(strMethod == nullptr || strId == nullptr || strVersion == nullptr)
					{
						m_UQueue->Push(bytes);
						return;
					}
				}
				catch(...)
				{
					m_UQueue->Push(bytes);
					return;
				}
				try
				{
					if(bSelfMessage && map->ContainsKey("chatId"))
					{
						String ^chatId = (String^)(map["chatId"]);
						if(chatId->Length == 38)
							SendSelfMessage(chatId, nullptr);
					}
					retObject->ret = OnProcessingRequest(strMethod, strVersion, strId, map);
					bOk = true;
				}
				catch(Exception ^myErr)
				{
					SetResponseHeader("Connection", "close");
					retObject->ret = myErr->Message;
				}
				if(bOk)
				{
					str = jss->Serialize(retObject);
					m_UQueue->SetSize(0);
					if (m_strJSCB->Length > 0)
						m_UQueue->Push(encoding->GetBytes(m_strJSCB));
					m_UQueue->Push(encoding->GetBytes(str));
					if (m_strJSCB->Length > 0)
						m_UQueue->Push(encoding->GetBytes(");"));
					res = SendResult(sRequestID, m_UQueue);
				}
			}
			break;
		default:
			break;
		}
	}

	String^ CHttpPushPeer::OnProcessingRequest(String ^strMethod, String^ strVersion, String ^strId, Dictionary<String^, Object^> ^mapParams)
	{
		return "";
	}

	void CHttpPushPeer::OnSwitchFrom(int ServiceID)
	{
		int nPort;
		CHttpPeerBase::OnSwitchFrom(ServiceID);
		m_strPushPath = "/UCHAT";
		m_nLeaseTime = 120000;
		m_nListeningTimeout = 60000;
		m_strJSCB = "";
		m_strBrowserIpAddress = GetPeerName(nPort);
	}

	void CHttpPushPeer::OnProcessingHttpPushRequestException(String ^strMethod, Exception ^myError)
	{
		
	}

	bool CHttpPushPeer::IsOk(Dictionary<String^, Object^> ^parameters)
	{
		return true;
	}

	void CHttpPushPeer::OnBeforeProcessingHttpPushRequest(String ^strMethod, String^ strVersion, String ^strId, Dictionary<String^, Object^> ^mapParams)
	{
		
	}

	void CHttpPushPeer::SetTimes()
	{
		String ^str = "";
		if(Headers->ContainsKey("User-Agent"))
			str = Headers["User-Agent"];
		str = str->ToLower();
		if(str->IndexOf("opera") != -1)
			m_nListeningTimeout = 20000; //20 seconds
		else if(str->IndexOf("safari") != -1)
			m_nListeningTimeout = 30000; //20 seconds
	}

	void CHttpPushPeer::OnFastRequestArrive(short sRequestID, int nLen)
	{
		CHttpPeerBase::OnFastRequestArrive(sRequestID, nLen);
		if(sRequestID == idHeader)
			SetTimes();
		HandleRequest(sRequestID);
	}

	int CHttpPushPeer::OnSlowRequestArrive(short sRequestID, int nLen)
	{
		if(sRequestID == idHeader)
			SetTimes();
		HandleRequest(sRequestID);
		return 0;
	}
}
}