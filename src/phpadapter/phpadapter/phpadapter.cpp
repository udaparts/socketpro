#include "stdafx.h"
#include "spa_consts.h"
#include "spa_cs_consts.h"
#include "phpbuffer.h"
#include "phpconncontext.h"
#include "roothandler.h"
#include "phpfile.h"
#include "phpdb.h"
#include "phpqueue.h"
#include "phpcert.h"
#include "phpsocket.h"
#include "phpsocketpool.h"

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
		PA::RegisterSpaClientConstsInto(ClientSide);
		PA::CPhpCert::RegisterInto(ClientSide);
		PA::CPhpSocket::RegisterInto(ClientSide);
		PA::CPhpConnContext::RegisterInto(ClientSide);
		PA::CPhpSocketPool::RegisterInto(ClientSide);
		PA::CPhpHandler::RegisterInto(ClientSide);
		PA::CPhpFile::RegisterInto(ClientSide);
		PA::CPhpDb::RegisterInto(ClientSide);
		PA::CPhpQueue::RegisterInto(ClientSide);

		SPA.add(ClientSide);
		extSpaPhp.add(SPA);

		// return the extension
		return extSpaPhp.module();
	}
}
