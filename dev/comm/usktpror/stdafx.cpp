// stdafx.cpp : source file that includes just the standard includes
// usktpror.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

namespace UHTTP {

#ifdef USE_SPIRIT_CLSSICAL_FOR_MULTIPART
    CRule R_CRLF = str_p("\r\n");
    CRule R_HEADER_END = R_CRLF >> R_CRLF;
    CRule R_HEADER = +(anychar_p - (ch_p(':') | space_p | ch_p((char) 0)));
    CRule R_VALUE = +(anychar_p - R_CRLF);
#endif

};