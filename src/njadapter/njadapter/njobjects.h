#pragma once

namespace NJA {

	class MyObject : public node::ObjectWrap {
	public:
		static void Init(Local<Object> exports);
		static void At_Exit(void *arg);

	private:
		explicit MyObject(double value = 0);
		~MyObject();

		MyObject(const MyObject &obj) = delete;
		MyObject& operator=(const MyObject &obj) = delete;

		static void New(const FunctionCallbackInfo<Value>& args);
		static void PlusOne(const FunctionCallbackInfo<Value>& args);
		static Persistent<Function> constructor;


		double value_;
	};
}
