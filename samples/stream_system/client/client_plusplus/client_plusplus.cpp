
#include "stdafx.h"

int main(int argc, char* argv[]) {
	SPA::UDB::CDBVariant vt(true);
	CComVariant vtDes;
	HRESULT hr = VariantChangeType(&vtDes, &vt, 0, VT_DECIMAL);
	//HRESULT hr = SPA::VariantChangeType(&vtDes, &vt, 0, VT_I4);
	SYSTEMTIME st;
	::GetLocalTime(&st);

	SPA::UDB::CDBVariant vtNow(st);
	vtNow.ullVal /= 2;
	if (vtNow.date < 1.0/3600 * 24) {
		std::cout << "High precision date time" << std::endl;
	}

	hr = VariantChangeType(&vtDes, &vtNow, 0, VT_I2);

	CConnectionContext cc;
	std::cout << "Remote host: " << std::endl;
	std::getline(std::cin, cc.Host);
	//cc.Host = "localhost";
	cc.Port = 20902;
	cc.UserId = L"root";
	cc.Password = L"Smash123";


	return 0;
}

