#ifndef SPA_PHP_FILE_H
#define SPA_PHP_FILE_H

namespace PA {
	class CPhpFile
	{
	public:
		CPhpFile();
		~CPhpFile();

	public:
		static void RegisterInto(Php::Namespace &cs);
	};
} //namespace PA
#endif