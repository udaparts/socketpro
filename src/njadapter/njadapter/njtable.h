#pragma once
namespace NJA {

	class NJTable : public node::ObjectWrap {
	public:
		NJTable(SPA::CTable *tbl);
		NJTable(const NJTable &tbl) = delete;
		~NJTable();

	public:
		inline SPA::CTable* get() const {
			return m_table;
		}
		
		static void Init(Local<Object> exports);
		static Local<Object> New(Isolate* isolate, SPA::CTable *tbl, bool setCb);

	private:
		NJTable& operator=(const NJTable &tbl) = delete;
		bool IsValid(Isolate* isolate);
		void Release();
		
		static const SPA::INT64 SECRECT_NUM = 0x4f25b02ce415;
		static void New(const FunctionCallbackInfo<Value>& args);

	private:
		static Persistent<Function> constructor;
		SPA::CTable *m_table;
	};

}