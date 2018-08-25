
#pragma once

#include "njhandlerroot.h"

namespace NJA {
	class NJFile : public NJHandlerRoot {
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

		static void getFilesQueued(const FunctionCallbackInfo<Value>& args);
		static void getFileSize(const FunctionCallbackInfo<Value>& args);
		static void Upload(const FunctionCallbackInfo<Value>& args);
		static void Download(const FunctionCallbackInfo<Value>& args);

	private:
		static Persistent<Function> constructor;
		CStreamingFile *m_file;
		std::deque<FileCb> m_deqFileCb; //protected by m_cs
		uv_async_t m_typeFile; //protected by m_cs
	};
}
