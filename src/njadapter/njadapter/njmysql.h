
#pragma once

#include "../../../include/async_mysql.h"
#include "njhandlerroot.h"

namespace NJA {
	class NJMysql : public NJHandlerRoot {
	public:
		NJMysql(SPA::ClientSide::CMysql *mysql);
		NJMysql(const NJMysql &h) = delete;
		~NJMysql();

	public:
		NJMysql& operator=(const NJMysql &h) = delete;
		static void Init(Local<Object> exports);
		static Local<Object> New(Isolate* isolate, SPA::ClientSide::CMysql *ash, bool setCb);

	private:
		void Release();
		bool IsValid(Isolate* isolate);

		static const SPA::INT64 SECRECT_NUM = 0x7fa0b0ffe0a5;
		static void New(const FunctionCallbackInfo<Value>& args);

	private:
		static Persistent<Function> constructor;
		SPA::ClientSide::CMysql *m_db;
	};
}
