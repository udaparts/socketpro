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
	Php::Value SaveDate(Php::Parameters &params);
	Php::Value LoadDate();
	
	Php::Value __get(const Php::Value &name);
	void __set(const Php::Value &name, const Php::Value &value);

private:
	SPA::CUQueue *m_pBuffer;
};

#endif