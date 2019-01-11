
#include "stdafx.h"
#include <cctype>

void Trim(std::string &str) {
	while (str.size() && std::isspace(str.back())) {
		str.pop_back();
	}
	while (str.size() && std::isspace(str.front())) {
		str.erase(0, 1);
	}
}

