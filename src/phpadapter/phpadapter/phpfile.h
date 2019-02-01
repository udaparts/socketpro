#ifndef SPA_PHP_FILE_H
#define SPA_PHP_FILE_H

#include "basehandler.h"

namespace PA {
	typedef SPA::ClientSide::CStreamingFile CAsyncFile;
	typedef SPA::ClientSide::CSocketPool<CAsyncFile> CPhpFilePool;

	class CPhpFile : public CPhpBaseHandler<CPhpFile>
	{
	public:
		CPhpFile(CPhpFilePool *pool, CAsyncFile *sh, bool locked);
		CPhpFile(const CPhpFile &file) = delete;

	public:
		CPhpFile& operator=(const CPhpFile &file) = delete;
		static void RegisterInto(Php::Namespace &cs);
		int __compare(const CPhpFile &f) const;

	private:
		CPhpFilePool *m_filePool;
		CAsyncFile *m_sh;
	};
} //namespace PA
#endif
