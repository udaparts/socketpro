using System;
using System.Runtime.InteropServices;

namespace SocketProAdapter
{
	public enum tagDefinedWindowMessage : ushort
	{
		wmUSER = 0x0400,
		//your window message can NOT be in the following range
		WM_SOCKETPRO_RESERVED_MIN	= (wmUSER + 0x0401),
		WM_SOCKETPRO_RESERVED_MAX	= (wmUSER + 0x06FF),
		WM_SOCKET_SVR_NOTIFY = (wmUSER + 0x0480),
		WM_ASK_FOR_PROCESSING = (wmUSER + 0x0500),
		WM_REQUEST_PROCESSED = (wmUSER+ 0x0501),
		WM_WORKER_THREAD_DYING = (wmUSER + 0x0502),
		WM_SET_PROCESSING_FUNC = (wmUSER + 0x0503),
		WM_SET_PRETRANS_FUNC = (wmUSER + 0x0504),
		WM_CANCEL_REQUEST = (wmUSER + 0x0505),
		WM_CONTINUE_PROCESSING = (wmUSER + 0x0506),
	}
	public enum tagSockDataType : ushort
	{
		sdVT_EMPTY	= 0,
		sdVT_NULL 	= 1,
		sdVT_I2		= 2,
		sdVT_I4		= 3,
		sdVT_R4		= 4,
		sdVT_R8		= 5,
		sdVT_CY		= 6,
		sdVT_BSTR	= 8,
		sdVT_DATE	= 7,
		sdVT_BOOL	= 11,
		sdVT_VARIANT	= 12,
		sdVT_DECIMAL	= 14,
		sdVT_I1		= 16,
		sdVT_UI1	= 17,
		sdVT_UI2	= 18,
		sdVT_UI4	= 19,
		sdVT_I8		= 20,
		sdVT_UI8	= 21,
		sdVT_FILETIME 	= 64,
		sdVT_CLSID	= 72,
		sdVT_BYTES	= 128,
		sdVT_STR	= 129,
		sdVT_WSTR	= 130,
        sdVT_USERIALIZER_OBJECT = 0xD00,
		sdVT_NETObject 	= 0xE00,
        sdVT_TIMESPAN = 0xC00,
        sdVT_DATETIMEOFFSET = 0xB00,
		sdVT_ARRAY	= 0x2000,
	};
}
