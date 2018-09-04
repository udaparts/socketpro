
#include "../../../include/membuffer.h"

#ifndef MY_STRUCT_DEMO_H
#define MY_STRUCT_DEMO_H

class CMyStruct {
public:

	CMyStruct()
		: NullString(nullptr),
		ADouble(0.0),
		ABool(false) {
			SYSTEMTIME st;
#ifdef WIN32_64
			::GetLocalTime(&st);
#else
			::gettimeofday(&st, nullptr);
#endif
			ADateTime = st;
	}

	CMyStruct(CMyStruct &&ms)
		: NullString(ms.NullString),
		ObjectNull(ms.ObjectNull),
		ADateTime(ms.ADateTime),
		ADouble(ms.ADouble),
		ABool(ms.ABool),
		UnicodeString(std::move(ms.UnicodeString)),
		AsciiString(std::move(ms.AsciiString)),
		ObjBool(ms.ObjBool),
#ifdef WIN32_64
		//ATL CComVariant doesnot support move
		ObjString(ms.ObjString),
		objArrString(ms.objArrString),
		objArrInt(ms.objArrInt)
#else
		ObjString(std::move(ms.ObjString)),
		objArrString(std::move(ms.objArrString)),
		objArrInt(std::move(ms.objArrInt))
#endif
	{
	}

	CMyStruct(const CMyStruct& ms)
		: NullString(ms.NullString),
		ObjectNull(ms.ObjectNull),
		ADateTime(ms.ADateTime),
		ADouble(ms.ADouble),
		ABool(ms.ABool),
		UnicodeString(ms.UnicodeString),
		AsciiString(ms.AsciiString),
		ObjBool(ms.ObjBool),
		ObjString(ms.ObjString),
		objArrString(ms.objArrString),
		objArrInt(ms.objArrInt) {

	}


public:

	CMyStruct& operator=(const CMyStruct& ms) {
		if (this != &ms) {
			NullString = ms.NullString;
			ObjectNull = ms.ObjectNull;
			ADateTime = ms.ADateTime;
			ADouble = ms.ADouble;
			ABool = ms.ABool;
			UnicodeString = ms.UnicodeString;
			AsciiString = ms.AsciiString;
			ObjBool = ms.ObjBool;
			ObjString = ms.ObjString;
			objArrString = ms.objArrString;
			objArrInt = ms.objArrInt;
		}
		return *this;
	}

	CMyStruct& operator=(CMyStruct&& ms) {
		if (this != &ms) {
			NullString = ms.NullString;
			ObjectNull = ms.ObjectNull;
			ADateTime = ms.ADateTime;
			ADouble = ms.ADouble;
			ABool = ms.ABool;
			UnicodeString = std::move(ms.UnicodeString);
			AsciiString = std::move(ms.AsciiString);
			ObjBool = ms.ObjBool;
#ifdef WIN32_64
			//ATL CComVariant doesnot support move
			ObjString = ms.ObjString;
			objArrString = ms.objArrString;
			objArrInt = ms.objArrInt;
#else
			ObjString = std::move(ms.ObjString);
			objArrString = std::move(ms.objArrString);
			objArrInt = std::move(ms.objArrInt);
#endif
		}
		return *this;
	}

	void SaveTo(SPA::CUQueue &q) const {
		q << NullString //4 bytes for length
			<< ObjectNull //2 bytes for data type
			<< ADateTime //8 bytes for unsigned __int64 with accuracy to 1 micro-second
			<< ADouble //8 bytes
			<< ABool //1 byte
			<< UnicodeString //4 bytes for string length + (length * 2) bytes for string data -- UTF16-lowendian
			<< AsciiString //4 bytes for ASCII string length + length bytes for string data
			<< ObjBool //2 bytes for data type + 2 bytes for variant bool
			<< ObjString //2 bytes for data type + 4 bytes for string length + (length * 2) bytes for string data -- UTF16-lowendian
			<< objArrString //2 bytes for data type + 4 bytes for array size + (4 bytes for string length + (length * 2) bytes for string data) * arraysize -- UTF16-lowendian
			<< objArrInt //2 bytes for data type + 4 bytes for array size + arraysize * 4 bytes for int data
			;
	}

	void LoadFrom(SPA::CUQueue &q) {
		unsigned int NullStringLen;
		q >> NullStringLen
			>> ObjectNull
			>> ADateTime
			>> ADouble
			>> ABool
			>> UnicodeString //UTF16-lowendian
			>> AsciiString
			>> ObjBool
			>> ObjString //UTF16-lowendian
			>> objArrString //UTF16-lowendian
			>> objArrInt
			;
		assert(NullStringLen == (~0));
	}

public:

	bool operator ==(const CMyStruct &ms) {
		return NullString == ms.NullString &&
			ObjectNull == ms.ObjectNull &&
			ADouble == ms.ADouble &&
			ABool == ms.ABool &&
			UnicodeString == ms.UnicodeString &&
			AsciiString == ms.AsciiString &&
			ADateTime == ms.ADateTime &&
			ObjBool == ms.ObjBool &&
			ObjString == ms.ObjString &&
#ifdef WIN32_64				
			SPA::IsEqual(objArrString, ms.objArrString) &&
			SPA::IsEqual(objArrInt, ms.objArrInt)
#else
			objArrString == ms.objArrString &&
			objArrInt == ms.objArrInt
#endif
			;
	}

private:
	wchar_t *NullString;
	SPA::UVariant ObjectNull;
	
public:
	SPA::UDateTime ADateTime;
	double ADouble;
	bool ABool;
	std::wstring UnicodeString;
	std::string AsciiString;
	SPA::UVariant ObjBool;
	SPA::UVariant ObjString;
	SPA::UVariant objArrString;
	SPA::UVariant objArrInt;
};

SPA::CUQueue& operator <<(SPA::CUQueue &mc, const CMyStruct &ms);
SPA::CUQueue& operator >>(SPA::CUQueue &mc, CMyStruct &ms);

void SetMyStruct(CMyStruct &ms);

#endif
