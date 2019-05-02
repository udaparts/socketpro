#ifndef ___SOCKETPRO_DEFINES_MYBLOWFISH_I_H__
#define ___SOCKETPRO_DEFINES_MYBLOWFISH_I_H__

//defines for service CMySecure
#define sidCMySecure	(odUserServiceIDMin + 101)

#define idOpenCMySecure	(odUserRequestIDMin + 0)
#define idBeginTransCMySecure	(idOpenCMySecure + 1)
#define idExecuteNoQueryCMySecure	(idBeginTransCMySecure + 1)
#define idCommitCMySecure	(idExecuteNoQueryCMySecure + 1)



#endif