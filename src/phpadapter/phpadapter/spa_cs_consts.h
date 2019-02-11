#ifndef SPA_CLIESIDE_PHP_CONSTS_H
#define SPA_CLIESIDE_PHP_CONSTS_H

namespace PA {
	struct tagConnectionState : public Php::Base {
		void __construct(Php::Parameters &params);
		static void RegisterInto(Php::Namespace &cs);
	};

	void RegisterSpaClientConstsInto(Php::Namespace &cs);

} //namespace PA

#endif
