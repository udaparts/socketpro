#include "../include/membuffer.h"
#include "../include/rawclient.h"
#include "prelogin.h"
#include <iostream>

using namespace SPA;

bool CALLBACK CVCallback(bool preverified, int depth, int errorCode, const char *errMessage, SPA::CertInfo * ci) {
	std::cout << "depth: " << depth << ", errCode: " << errMessage << "\n";
	return true;
}

int main()
{
	tds::Prelogin pl(1, true);
	SPA::CScopeUQueue sb;
	bool ok = pl.GetPreloginMessage(*sb);
	std::cout << "Press a key to shut down the application ......\n";
	::getchar();
	return 0;
}
