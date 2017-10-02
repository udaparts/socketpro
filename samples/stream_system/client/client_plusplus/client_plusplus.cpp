
#include "stdafx.h"

namespace SPA {
	static HRESULT VariantChangeType(VARIANT *pvargDest, const VARIANT *pvarSrc, unsigned short wFlags, VARTYPE vt) {
		if (!pvargDest || !pvarSrc)
			return E_INVALIDARG;
		::VariantClear(pvargDest);
		pvargDest->vt = vt;
		switch (vt) {
		case VT_I1:
			switch (pvarSrc->vt) {
			case VT_I1:
				pvargDest->cVal = pvarSrc->cVal;
				break;
			case VT_I2:
				pvargDest->cVal = (char)pvarSrc->iVal;
				break;
			case VT_INT:
			case VT_I4:
				pvargDest->cVal = (char)pvarSrc->intVal;
				break;
			case VT_I8:
				pvargDest->cVal = (char)pvarSrc->llVal;
				break;
			case VT_UI1:
				pvargDest->cVal = (char)pvarSrc->bVal;
				break;
			case VT_UI2:
				pvargDest->cVal = (char)pvarSrc->uiVal;
				break;
			case VT_UINT:
			case VT_UI4:
				pvargDest->cVal = (char)pvarSrc->ulVal;
				break;
			case VT_UI8:
				pvargDest->cVal = (char)pvarSrc->ullVal;
				break;
			case VT_R4:
				pvargDest->cVal = (char)pvarSrc->fltVal;
				break;
			case VT_R8:
				pvargDest->cVal = (char)pvarSrc->dblVal;
				break;
			case VT_BOOL:
				pvargDest->cVal = (char)(pvarSrc->boolVal ? 1 : 0);
				break;
			case VT_DECIMAL:
				pvargDest->cVal = (char)SPA::ToDouble(pvarSrc->decVal);
				break;
			case VT_BSTR:
			{
				INT64 data;
				int res = swscanf(pvarSrc->bstrVal, L"%lld", &data);
				if (res)
					pvargDest->cVal = (char)data;
				else
				{
					pvargDest->vt = VT_EMPTY;
					return E_UNEXPECTED;
				}
			}
			break;
			case (VT_I1 | VT_ARRAY) :
			{
				INT64 data;
				const char *s;
				::SafeArrayAccessData(pvarSrc->parray, (void**)&s);
				int res = sscanf(s, "%lld", &data);
				::SafeArrayUnaccessData(pvarSrc->parray);
				if (res)
					pvargDest->cVal = (char)data;
				else
				{
					pvargDest->vt = VT_EMPTY;
					return E_UNEXPECTED;
				}
			}
			break;
			default:
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
			break;
		case VT_I2:
			switch (pvarSrc->vt) {
			case VT_I1:
				pvargDest->iVal = pvarSrc->cVal;
				break;
			case VT_I2:
				pvargDest->iVal = pvarSrc->iVal;
				break;
			case VT_INT:
			case VT_I4:
				pvargDest->iVal = (short)pvarSrc->intVal;
				break;
			case VT_I8:
				pvargDest->iVal = (short)pvarSrc->llVal;
				break;
			case VT_UI1:
				pvargDest->iVal = (short)pvarSrc->bVal;
				break;
			case VT_UI2:
				pvargDest->iVal = (short)pvarSrc->uiVal;
				break;
			case VT_UINT:
			case VT_UI4:
				pvargDest->iVal = (short)pvarSrc->ulVal;
				break;
			case VT_UI8:
				pvargDest->iVal = (short)pvarSrc->ullVal;
				break;
			case VT_R4:
				pvargDest->iVal = (short)pvarSrc->fltVal;
				break;
			case VT_R8:
				pvargDest->iVal = (short)pvarSrc->dblVal;
				break;
			case VT_BOOL:
				pvargDest->iVal = (short)(pvarSrc->boolVal ? 1 : 0);
				break;
			case VT_DECIMAL:
				pvargDest->iVal = (short)SPA::ToDouble(pvarSrc->decVal);
				break;
			case VT_BSTR:
			{
				INT64 data;
				int res = swscanf(pvarSrc->bstrVal, L"%lld", &data);
				if (res)
					pvargDest->iVal = (short)data;
				else
				{
					pvargDest->vt = VT_EMPTY;
					return E_UNEXPECTED;
				}
			}
			break;
			case (VT_I1 | VT_ARRAY) :
			{
				INT64 data;
				const char *s;
				::SafeArrayAccessData(pvarSrc->parray, (void**)&s);
				int res = sscanf(s, "%lld", &data);
				::SafeArrayUnaccessData(pvarSrc->parray);
				if (res)
					pvargDest->iVal = (short)data;
				else
				{
					pvargDest->vt = VT_EMPTY;
					return E_UNEXPECTED;
				}
			}
			break;
			default:
				return E_UNEXPECTED;
			}
			break;
		case VT_INT:
		case VT_I4:
			switch (pvarSrc->vt) {
			case VT_I1:
				pvargDest->intVal = pvarSrc->cVal;
				break;
			case VT_I2:
				pvargDest->intVal = pvarSrc->iVal;
				break;
			case VT_INT:
			case VT_I4:
				pvargDest->intVal = pvarSrc->intVal;
				break;
			case VT_I8:
				pvargDest->intVal = (int)pvarSrc->llVal;
				break;
			case VT_UI1:
				pvargDest->intVal = (int)pvarSrc->bVal;
				break;
			case VT_UI2:
				pvargDest->intVal = (int)pvarSrc->uiVal;
				break;
			case VT_UINT:
			case VT_UI4:
				pvargDest->intVal = (int)pvarSrc->ulVal;
				break;
			case VT_UI8:
				pvargDest->intVal = (int)pvarSrc->ullVal;
				break;
			case VT_R4:
				pvargDest->intVal = (int)pvarSrc->fltVal;
				break;
			case VT_R8:
				pvargDest->intVal = (int)pvarSrc->dblVal;
				break;
			case VT_BOOL:
				pvargDest->intVal = (int)(pvarSrc->boolVal ? 1 : 0);
				break;
			case VT_DECIMAL:
				pvargDest->intVal = (int)SPA::ToDouble(pvarSrc->decVal);
				break;
			case VT_BSTR:
			{
				INT64 data;
				int res = swscanf(pvarSrc->bstrVal, L"%lld", &data);
				if (res)
					pvargDest->intVal = (int)data;
				else
				{
					pvargDest->vt = VT_EMPTY;
					return E_UNEXPECTED;
				}
			}
			break;
			case (VT_I1 | VT_ARRAY) :
			{
				INT64 data;
				const char *s;
				::SafeArrayAccessData(pvarSrc->parray, (void**)&s);
				int res = sscanf(s, "%lld", &data);
				::SafeArrayUnaccessData(pvarSrc->parray);
				if (res)
					pvargDest->intVal = (int)data;
				else
				{
					pvargDest->vt = VT_EMPTY;
					return E_UNEXPECTED;
				}
			}
			break;
			default:
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
			break;
		case VT_I8:
			switch (pvarSrc->vt) {
			case VT_I1:
				pvargDest->llVal = pvarSrc->cVal;
				break;
			case VT_I2:
				pvargDest->llVal = pvarSrc->iVal;
				break;
			case VT_INT:
			case VT_I4:
				pvargDest->llVal = pvarSrc->intVal;
				break;
			case VT_I8:
				pvargDest->llVal = pvarSrc->llVal;
				break;
			case VT_UI1:
				pvargDest->llVal = pvarSrc->bVal;
				break;
			case VT_UI2:
				pvargDest->llVal = (INT64)pvarSrc->uiVal;
				break;
			case VT_UINT:
			case VT_UI4:
				pvargDest->llVal = (INT64)pvarSrc->ulVal;
				break;
			case VT_UI8:
				pvargDest->llVal = (INT64)pvarSrc->ullVal;
				break;
			case VT_R4:
				pvargDest->llVal = (INT64)pvarSrc->fltVal;
				break;
			case VT_R8:
				pvargDest->llVal = (INT64)pvarSrc->dblVal;
				break;
			case VT_BOOL:
				pvargDest->llVal = (pvarSrc->boolVal ? 1 : 0);
				break;
			case VT_DECIMAL:
				pvargDest->llVal = (INT64)SPA::ToDouble(pvarSrc->decVal);
				break;
			case VT_BSTR:
			{
				INT64 data;
				int res = swscanf(pvarSrc->bstrVal, L"%lld", &data);
				if (res)
					pvargDest->llVal = (char)data;
				else
				{
					pvargDest->vt = VT_EMPTY;
					return E_UNEXPECTED;
				}
			}
			break;
			case (VT_I1 | VT_ARRAY) :
			{
				INT64 data;
				const char *s;
				::SafeArrayAccessData(pvarSrc->parray, (void**)&s);
				int res = sscanf(s, "%lld", &data);
				::SafeArrayUnaccessData(pvarSrc->parray);
				if (res)
					pvargDest->llVal = data;
				else
				{
					pvargDest->vt = VT_EMPTY;
					return E_UNEXPECTED;
				}
			}
			break;
			default:
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
			break;
		case VT_R4:
			switch (pvarSrc->vt) {
			case VT_I1:
				pvargDest->fltVal = (float)pvarSrc->cVal;
				break;
			case VT_I2:
				pvargDest->fltVal = (float)pvarSrc->iVal;
				break;
			case VT_INT:
			case VT_I4:
				pvargDest->fltVal = (float)pvarSrc->intVal;
				break;
			case VT_I8:
				pvargDest->fltVal = (float)pvarSrc->llVal;
				break;
			case VT_UI1:
				pvargDest->fltVal = (float)pvarSrc->bVal;
				break;
			case VT_UI2:
				pvargDest->fltVal = (float)pvarSrc->uiVal;
				break;
			case VT_UINT:
			case VT_UI4:
				pvargDest->fltVal = (float)pvarSrc->ulVal;
				break;
			case VT_UI8:
				pvargDest->fltVal = (float)pvarSrc->ullVal;
				break;
			case VT_R4:
				pvargDest->fltVal = pvarSrc->fltVal;
				break;
			case VT_R8:
				pvargDest->fltVal = (float)pvarSrc->dblVal;
				break;
			case VT_BOOL:
				pvargDest->fltVal = (float)(pvarSrc->boolVal ? 1 : 0);
				break;
			case VT_DECIMAL:
				pvargDest->fltVal = (float)SPA::ToDouble(pvarSrc->decVal);
				break;
			case VT_BSTR:
			{
				float data;
				int res = swscanf(pvarSrc->bstrVal, L"%f", &data);
				if (res)
					pvargDest->fltVal = data;
				else
				{
					pvargDest->vt = VT_EMPTY;
					return E_UNEXPECTED;
				}
			}
			break;
			case (VT_I1 | VT_ARRAY) :
			{
				float data;
				const char *s;
				::SafeArrayAccessData(pvarSrc->parray, (void**)&s);
				int res = sscanf(s, "%f", &data);
				::SafeArrayUnaccessData(pvarSrc->parray);
				if (res)
					pvargDest->fltVal = data;
				else
				{
					pvargDest->vt = VT_EMPTY;
					return E_UNEXPECTED;
				}
			}
									break;
			default:
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
			break;
		case VT_R8:
			switch (pvarSrc->vt) {
			case VT_I1:
				pvargDest->dblVal = (double)pvarSrc->cVal;
				break;
			case VT_I2:
				pvargDest->dblVal = (double)pvarSrc->iVal;
				break;
			case VT_INT:
			case VT_I4:
				pvargDest->dblVal = (double)pvarSrc->intVal;
				break;
			case VT_I8:
				pvargDest->dblVal = (double)pvarSrc->llVal;
				break;
			case VT_UI1:
				pvargDest->dblVal = (double)pvarSrc->bVal;
				break;
			case VT_UI2:
				pvargDest->dblVal = (double)pvarSrc->uiVal;
				break;
			case VT_UINT:
			case VT_UI4:
				pvargDest->dblVal = (double)pvarSrc->ulVal;
				break;
			case VT_UI8:
				pvargDest->dblVal = (double)pvarSrc->ullVal;
				break;
			case VT_R4:
				pvargDest->dblVal = pvarSrc->fltVal;
				break;
			case VT_R8:
				pvargDest->dblVal = pvarSrc->dblVal;
				break;
			case VT_BOOL:
				pvargDest->dblVal = (double)(pvarSrc->boolVal ? 1 : 0);
				break;
			case VT_DECIMAL:
				pvargDest->dblVal = SPA::ToDouble(pvarSrc->decVal);
				break;
			case VT_BSTR:
			{
				double data;
				int res = swscanf(pvarSrc->bstrVal, L"%lf", &data);
				if (res)
					pvargDest->dblVal = data;
				else
				{
					pvargDest->vt = VT_EMPTY;
					return E_UNEXPECTED;
				}
			}
			break;
			case (VT_I1 | VT_ARRAY) :
			{
				double data;
				const char *s;
				::SafeArrayAccessData(pvarSrc->parray, (void**)&s);
				int res = sscanf(s, "%lf", &data);
				::SafeArrayUnaccessData(pvarSrc->parray);
				if (res)
					pvargDest->dblVal = data;
				else
				{
					pvargDest->vt = VT_EMPTY;
					return E_UNEXPECTED;
				}
			}
									break;
			default:
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
			break;
		case VT_UI1:
			switch (pvarSrc->vt) {
			case VT_I1:
				pvargDest->bVal = (unsigned char)pvarSrc->cVal;
				break;
			case VT_I2:
				pvargDest->bVal = (unsigned char)pvarSrc->iVal;
				break;
			case VT_INT:
			case VT_I4:
				pvargDest->bVal = (unsigned char)pvarSrc->intVal;
				break;
			case VT_I8:
				pvargDest->bVal = (unsigned char)pvarSrc->llVal;
				break;
			case VT_UI1:
				pvargDest->bVal = (unsigned char)pvarSrc->bVal;
				break;
			case VT_UI2:
				pvargDest->bVal = (unsigned char)pvarSrc->uiVal;
				break;
			case VT_UINT:
			case VT_UI4:
				pvargDest->bVal = (unsigned char)pvarSrc->ulVal;
				break;
			case VT_UI8:
				pvargDest->bVal = (unsigned char)pvarSrc->ullVal;
				break;
			case VT_R4:
				pvargDest->bVal = (unsigned char)pvarSrc->fltVal;
				break;
			case VT_R8:
				pvargDest->bVal = (unsigned char)pvarSrc->dblVal;
				break;
			case VT_BOOL:
				pvargDest->bVal = (unsigned char)(pvarSrc->boolVal ? 1 : 0);
				break;
			case VT_DECIMAL:
				pvargDest->bVal = (unsigned char)SPA::ToDouble(pvarSrc->decVal);
				break;
			case VT_BSTR:
			{
				UINT64 data;
				int res = swscanf(pvarSrc->bstrVal, L"%llu", &data);
				if (res)
					pvargDest->bVal = (char)data;
				else
				{
					pvargDest->vt = VT_EMPTY;
					return E_UNEXPECTED;
				}
			}
			break;
			case (VT_I1 | VT_ARRAY) :
			{
				UINT64 data;
				const char *s;
				::SafeArrayAccessData(pvarSrc->parray, (void**)&s);
				int res = sscanf(s, "%llu", &data);
				::SafeArrayUnaccessData(pvarSrc->parray);
				if (res)
					pvargDest->bVal = (unsigned char)data;
				else
				{
					pvargDest->vt = VT_EMPTY;
					return E_UNEXPECTED;
				}
			}
			break;
			default:
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
			break;
		case VT_UI2:
			switch (pvarSrc->vt) {
			case VT_I1:
				pvargDest->uiVal = (unsigned short)pvarSrc->cVal;
				break;
			case VT_I2:
				pvargDest->uiVal = (unsigned short)pvarSrc->iVal;
				break;
			case VT_INT:
			case VT_I4:
				pvargDest->uiVal = (unsigned short)pvarSrc->intVal;
				break;
			case VT_I8:
				pvargDest->uiVal = (unsigned short)pvarSrc->llVal;
				break;
			case VT_UI1:
				pvargDest->uiVal = (unsigned short)pvarSrc->bVal;
				break;
			case VT_UI2:
				pvargDest->uiVal = (unsigned short)pvarSrc->uiVal;
				break;
			case VT_UINT:
			case VT_UI4:
				pvargDest->uiVal = (unsigned short)pvarSrc->ulVal;
				break;
			case VT_UI8:
				pvargDest->uiVal = (unsigned short)pvarSrc->ullVal;
				break;
			case VT_R4:
				pvargDest->uiVal = (unsigned short)pvarSrc->fltVal;
				break;
			case VT_R8:
				pvargDest->uiVal = (unsigned short)pvarSrc->dblVal;
				break;
			case VT_BOOL:
				pvargDest->uiVal = (unsigned short)(pvarSrc->boolVal ? 1 : 0);
				break;
			case VT_DECIMAL:
				pvargDest->uiVal = (unsigned short)SPA::ToDouble(pvarSrc->decVal);
				break;
			case VT_BSTR:
			{
				UINT64 data;
				int res = swscanf(pvarSrc->bstrVal, L"%llu", &data);
				if (res)
					pvargDest->uiVal = (unsigned short)data;
				else
				{
					pvargDest->vt = VT_EMPTY;
					return E_UNEXPECTED;
				}
			}
			break;
			case (VT_I1 | VT_ARRAY) :
			{
				UINT64 data;
				const char *s;
				::SafeArrayAccessData(pvarSrc->parray, (void**)&s);
				int res = sscanf(s, "%llu", &data);
				::SafeArrayUnaccessData(pvarSrc->parray);
				if (res)
					pvargDest->uiVal = (unsigned short)data;
				else
				{
					pvargDest->vt = VT_EMPTY;
					return E_UNEXPECTED;
				}
			}
									break;
			default:
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
			break;
		case VT_UINT:
		case VT_UI4:
			switch (pvarSrc->vt) {
			case VT_I1:
				pvargDest->uintVal = (unsigned int)pvarSrc->cVal;
				break;
			case VT_I2:
				pvargDest->uintVal = (unsigned int)pvarSrc->iVal;
				break;
			case VT_INT:
			case VT_I4:
				pvargDest->uintVal = (unsigned int)pvarSrc->intVal;
				break;
			case VT_I8:
				pvargDest->uintVal = (unsigned int)pvarSrc->llVal;
				break;
			case VT_UI1:
				pvargDest->uintVal = (unsigned int)pvarSrc->bVal;
				break;
			case VT_UI2:
				pvargDest->uintVal = (unsigned int)pvarSrc->uiVal;
				break;
			case VT_UINT:
			case VT_UI4:
				pvargDest->uintVal = (unsigned int)pvarSrc->ulVal;
				break;
			case VT_UI8:
				pvargDest->uintVal = (unsigned int)pvarSrc->ullVal;
				break;
			case VT_R4:
				pvargDest->uintVal = (unsigned int)pvarSrc->fltVal;
				break;
			case VT_R8:
				pvargDest->uintVal = (unsigned int)pvarSrc->dblVal;
				break;
			case VT_BOOL:
				pvargDest->uintVal = (unsigned int)(pvarSrc->boolVal ? 1 : 0);
				break;
			case VT_DECIMAL:
				pvargDest->uintVal = (unsigned int)SPA::ToDouble(pvarSrc->decVal);
				break;
			case VT_BSTR:
			{
				UINT64 data;
				int res = swscanf(pvarSrc->bstrVal, L"%lld", &data);
				if (res)
					pvargDest->uintVal = (unsigned int)data;
				else
				{
					pvargDest->vt = VT_EMPTY;
					return E_UNEXPECTED;
				}
			}
			break;
			case (VT_I1 | VT_ARRAY) :
			{
				UINT64 data;
				const char *s;
				::SafeArrayAccessData(pvarSrc->parray, (void**)&s);
				int res = sscanf(s, "%llu", &data);
				::SafeArrayUnaccessData(pvarSrc->parray);
				if (res)
					pvargDest->uintVal = (unsigned int)data;
				else
				{
					pvargDest->vt = VT_EMPTY;
					return E_UNEXPECTED;
				}
			}
									break;
			default:
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
			break;
		case VT_UI8:
			switch (pvarSrc->vt) {
			case VT_I1:
				pvargDest->ullVal = (UINT64)pvarSrc->cVal;
				break;
			case VT_I2:
				pvargDest->ullVal = (UINT64)pvarSrc->iVal;
				break;
			case VT_INT:
			case VT_I4:
				pvargDest->ullVal = (UINT64)pvarSrc->intVal;
				break;
			case VT_I8:
				pvargDest->ullVal = (UINT64)pvarSrc->llVal;
				break;
			case VT_UI1:
				pvargDest->ullVal = (UINT64)pvarSrc->bVal;
				break;
			case VT_UI2:
				pvargDest->ullVal = (UINT64)pvarSrc->uiVal;
				break;
			case VT_UINT:
			case VT_UI4:
				pvargDest->ullVal = (UINT64)pvarSrc->ulVal;
				break;
			case VT_UI8:
				pvargDest->ullVal = (UINT64)pvarSrc->ullVal;
				break;
			case VT_R4:
				pvargDest->ullVal = (UINT64)pvarSrc->fltVal;
				break;
			case VT_R8:
				pvargDest->ullVal = (UINT64)pvarSrc->dblVal;
				break;
			case VT_BOOL:
				pvargDest->ullVal = (UINT64)(pvarSrc->boolVal ? 1 : 0);
				break;
			case VT_DECIMAL:
				pvargDest->ullVal = (UINT64)SPA::ToDouble(pvarSrc->decVal);
				break;
			case VT_BSTR:
			{
				UINT64 data;
				int res = swscanf(pvarSrc->bstrVal, L"%llu", &data);
				if (res)
					pvargDest->ullVal = data;
				else
				{
					pvargDest->vt = VT_EMPTY;
					return E_UNEXPECTED;
				}
			}
			break;
			case (VT_I1 | VT_ARRAY) :
			{
				UINT64 data;
				const char *s;
				::SafeArrayAccessData(pvarSrc->parray, (void**)&s);
				int res = sscanf(s, "%llu", &data);
				::SafeArrayUnaccessData(pvarSrc->parray);
				if (res)
					pvargDest->ullVal = data;
				else
				{
					pvargDest->vt = VT_EMPTY;
					return E_UNEXPECTED;
				}
			}
			break;
			default:
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
			break;
		case VT_BSTR:
			break;
		case (VT_I1 | VT_ARRAY) :
			break;
		case VT_DATE:
			break;
		case VT_DECIMAL:
			break;
		case VT_BOOL:
			break;
		default:
			return E_UNEXPECTED;
		}
		return S_OK;
	}
};

int main(int argc, char* argv[]) {
	SPA::UDB::CDBVariant vt("-123.75");
	CComVariant vtDes;
	SPA::VariantChangeType(&vtDes, &vt, 0, VT_R8);



	CConnectionContext cc;
	std::cout << "Remote host: " << std::endl;
	std::getline(std::cin, cc.Host);
	//cc.Host = "localhost";
	cc.Port = 20902;
	cc.UserId = L"root";
	cc.Password = L"Smash123";


	return 0;
}

