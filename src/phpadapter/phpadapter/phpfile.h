#ifndef SPA_PHP_FILE_H
#define SPA_PHP_FILE_H

namespace PA {
	typedef SPA::ClientSide::CStreamingFile CAsyncFile;
	typedef SPA::ClientSide::CSocketPool<CAsyncFile> CPhpFilePool;

	class CPhpFile : public Php::Base
	{
	public:
		CPhpFile(CPhpFilePool *pool, CAsyncFile *sh, bool locked);
		CPhpFile(const CPhpFile &file) = delete;
		~CPhpFile();

	public:
		CPhpFile& operator=(const CPhpFile &file) = delete;
		void __construct(Php::Parameters &params);
		static void RegisterInto(Php::Namespace &cs);
		Php::Value SendRequest(Php::Parameters &params);
		bool IsLocked();

	private:
		CPhpFilePool *m_filePool;
		CAsyncFile *m_sh;
		bool m_locked;
	};
} //namespace PA
#endif
