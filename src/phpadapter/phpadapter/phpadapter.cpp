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

		//IUSerializer
		Php::Interface serializer("IUSerializer");
		serializer.method("LoadFrom", {
			Php::ByRef("q", "CUQueue")
		});
		serializer.method("SaveTo", {
			Php::ByRef("q", "CUQueue")
		});
		SPA.add(serializer);

		//tag and other const defines
		RegisterSpaConstsInto(SPA);

		//CUQueue
		CPhpBuffer::RegisterInto(SPA);

		//namespace ClientSide
		Php::Namespace ClientSide("ClientSide");
		SPA.add(ClientSide);
		extSpaPhp.add(SPA);

		// return the extension
		return extSpaPhp.module();
	}
}
