#ifndef SPA_PHP_FILE_H
#define SPA_PHP_FILE_H

#include "roothandler.h"

namespace PA {
	class CPhpFile : public CRootHandler
	{
	public:
		CPhpFile(SPA::ClientSide::CStreamingFile *sh, bool locked = false);
		~CPhpFile();

	public:
		static void RegisterInto(Php::Namespace &cs);

	private:
		SPA::ClientSide::CStreamingFile *m_sh;
	};
} //namespace PA
#endif
