
#pragma once


#include "../../../include/async_odbc.h"
#include "njhandlerroot.h"

namespace NJA {
	class NJOdbc : public NJHandlerRoot {
	public:
		NJOdbc(SPA::ClientSide::COdbc *odbc);
		NJOdbc(const NJOdbc &h) = delete;
		~NJOdbc();

	public:
		NJOdbc& operator=(const NJOdbc &h) = delete;
		static void Init(Local<Object> exports);
		static Local<Object> New(Isolate* isolate, SPA::ClientSide::COdbc *ash, bool setCb);

	private:
		void Release();
		bool IsValid(Isolate* isolate);

		static const SPA::INT64 SECRECT_NUM = 0x7fa2b3ffe3a5;
		static void New(const FunctionCallbackInfo<Value>& args);

	private:
		static Persistent<Function> constructor;
		SPA::ClientSide::COdbc *m_odbc;
	};
}
