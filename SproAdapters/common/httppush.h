#ifndef ____CHECK_UHTTP_PUSH_H_____
#define ____CHECK_UHTTP_PUSH_H_____

#include "ClientPeer.h"
#include "jobmanagerinterface.h"

namespace SocketProAdapter
{
namespace ServerSide
{
	/// <summary>
	/// A base class for implementing HTTP server push and supporting HTTP request from a web browser through SocketProAdapter for JavaScript.
	/// The class represents a simplified HTTP service supporting two HTTP commands, GET and POST.
	/// It also supports downloading files which belong to one of pre-defined file extensions (see the property FileExtensions).
	/// </summary>
	public ref class CHttpPushPeer : public CHttpPeerBase
	{
	public:
		/// <summary>
		/// Send a message (strMsg) to a browser through this socket connection from a given chat id (strChatId).
		/// The method returns true if successful.
		/// </summary>
		bool DoCallback(String ^strChatId, String ^strMsg);

	protected:
		virtual void OnFastRequestArrive(short sRequestID, int nLen) override;
		virtual int OnSlowRequestArrive(short sRequestID, int nLen) override;
		virtual void OnSwitchFrom(int ServiceID) override;

		
		/// <summary>
		/// A virtual function which can be used for blocking chatting HTTP requests according to ip address.
		/// If the virtual function returns false, there is no joining for HTTP server push.
		/// For example, this is especially useful to cross-site HTTP chat requests for better security.
		/// </summary>
		virtual bool IsOk(Dictionary<String^, Object^> ^parameters);
		
		/// <summary>
		/// The virtual function is called if there is an exception (myError) during processing.
		/// </summary>
		virtual void OnProcessingHttpPushRequestException(String ^strMethod, Exception ^myError);
		
		/// <summary>
		/// The virtual function is called right before processing HTTP push request.
		/// </summary>
		virtual void OnBeforeProcessingHttpPushRequest(String ^strMethod, String^ strVersion, String ^strId, Dictionary<String^, Object^> ^mapParams);
		
		/// <summary>
		/// The virtual function is called right after processing HTTP push request.
		/// </summary>
		virtual void OnAfterProcessingHttpPushRequest(String ^strMethod, Object ^result);

		/// <summary>
		/// The virtual function is called where there is a HTTP request coming from a web browser through SocketProAdapter for JavaScript.
		/// The return value will be sent to a browser.
		/// </summary>
		virtual String^ OnProcessingRequest(String ^strMethod, String^ strVersion, String ^strId, Dictionary<String^, Object^> ^mapParams);
		
		/// <summary>
		/// The virtual function is called right before downloading a file (strFile).
		/// If the virtual function returns false, there is no downloading.
		/// You can allow or dis-allow file downloading by overriding this virtual function.
		/// </summary>
		virtual bool OnDownloading(String ^strFile);

	private:
		static String^ PARAMS = "params";
		static array<long>^ ConvertObjectIntoIntArray(Object^ groups);
		String^ GetBrowserVersion(String ^str);
		bool HasNativeJson(String^ strUserAgent);
		void HandleRequest(short sRequestID);
		bool SendSelfMessage(String ^strChatId, String ^strMsg);
		CRet^ HandleJSCall(String ^strMethod, String^ strVersion, String ^strId, Dictionary<String^, Object^> ^mapParams, String ^strRequest);
		void SetTimes();
		bool IsFile(String ^%fileExt);
		static CHttpPushPeer()
		{
			m_lstFileExtension->Add("js");
			m_lstFileExtension->Add("zip");
			m_lstFileExtension->Add("htm");
			m_lstFileExtension->Add("html");
			m_lstFileExtension->Add("xap");
			m_lstFileExtension->Add("xml");
			m_lstFileExtension->Add("txt");
			m_lstFileExtension->Add("jpg");
			m_lstFileExtension->Add("jpeg");
			m_lstFileExtension->Add("bmp");
			m_lstFileExtension->Add("dib");
			m_lstFileExtension->Add("gif");
			m_lstFileExtension->Add("tif");
			m_lstFileExtension->Add("tiff");
			m_lstFileExtension->Add("png");
			m_lstFileExtension->Add("doc");
			m_lstFileExtension->Add("csv");
			m_lstFileExtension->Add("rtf");
			m_lstFileExtension->Add("mht");
			m_lstFileExtension->Add("mhtml");
			m_lstFileExtension->Add("dif");
			m_lstFileExtension->Add("prn");
		}

	public:
		/// <summary>
		/// A case-sensitive string indicating a chat path or channel. It defaults to '/UCHAT'.
		/// </summary>
		property String^ PushPath
		{
			String^ get()
			{
				return m_strPushPath;
			}
			void set(String ^strPushPath)
			{
				m_strPushPath = "/";
				if(strPushPath != nullptr)
				{
					m_strPushPath = String::Copy(strPushPath);
					array<WCHAR> ^sep = {' ', '\\', '/'};
					m_strPushPath = m_strPushPath->Trim(sep);
					m_strPushPath = "/" + m_strPushPath;
				}
			}
		}

		/// <summary>
		/// A default file name with relative path.
		/// </summary>
		static property String^ Default
		{
			String^ get()
			{
				CAutoLock	AutoLock(&g_cs.m_sec);
				return m_strDefault;
			}

			void set(String^ str)
			{
				m_strDefault = "";
				if(str != nullptr)
				{
					str = str->Trim(' ', '/', '.', '\n', '\\');
					str = str->Replace("/", "\\");
					g_cs.Lock();
					m_strDefault = str;
					g_cs.Unlock();
				}
			}
		}
		
		/// <summary>
		/// The property indicates if the current request is a cross-site HTTP request originating (ujsonxml.js) from SocketProAdapter for JavaScript.
		/// You may override this property for your requirements.
		/// </summary>
		property virtual bool CrossSite
		{
			bool get()
			{
				return (m_strJSCB->Length > 0);
			}
		}
		
		/// <summary>
		/// Lease time in ms. It defaults to 120,000 (120 seconds) for most of major browsers.
		/// Don't change the default value unless you must do so.
		/// </summary>
		property int LeaseTime
		{
			int get()
			{
				return m_nLeaseTime;
			}
			void set(int nLeaseTime)
			{
				m_nLeaseTime = nLeaseTime;
			}
		}
		
		/// <summary>
		/// Listening timeout in ms. It defaults to 60,000 (60 seconds) for most of major browsers.
		/// Don't change the default value unless you must do so.
		/// </summary>
		property int ListeningTimeout
		{
			int get()
			{
				return m_nListeningTimeout;
			}
			void set(int nListeningTimeout)
			{
				m_nListeningTimeout = nListeningTimeout;
			}
		}
		
		/// <summary>
		/// Browser IP address which a unique chat id is associated with.
		/// SocketPro HTTP push service will associate a chat id with an IP address when a browser calls the JavaScript method enter.
		/// If the IP address is not recognized, the push service will return false for all of the other calls.
		/// This design makes security better especially for cross-site HTTP push.
		/// </summary>
		property String^ BrowserIpAddress
		{
			String^ get()
			{
				return m_strBrowserIpAddress;
			}
		}
		
		/// <summary>
		/// An array of file extensions supported for downloading by default.
		/// To add or remove file extensions, use the methods AddFileExtension and RemoveFileExtension, respectively.
		/// </summary>
		static property array<String^>^ FileExtensions
		{
			array<String^>^ get()
			{
				CAutoLock	AutoLock(&g_cs.m_sec);
				return m_lstFileExtension->ToArray();
			}
		}
		
		/// <summary>
		/// add a file extension.
		/// </summary>
		static void AddFileExtension(String ^strFileExtension)
		{
			if(strFileExtension == nullptr)
				strFileExtension = "";
			strFileExtension = strFileExtension->Trim(' ', '.', '/', '\\');
			strFileExtension = strFileExtension->ToLower();
			CAutoLock	AutoLock(&g_cs.m_sec);
			if(m_lstFileExtension->IndexOf(strFileExtension) == -1)
				m_lstFileExtension->Add(strFileExtension);
		}

		/// <summary>
		/// remove a file extension.
		/// </summary>
		static void RemoveFileExtension(String ^strFileExtension)
		{
			if(strFileExtension == nullptr)
				strFileExtension = "";
			strFileExtension = strFileExtension->Trim(' ', '.', '/', '\\');
			if(strFileExtension->Length == 0)
				return;
			strFileExtension = strFileExtension->ToLower();
			CAutoLock	AutoLock(&g_cs.m_sec);
			if(m_lstFileExtension->IndexOf(strFileExtension) != -1)
				m_lstFileExtension->Remove(strFileExtension);
		}

	private:
		static List<String^>^	m_lstFileExtension = gcnew List<String^>();
		int m_nLeaseTime;
		int m_nListeningTimeout;
		String ^m_strPushPath;
		String ^m_strJSCB;
		String ^m_strBrowserIpAddress;
		static CScopeUQueue ^m_su = gcnew CScopeUQueue();
		static CScopeUQueue ^m_json = gcnew CScopeUQueue();
		static String ^m_strDefault = "";
		double m_dAgentVersion;
	};
}
}

#endif