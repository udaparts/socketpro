#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/1719658846/includes.o \
	${OBJECTDIR}/_ext/1719658846/myopenssl.o \
	${OBJECTDIR}/_ext/1719658846/ucertimpl.o \
	${OBJECTDIR}/_ext/1719658846/uthread.o \
	${OBJECTDIR}/_ext/262068057/membuffer.o \
	${OBJECTDIR}/_ext/648362257/connectioncontext.o \
	${OBJECTDIR}/_ext/648362257/httpcontext.o \
	${OBJECTDIR}/_ext/648362257/httpgrammar.o \
	${OBJECTDIR}/_ext/648362257/jsloader.o \
	${OBJECTDIR}/_ext/648362257/listimpl.o \
	${OBJECTDIR}/_ext/648362257/multipartgrammar.o \
	${OBJECTDIR}/_ext/648362257/resindeximpl.o \
	${OBJECTDIR}/_ext/648362257/serverthread.o \
	${OBJECTDIR}/_ext/648362257/servicecontext.o \
	${OBJECTDIR}/_ext/648362257/svrimpl.o \
	${OBJECTDIR}/_ext/648362257/webrequestprocessor.o \
	${OBJECTDIR}/_ext/648362257/webresponseprocessor.o \
	${OBJECTDIR}/server.o \
	${OBJECTDIR}/session.o


# C Compiler Flags
CFLAGS=-std=c++11

# CC Compiler Flags
CCFLAGS=-std=c++11
CXXFLAGS=-std=c++11

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lpthread -ldl ../uzip/dist/Release/GNU-Linux-x86/libuzip.a `pkg-config --libs libcrypto` `pkg-config --libs libssl` -lm  /usr/lib/gcc/x86_64-linux-gnu/4.8.4/libstdc++.a  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libuservercore.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libuservercore.${CND_DLIB_EXT}: ../uzip/dist/Release/GNU-Linux-x86/libuzip.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libuservercore.${CND_DLIB_EXT}: /usr/lib/gcc/x86_64-linux-gnu/4.8.4/libstdc++.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libuservercore.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libuservercore.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -Wl,--no-undefined -shared -s -fPIC

${OBJECTDIR}/_ext/1719658846/includes.o: ../core_shared/shared/includes.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1719658846
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto` `pkg-config --cflags libssl`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1719658846/includes.o ../core_shared/shared/includes.cpp

${OBJECTDIR}/_ext/1719658846/myopenssl.o: ../core_shared/shared/myopenssl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1719658846
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto` `pkg-config --cflags libssl`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1719658846/myopenssl.o ../core_shared/shared/myopenssl.cpp

${OBJECTDIR}/_ext/1719658846/ucertimpl.o: ../core_shared/shared/ucertimpl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1719658846
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto` `pkg-config --cflags libssl`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1719658846/ucertimpl.o ../core_shared/shared/ucertimpl.cpp

${OBJECTDIR}/_ext/1719658846/uthread.o: ../core_shared/shared/uthread.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1719658846
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto` `pkg-config --cflags libssl`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1719658846/uthread.o ../core_shared/shared/uthread.cpp

${OBJECTDIR}/_ext/262068057/membuffer.o: ../include/membuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/262068057
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto` `pkg-config --cflags libssl`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/262068057/membuffer.o ../include/membuffer.cpp

${OBJECTDIR}/_ext/648362257/connectioncontext.o: ../servercore/connectioncontext.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/648362257
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto` `pkg-config --cflags libssl`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/648362257/connectioncontext.o ../servercore/connectioncontext.cpp

${OBJECTDIR}/_ext/648362257/httpcontext.o: ../servercore/httpcontext.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/648362257
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto` `pkg-config --cflags libssl`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/648362257/httpcontext.o ../servercore/httpcontext.cpp

${OBJECTDIR}/_ext/648362257/httpgrammar.o: ../servercore/httpgrammar.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/648362257
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto` `pkg-config --cflags libssl`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/648362257/httpgrammar.o ../servercore/httpgrammar.cpp

${OBJECTDIR}/_ext/648362257/jsloader.o: ../servercore/jsloader.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/648362257
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto` `pkg-config --cflags libssl`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/648362257/jsloader.o ../servercore/jsloader.cpp

${OBJECTDIR}/_ext/648362257/listimpl.o: ../servercore/listimpl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/648362257
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto` `pkg-config --cflags libssl`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/648362257/listimpl.o ../servercore/listimpl.cpp

${OBJECTDIR}/_ext/648362257/multipartgrammar.o: ../servercore/multipartgrammar.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/648362257
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto` `pkg-config --cflags libssl`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/648362257/multipartgrammar.o ../servercore/multipartgrammar.cpp

${OBJECTDIR}/_ext/648362257/resindeximpl.o: ../servercore/resindeximpl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/648362257
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto` `pkg-config --cflags libssl`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/648362257/resindeximpl.o ../servercore/resindeximpl.cpp

${OBJECTDIR}/_ext/648362257/serverthread.o: ../servercore/serverthread.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/648362257
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto` `pkg-config --cflags libssl`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/648362257/serverthread.o ../servercore/serverthread.cpp

${OBJECTDIR}/_ext/648362257/servicecontext.o: ../servercore/servicecontext.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/648362257
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto` `pkg-config --cflags libssl`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/648362257/servicecontext.o ../servercore/servicecontext.cpp

${OBJECTDIR}/_ext/648362257/svrimpl.o: ../servercore/svrimpl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/648362257
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto` `pkg-config --cflags libssl`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/648362257/svrimpl.o ../servercore/svrimpl.cpp

${OBJECTDIR}/_ext/648362257/webrequestprocessor.o: ../servercore/webrequestprocessor.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/648362257
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto` `pkg-config --cflags libssl`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/648362257/webrequestprocessor.o ../servercore/webrequestprocessor.cpp

${OBJECTDIR}/_ext/648362257/webresponseprocessor.o: ../servercore/webresponseprocessor.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/648362257
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto` `pkg-config --cflags libssl`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/648362257/webresponseprocessor.o ../servercore/webresponseprocessor.cpp

${OBJECTDIR}/server.o: server.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto` `pkg-config --cflags libssl`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/server.o server.cpp

${OBJECTDIR}/session.o: session.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto` `pkg-config --cflags libssl`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/session.o session.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libuservercore.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
