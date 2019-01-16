#include "stdafx.h"
#include "spa_consts.h"
#include "phpbuffer.h"

extern "C" {
	SPA_PHP_EXPORT void *get_module() {
		// create static instance of the extension object
		static Php::Extension extSpaPhp("spaphp", "1.0");
		Php::Namespace SPA("SPA");

		SPA.add(Php::Constant("OperationSystem", SPA::GetOS()));
		SPA.add(Php::Constant("Endian", SPA::IsBigEndian()));

		//tag and other const defines
		PA::RegisterSpaConstsInto(SPA);

		//CUQueue
		PA::CPhpBuffer::RegisterInto(SPA);

		//namespace ClientSide
		Php::Namespace ClientSide("ClientSide");
		SPA.add(ClientSide);
		extSpaPhp.add(SPA);

		// return the extension
		return extSpaPhp.module();
	}
}
