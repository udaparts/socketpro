#include "stdafx.h"
#include "spa_consts.h"
#include "spa_cs_consts.h"
#include "phpsocket.h"
#include "roothandler.h"
#include "phpfile.h"
#include "phpdb.h"
#include "phpqueue.h"
#include "phpcert.h"
#include "phpsocketpool.h"
#include "phpmanager.h"
#include "phpdbcolumninfo.h"
#include "phpdbparaminfo.h"
#include "phppush.h"
#include "phpclientqueue.h"
#include "phptable.h"
#include "phpdataset.h"

extern "C" {
	SPA_PHP_EXPORT void *get_module() {
		// create static instance of the extension object
		static Php::Extension extSpaPhp("spaphp", "1.0.0.1");
		Php::Namespace SPA("SPA");

		SPA.add(Php::Constant("OperationSystem", SPA::GetOS()));
		SPA.add(Php::Constant("Endian", SPA::IsBigEndian()));

		PA::CPhpTable::RegisterInto(SPA);
		PA::CPhpDataSet::RegisterInto(SPA);

		//tag and other const defines
		PA::RegisterSpaConstsInto(SPA);

		//CUQueue
		PA::CPhpBuffer::RegisterInto(SPA);

		//namespace ClientSide
		Php::Namespace ClientSide("ClientSide");
		PA::RegisterSpaClientConstsInto(ClientSide);
		PA::CPhpCert::RegisterInto(ClientSide);
		PA::CPhpSocket::RegisterInto(ClientSide);
		PA::CPhpSocketPool::RegisterInto(ClientSide);
		PA::CPhpHandler::RegisterInto(ClientSide);
		PA::CPhpFile::RegisterInto(ClientSide);
		PA::CPhpDb::RegisterInto(ClientSide);
		PA::CPhpQueue::RegisterInto(ClientSide);
		PA::CPhpManager::RegisterInto(ClientSide);
		PA::CPhpDBColumnInfo::RegisterInto(ClientSide);
		PA::CPhpDBParamInfo::RegisterInto(ClientSide);
		PA::CPhpPush::RegisterInto(ClientSide);
		PA::CPhpClientQueue::RegisterInto(ClientSide);

		ClientSide.add("GetManager", PA::GetManager);
		SPA.add(ClientSide);
		extSpaPhp.add(SPA);
		extSpaPhp.add("GetSpManager", PA::GetManager);
		extSpaPhp.add("GetSpPool", PA::GetSpPool, {
			Php::ByVal(PA::PHP_KEY, Php::Type::String)
		});
		extSpaPhp.add("SeekSpHandler", PA::GetSpHandler, {
			Php::ByVal(PA::PHP_KEY, Php::Type::String)
		});
		extSpaPhp.add("GetSpHandler", PA::GetSpHandler, {
			Php::ByVal(PA::PHP_KEY, Php::Type::String)
		});
		extSpaPhp.add("LockSpHandler", PA::LockSpHandler, {
			Php::ByVal(PA::PHP_KEY, Php::Type::String),
			Php::ByVal(PA::PHP_TIMEOUT, Php::Type::Numeric, false)
		});
		extSpaPhp.add("SpBuffer", PA::SpBuff);
		return extSpaPhp.module();
	}
}
