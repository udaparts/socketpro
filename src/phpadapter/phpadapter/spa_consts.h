#ifndef SPA_PHP_CONSTS_H
#define SPA_PHP_CONSTS_H

namespace PA {

	struct tagZipLevel : public Php::Base {
		static void RegisterInto(Php::Namespace &spa);
	};

	struct tagOperationSystem : public Php::Base {
		static void RegisterInto(Php::Namespace &spa);
	};

	struct tagBaseRequestID : public Php::Base {
		static void RegisterInto(Php::Namespace &spa);
	};

	struct BaseServiceID : public Php::Base {
		static void RegisterInto(Php::Namespace &spa);
	};

	struct tagEncryptionMethod : public Php::Base {
		static void RegisterInto(Php::Namespace &spa);
	};

	struct tagQueueStatus : public Php::Base {
		static void RegisterInto(Php::Namespace &spa);
	};

	struct tagOptimistic : public Php::Base {
		static void RegisterInto(Php::Namespace &spa);
	};

	void RegisterSpaConstsInto(Php::Namespace &spa);

} //namespace PA {

#endif