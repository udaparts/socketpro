
#pragma once
namespace NJA {
	class NJFile : public node::ObjectWrap, public CTable {
	public:
		NJFile(CStreamingFile *file);
		NJFile(const NJFile &h) = delete;
		~NJFile();

		struct FileCb {
			Persistent<Function> Download;
			Persistent<Function> Trans;
		};

	public:
		NJFile& operator=(const NJFile &h) = delete;
		static void Init(Local<Object> exports);
		static Local<Object> New(Isolate* isolate, CStreamingFile *ash, bool setCb);

	private:
		void Release();
		void SetCb();
		bool IsValid(Isolate* isolate);

		static const SPA::INT64 SECRECT_NUM = 0x7fa114ff2345;
		static void file_cb(uv_async_t* handle);
		static void New(const FunctionCallbackInfo<Value>& args);

		static void getSvsId(const FunctionCallbackInfo<Value>& args);
		static void AbortBatching(const FunctionCallbackInfo<Value>& args);
		static void AbortDequeuedMessage(const FunctionCallbackInfo<Value>& args);
		static void CleanCallbacks(const FunctionCallbackInfo<Value>& args);
		static void CommitBatching(const FunctionCallbackInfo<Value>& args);
		static void getRequestsQueued(const FunctionCallbackInfo<Value>& args);
		static void IsBatching(const FunctionCallbackInfo<Value>& args);
		static void IsDequeuedMessageAborted(const FunctionCallbackInfo<Value>& args);
		static void IsDequeuedResult(const FunctionCallbackInfo<Value>& args);
		static void IsRouteeResult(const FunctionCallbackInfo<Value>& args);
		static void StartBatching(const FunctionCallbackInfo<Value>& args);
		static void SendRequest(const FunctionCallbackInfo<Value>& args);
		static void Dispose(const FunctionCallbackInfo<Value>& args);

		static void getFilesQueued(const FunctionCallbackInfo<Value>& args);
		static void getFileSize(const FunctionCallbackInfo<Value>& args);
		static void Upload(const FunctionCallbackInfo<Value>& args);
		static void Download(const FunctionCallbackInfo<Value>& args);

	private:
		static Persistent<Function> constructor;
		static SPA::CUCriticalSection m_cs;
		CStreamingFile *m_ash;
		std::deque<FileCb> m_deqFileCb; //protected by m_cs
		uv_async_t m_typeFile; //protected by m_cs
	};
}
