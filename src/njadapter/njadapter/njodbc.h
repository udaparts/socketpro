
#pragma once

#include "../../../include/odbc/uodbc.h"
#include "../../../include/udb_client.h"
#include "njhandlerroot.h"

namespace NJA {
	
	typedef SPA::ClientSide::CAsyncDBHandler<SPA::Odbc::sidOdbc> COdbc;

	class NJOdbc : public NJHandlerRoot {
	public:
		NJOdbc(COdbc *odbc);
		NJOdbc(const NJOdbc &h) = delete;
		~NJOdbc();

	public:
		NJOdbc& operator=(const NJOdbc &h) = delete;
		static void Init(Local<Object> exports);
		static Local<Object> New(Isolate* isolate, COdbc *ash, bool setCb);

	private:
		void Release();
		bool IsValid(Isolate* isolate);

		static const SPA::INT64 SECRECT_NUM = 0x7fa2b3ffe3a5;
		static void New(const FunctionCallbackInfo<Value>& args);

	private:
		static Persistent<Function> constructor;
		COdbc *m_db;
	};
}
