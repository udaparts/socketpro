#ifndef SPA_PHP_CONSTS_H
#define SPA_PHP_CONSTS_H

struct tagZipLevel : public Php::Base {
	static void RegisterInto(Php::Namespace &spa);
};

struct tagSocketOption : public Php::Base {
	static void RegisterInto(Php::Namespace &spa);
};

struct tagSocketLevel : public Php::Base {
	static void RegisterInto(Php::Namespace &spa);
};

struct tagOperationSystem : public Php::Base {
	static void RegisterInto(Php::Namespace &spa);
};

struct tagThreadApartment : public Php::Base {
	static void RegisterInto(Php::Namespace &spa);
};


struct tagBaseRequestID : public Php::Base {
	static void RegisterInto(Php::Namespace &spa);
};

struct tagChatRequestID : public Php::Base {
	static void RegisterInto(Php::Namespace &spa);
};

struct BaseServiceID : public Php::Base {
	static void RegisterInto(Php::Namespace &spa);
};

struct BaseExceptionCode : public Php::Base {
	static void RegisterInto(Php::Namespace &spa);
};

struct tagEncryptionMethod : public Php::Base {
	static void RegisterInto(Php::Namespace &spa);
};

//tagShutdownType
struct tagSType : public Php::Base {
	static void RegisterInto(Php::Namespace &spa);
};

struct tagQueueStatus : public Php::Base {
	static void RegisterInto(Php::Namespace &spa);
};

struct tagOptimistic : public Php::Base {
	static void RegisterInto(Php::Namespace &spa);
};

void RegisterSpaConstsInto(Php::Namespace &spa);

#endif