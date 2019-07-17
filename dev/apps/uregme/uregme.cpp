// uregme.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>

#include "../../ucomm/core_shared/pinc/regkeys.h"
#include "../../ucomm/core_shared/pinc/getsysid.h"

int main(int argc, char* argv[])
{
	int os;
	int manyMachines = 0;
	char secret[10] = USecretKey;
	std::string qName;
	std::cout << "Please input a queue file name without the part '_sp.json' ......" << std::endl;
	std::cin >> qName;
	std::cout << "Please input a number for a license type (0 -- one machine only, 1 -- many machines, and 2 -- enterprise) ......" << std::endl;
	std::cin >> manyMachines;
	if (manyMachines > 2) {
		manyMachines = 2;
	}
aa:	std::cout << "What is operation system (win = 0, apple = 1, unix = 2) ?" << std::endl;
	std::cin >> os;
	if (os < 0 || os > 2) {
		std::cout << "Bad operation system! Please re-input it." << std::endl;
		goto  aa;
	}

	std::string s = SPA::ServerSide::CreateKey(qName.c_str(), (unsigned char)manyMachines, secret, (SPA::tagOperationSystem)os);
    std::ofstream outfile("key.txt");
    if (outfile) {
        outfile << s;
        outfile.close();
    }
	return 0;
}

