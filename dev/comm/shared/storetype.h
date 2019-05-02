

#ifndef __UDAPARTS_SOFTWARE_CERTIFICATE_STORE_TYPE_H_
#define __UDAPARTS_SOFTWARE_CERTIFICATE_STORE_TYPE_H_

namespace SPA {

	enum tagCertStoreType
	{
		cstUnknown = 0,
		cstRoot = 1,
		cstCa = 2,
		cstMy = 3,
		cstSpc = 4,
		cstCertFile = 5,
		cstPfx = 6,
	};

} //namespace SC

#endif