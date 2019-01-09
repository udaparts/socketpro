#include "stdafx.h"
#include "spa_consts.h"
#include "phpbuffer.h"

extern "C" {
	SPA_PHP_EXPORT void *get_module() {
		// create static instance of the extension object
		static Php::Extension extSpaPhp("spaphp", "1.0");

		Php::Namespace SPA("SPA");
		RegisterSpaConstsInto(SPA);

		CPhpBuffer::RegisterInto(SPA);

		Php::Namespace ClientSide("ClientSide");

		SPA.add(ClientSide);
		extSpaPhp.add(SPA);

		// return the extension
		return extSpaPhp.module();
	}
}
