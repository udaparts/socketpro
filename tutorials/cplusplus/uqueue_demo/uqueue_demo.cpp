
#include "stdafx.h"
#include "mystruct.h"

int main(int argc, char* argv[]) {
	SPA::CScopeUQueue su;
	CMyStruct msOriginal;
	{
		msOriginal.ObjBool = true;
		msOriginal.UnicodeString = L"Unicode";
		msOriginal.ABool = true;
		msOriginal.ADouble = 1234.567;
		msOriginal.AsciiString = "ASCII";
#ifdef WIN32_64
		msOriginal.ObjString = L"test";
		{
			long *data;
			SAFEARRAYBOUND sab[1] = {2, 0};
			msOriginal.objArrInt.vt = (VT_ARRAY | VT_I4);
			msOriginal.objArrInt.parray = ::SafeArrayCreate(VT_I4, 1, sab);
			::SafeArrayAccessData(msOriginal.objArrInt.parray, (void**) &data);
			data[0] = 1;
			data[1] = 76890;
			::SafeArrayUnaccessData(msOriginal.objArrInt.parray);
		}

		{
			BSTR *data;
			SAFEARRAYBOUND sab[1] = {2, 0};
			msOriginal.objArrString.vt = (VT_ARRAY | VT_BSTR);
			msOriginal.objArrString.parray = ::SafeArrayCreate(VT_BSTR, 1, sab);
			::SafeArrayAccessData(msOriginal.objArrString.parray, (void**) &data);
			data[0] = ::SysAllocString(L"Hello");
			data[1] = ::SysAllocString(L"world");
			::SafeArrayUnaccessData(msOriginal.objArrString.parray);
		}
#else
		msOriginal.ObjString = std::wstring(L"test");

		{
			std::vector<int> vs;
			vs.push_back(1);
			vs.push_back(76890);
			msOriginal.objArrInt = vs;
		}

		{

			std::vector<std::wstring> vs;
			vs.push_back(std::wstring(L"Hello"));
			vs.push_back(std::wstring(L"world"));
			msOriginal.objArrString = vs;
		}
#endif
		su << msOriginal;

		CMyStruct ms;
		su >> ms;

		assert(su->GetSize() == 0);
		bool equal = (ms == msOriginal);
		assert(equal);
	}
	return 0;
}

