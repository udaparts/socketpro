#ifndef SPA_PHP_FILE_H
#define SPA_PHP_FILE_H

#include "basehandler.h"

namespace PA {
	typedef SPA::ClientSide::CStreamingFile CAsyncFile;
	typedef SPA::ClientSide::CSocketPool<CAsyncFile> CPhpFilePool;

	class CPhpFile : public CPhpBaseHandler
	{
	public:
		CPhpFile(unsigned int poolId, CAsyncFile *sh, bool locked);
		CPhpFile(const CPhpFile &file) = delete;

	public:
		CPhpFile& operator=(const CPhpFile &file) = delete;
		static void RegisterInto(Php::Class<CPhpBaseHandler> &base, Php::Namespace &cs);
		Php::Value __get(const Php::Value &name);

	protected:
		void PopTopCallbacks(PACallback &cb);

	private:
		Php::Value Download(Php::Parameters &params);
		Php::Value Upload(Php::Parameters &params);
		CAsyncFile::DDownload SetResCallback(Php::Value phpDl, std::shared_ptr<Php::Value> &pV, unsigned int &timeout);
		static void MapFilePaths(Php::Value phpLocal, Php::Value phpRemote, std::wstring &local, std::wstring &remote);

	private:
		CAsyncFile *m_sh;
	};
} //namespace PA
#endif
