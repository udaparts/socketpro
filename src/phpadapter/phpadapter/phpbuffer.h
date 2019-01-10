#ifndef SPA_PHP_BUFFER_H
#define SPA_PHP_BUFFER_H

class CPhpBuffer : public Php::Base {
public:
	CPhpBuffer(SPA::CUQueue *buffer = nullptr);
	~CPhpBuffer();

public:
	static void RegisterInto(Php::Namespace &spa);
	void __construct(Php::Parameters &params);
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

	Php::Value __get(const Php::Value &name);
	void __set(const Php::Value &name, const Php::Value &value);


private:
	void EnsureBuffer();

private:
	SPA::CUQueue *m_pBuffer;
};

#endif