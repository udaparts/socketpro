#ifndef SPA_PHP_FILE_H
#define SPA_PHP_FILE_H

#include "roothandler.h"

namespace PA {
	typedef SPA::ClientSide::CStreamingFile CAsyncFile;

	class CPhpFile : public CRootHandler
	{
	public:
		CPhpFile(CPhpPool *pool, CAsyncFile *sh, bool locked);
		CPhpFile(const CPhpFile &file) = delete;

	public:
		CPhpFile& operator=(const CPhpFile &file) = delete;
		void __construct(Php::Parameters &params);
		static void RegisterInto(Php::Namespace &cs);

	private:
		SPA::ClientSide::CStreamingFile *m_sh;
	};
} //namespace PA
#endif
