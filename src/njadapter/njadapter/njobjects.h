#ifndef __SOCKETPRO_NODEJS_ADAPTER_NJOBJECTS_H__
#define __SOCKETPRO_NODEJS_ADAPTER_NJOBJECTS_H__

namespace NJA {

	class NJAObjects : public node::ObjectWrap {
	public:
		static void Init(Local<Object> exports);
		static void At_Exit(void *arg);

	private:
		explicit NJAObjects(double value = 0);
		~NJAObjects();
		NJAObjects(const NJAObjects &obj) = delete;
		NJAObjects& operator=(const NJAObjects &obj) = delete;

		static void New(const FunctionCallbackInfo<Value>& args);
		static void PlusOne(const FunctionCallbackInfo<Value>& args);
		static Persistent<Function> constructor;

		double value_;
	};
}

#endif
