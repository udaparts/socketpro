
#include "stdafx.h"

void myFunction()
{
	Php::out << "example output" << std::endl;
}

extern "C" {
	SPA_PHP_EXPORT void *get_module() {
		static Php::Extension extension("my_extension", "1.0");
		extension.add("myFunction", myFunction);
		return extension;
	}
}
