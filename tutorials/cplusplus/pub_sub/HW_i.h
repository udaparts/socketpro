#ifndef ___SOCKETPRO_DEFINES_HW_I_H__
#define ___SOCKETPRO_DEFINES_HW_I_H__

//defines for service HelloWorld
#define sidHelloWorld	(sidReserved + 1)

#define idSayHelloHelloWorld	(SPA::idReservedTwo + 1)
#define idSleepHelloWorld	(idSayHelloHelloWorld + 1)
#define idEchoHelloWorld (idSleepHelloWorld + 1)

static std::string ToString(const unsigned int *groups, int count) {
	int n;
	if (!groups)
		count = 0;
	std::string s = "[";
	for (n = 0; n < count; ++n) {
		if (n != 0)
			s += ", ";
		s += std::to_string((SPA::UINT64)(groups[n]));
		++n;
	}
	s += "]";
	return s;
}

#endif