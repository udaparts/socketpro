
#pragma once


#include "../../../include/async_sqlite.h"
#include "njhandlerroot.h"

namespace NJA {
	class NJSqlite : public NJHandlerRoot {
	public:
		NJSqlite(SPA::ClientSide::CSqlite *sqlite);
		NJSqlite(const NJSqlite &h) = delete;
		~NJSqlite();

	public:
		NJSqlite& operator=(const NJSqlite &h) = delete;
		static void Init(Local<Object> exports);
		static Local<Object> New(Isolate* isolate, SPA::ClientSide::CSqlite *ash, bool setCb);

	private:
		void Release();
		bool IsValid(Isolate* isolate);

		static const SPA::INT64 SECRECT_NUM = 0x7fa0b0ffe0a5;
		static void New(const FunctionCallbackInfo<Value>& args);
		static void BeginTrans(const FunctionCallbackInfo<Value>& args);
		static void Close(const FunctionCallbackInfo<Value>& args);
		static void EndTrans(const FunctionCallbackInfo<Value>& args);
		static void Open(const FunctionCallbackInfo<Value>& args);
		static void Prepare(const FunctionCallbackInfo<Value>& args);
		static void Execute(const FunctionCallbackInfo<Value>& args);
		static void ExecuteBatch(const FunctionCallbackInfo<Value>& args);
		static void IsOpened(const FunctionCallbackInfo<Value>& args);

	private:
		static Persistent<Function> constructor;
		SPA::ClientSide::CSqlite *m_db;
	};
}
