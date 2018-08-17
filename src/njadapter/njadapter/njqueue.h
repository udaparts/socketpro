#pragma once
#include "../../../include/membuffer.h"

namespace NJA {
	using SPA::CUQueue;
	class NJQueue : public node::ObjectWrap {
	public:
		NJQueue(unsigned int initialSize, unsigned int blockSize);
		~NJQueue();

	public:
		inline CUQueue* get() const {
			return m_Buffer;
		}
		static void Init(Local<Object> exports);

	private:
		void Release();
		void Ensure();
		static void New(const FunctionCallbackInfo<Value>& args);
		static Persistent<Function> constructor;
		template <class ctype>
		unsigned int Load(Isolate* isolate, ctype &buffer) {
			if (!m_Buffer) {
				isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "No buffer available")));
				return 0;
			}
			try {
				return m_Buffer->Pop((unsigned char*)&buffer, sizeof(ctype), 0);
			}
			catch (std::exception &err) {
				isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, err.what())));
			}
			return 0;
		}

		template <class ctype>
		void Save(const FunctionCallbackInfo<Value>& args, const ctype &buffer) {
			Ensure();
			m_Buffer->Push((const unsigned char *)&buffer, sizeof(ctype));
			args.GetReturnValue().Set(args.Holder());
		}

		static void Discard(const FunctionCallbackInfo<Value>& args);
		static void Empty(const FunctionCallbackInfo<Value>& args);
		static void getSize(const FunctionCallbackInfo<Value>& args);
		static void setSize(const FunctionCallbackInfo<Value>& args);

		static void LoadBoolean(const FunctionCallbackInfo<Value>& args);
		static void LoadByte(const FunctionCallbackInfo<Value>& args);
		static void LoadAChar(const FunctionCallbackInfo<Value>& args);
		static void LoadShort(const FunctionCallbackInfo<Value>& args);
		static void LoadInt(const FunctionCallbackInfo<Value>& args);
		static void LoadFloat(const FunctionCallbackInfo<Value>& args);
		static void LoadDouble(const FunctionCallbackInfo<Value>& args);
		static void LoadLong(const FunctionCallbackInfo<Value>& args);
		static void LoadULong(const FunctionCallbackInfo<Value>& args);
		static void LoadUInt(const FunctionCallbackInfo<Value>& args);
		static void LoadUShort(const FunctionCallbackInfo<Value>& args);
		static void LoadBytes(const FunctionCallbackInfo<Value>& args);
		static void LoadAString(const FunctionCallbackInfo<Value>& args);
		static void LoadString(const FunctionCallbackInfo<Value>& args);
		static void LoadDecimal(const FunctionCallbackInfo<Value>& args);
		static void LoadDate(const FunctionCallbackInfo<Value>& args);
		static void LoadUUID(const FunctionCallbackInfo<Value>& args);
		static void PopBytes(const FunctionCallbackInfo<Value>& args);

		static void SaveBoolean(const FunctionCallbackInfo<Value>& args);
		static void SaveByte(const FunctionCallbackInfo<Value>& args);
		static void SaveAChar(const FunctionCallbackInfo<Value>& args);
		static void SaveShort(const FunctionCallbackInfo<Value>& args);
		static void SaveInt(const FunctionCallbackInfo<Value>& args);
		static void SaveFloat(const FunctionCallbackInfo<Value>& args);
		static void SaveDouble(const FunctionCallbackInfo<Value>& args);
		static void SaveLong(const FunctionCallbackInfo<Value>& args);
		static void SaveULong(const FunctionCallbackInfo<Value>& args);
		static void SaveUInt(const FunctionCallbackInfo<Value>& args);
		static void SaveUShort(const FunctionCallbackInfo<Value>& args);
		static void SaveBytes(const FunctionCallbackInfo<Value>& args);
		static void SaveAString(const FunctionCallbackInfo<Value>& args);
		static void SaveString(const FunctionCallbackInfo<Value>& args);
		static void SaveDecimal(const FunctionCallbackInfo<Value>& args);
		static void SaveDate(const FunctionCallbackInfo<Value>& args);
		static void SaveUUID(const FunctionCallbackInfo<Value>& args);

	private:
		unsigned int m_initSize;
		unsigned int m_blockSize;
		CUQueue *m_Buffer;
	};

}