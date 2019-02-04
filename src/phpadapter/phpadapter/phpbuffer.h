#ifndef SPA_PHP_BUFFER_H
#define SPA_PHP_BUFFER_H

namespace PA {

	class CPhpBuffer : public Php::Base {
	public:
		CPhpBuffer();
		CPhpBuffer(const CPhpBuffer &b) = delete;
		~CPhpBuffer();

	public:
		CPhpBuffer& operator=(const CPhpBuffer &b) = delete;
		static void RegisterInto(Php::Namespace &spa);
		void __construct(Php::Parameters &params);
		int __compare(const CPhpBuffer &b) const;
		void Empty();
		void CleanTrack();
		Php::Value Discard(Php::Parameters &params);
		Php::Value SaveByte(Php::Parameters &params);
		Php::Value LoadByte();
		Php::Value SaveDate(Php::Parameters &params);
		Php::Value LoadDate();
		Php::Value SaveAChar(Php::Parameters &params);
		Php::Value LoadAChar();
		Php::Value SaveBool(Php::Parameters &params);
		Php::Value LoadBool();
		Php::Value SaveShort(Php::Parameters &params);
		Php::Value LoadShort();
		Php::Value SaveInt(Php::Parameters &params);
		Php::Value LoadInt();
		Php::Value SaveUInt(Php::Parameters &params);
		Php::Value LoadUInt();
		Php::Value SaveUShort(Php::Parameters &params);
		Php::Value LoadUShort();
		Php::Value SaveLong(Php::Parameters &params);
		Php::Value LoadLong();
		Php::Value SaveDouble(Php::Parameters &params);
		Php::Value LoadDouble();
		Php::Value SaveFloat(Php::Parameters &params);
		Php::Value LoadFloat();
		Php::Value SaveAString(Php::Parameters &params);
		Php::Value LoadAString();
		Php::Value SaveString(Php::Parameters &params);
		Php::Value LoadString();
		Php::Value SaveDecimal(Php::Parameters &params);
		Php::Value LoadDecimal();
		Php::Value PushBytes(Php::Parameters &params);
		Php::Value PopBytes(Php::Parameters &params);
		Php::Value SaveUUID(Php::Parameters &params);
		Php::Value LoadUUID();
		Php::Value SaveObject(Php::Parameters &params);
		Php::Value LoadObject();
		Php::Value Save(Php::Parameters &params);
		Php::Value Load(Php::Parameters &params);
		Php::Value __get(const Php::Value &name);
		void __set(const Php::Value &name, const Php::Value &value);
		void Swap(SPA::CUQueue *q);
		void Swap(CPhpBuffer *qPhp);

	private:
		void EnsureBuffer();
		void SaveObject(const Php::Value &param, const std::string &id = "");
		void SaveString(const Php::Value &param);
		void SaveDecimal(const Php::Value &param);
		void SaveDate(const Php::Value &param);
		void SaveAString(const Php::Value &param);

	private:
		SPA::CUQueue *m_pBuffer;
		friend void ToVariant(const Php::Value &data, SPA::UDB::CDBVariant &vt, const std::string &id);
		friend void ToVariant(const Php::Value &data, CComVariant &vt, const std::string &id);
		friend SPA::ClientSide::CAsyncServiceHandler;
		friend class CPhpQueue;
	};

} //namespace PA

#endif