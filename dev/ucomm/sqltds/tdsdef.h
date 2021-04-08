#ifndef _H_SQL_TDS_DEFINES_H_
#define _H_SQL_TDS_DEFINES_H_

namespace tds {

	enum class tagRequest : unsigned char {
		rBatch = 0x01,
		rRpc = 0x03,
		rAttention = 0x06,
		rBulk = 0x07,
		rFederated = 0x08,
		rTransManager = 0x0E,
		rLogin7 = 0x10,
		rSspi = 0x11,
		rPrelogin = 0x12
	};


};


#endif
