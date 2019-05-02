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
	${OBJECTDIR}/_ext/672919334/chrono.o \
	${OBJECTDIR}/_ext/1115299385/appsettings.o \
	${OBJECTDIR}/_ext/1115299385/chat.o \
	${OBJECTDIR}/_ext/1115299385/connectioncontext.o \
	${OBJECTDIR}/_ext/1115299385/httpcontext.o \
	${OBJECTDIR}/_ext/1115299385/httpgrammar.o \
	${OBJECTDIR}/_ext/1115299385/jsloader.o \
	${OBJECTDIR}/_ext/1115299385/multipartgrammar.o \
	${OBJECTDIR}/_ext/1115299385/stdafx.o \
	${OBJECTDIR}/_ext/1115299385/webrequestprocessor.o \
	${OBJECTDIR}/_ext/1115299385/webresponseprocessor.o \
	${OBJECTDIR}/_ext/932346631/membuffer.o \
	${OBJECTDIR}/_ext/1027374270/listimpl.o \
	${OBJECTDIR}/_ext/1027374270/resindeximpl.o \
	${OBJECTDIR}/_ext/1027374270/serverthread.o \
	${OBJECTDIR}/_ext/1027374270/servicecontext.o \
	${OBJECTDIR}/_ext/1027374270/svrimpl.o \
	${OBJECTDIR}/_ext/877975035/includes.o \
	${OBJECTDIR}/_ext/877975035/myopenssl.o \
	${OBJECTDIR}/_ext/877975035/ucertimpl.o \
	${OBJECTDIR}/_ext/877975035/uthread.o \
	${OBJECTDIR}/_ext/1472/server.o \
	${OBJECTDIR}/_ext/1472/servercore.o \
	${OBJECTDIR}/_ext/1472/session.o \
	${OBJECTDIR}/_ext/1472/ucommexception.o


# C Compiler Flags
CFLAGS=-fPIC -static-libgcc -static-libstdc++ -std=c++11

# CC Compiler Flags
CCFLAGS=-fPIC -std=c++11 -static-libgcc -static-libstdc++
CXXFLAGS=-fPIC -std=c++11 -static-libgcc -static-libstdc++

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lssl ../../../apps/uzip/uzip/dist/Release/GNU-Linux-x86/libuzip.a -lpthread -lrt `pkg-config --libs libcrypto` -ldl   

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libuservercore.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libuservercore.${CND_DLIB_EXT}: ../../../apps/uzip/uzip/dist/Release/GNU-Linux-x86/libuzip.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libuservercore.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libuservercore.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -s -fPIC

${OBJECTDIR}/_ext/672919334/chrono.o: ../../../../../Downloads/boost/libs/chrono/src/chrono.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/672919334
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/672919334/chrono.o ../../../../../Downloads/boost/libs/chrono/src/chrono.cpp

${OBJECTDIR}/_ext/1115299385/appsettings.o: ../../../apps/hparser/appsettings.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1115299385
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1115299385/appsettings.o ../../../apps/hparser/appsettings.cpp

${OBJECTDIR}/_ext/1115299385/chat.o: ../../../apps/hparser/chat.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1115299385
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1115299385/chat.o ../../../apps/hparser/chat.cpp

${OBJECTDIR}/_ext/1115299385/connectioncontext.o: ../../../apps/hparser/connectioncontext.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1115299385
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1115299385/connectioncontext.o ../../../apps/hparser/connectioncontext.cpp

${OBJECTDIR}/_ext/1115299385/httpcontext.o: ../../../apps/hparser/httpcontext.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1115299385
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1115299385/httpcontext.o ../../../apps/hparser/httpcontext.cpp

${OBJECTDIR}/_ext/1115299385/httpgrammar.o: ../../../apps/hparser/httpgrammar.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1115299385
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1115299385/httpgrammar.o ../../../apps/hparser/httpgrammar.cpp

${OBJECTDIR}/_ext/1115299385/jsloader.o: ../../../apps/hparser/jsloader.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1115299385
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1115299385/jsloader.o ../../../apps/hparser/jsloader.cpp

${OBJECTDIR}/_ext/1115299385/multipartgrammar.o: ../../../apps/hparser/multipartgrammar.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1115299385
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1115299385/multipartgrammar.o ../../../apps/hparser/multipartgrammar.cpp

${OBJECTDIR}/_ext/1115299385/stdafx.o: ../../../apps/hparser/stdafx.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1115299385
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1115299385/stdafx.o ../../../apps/hparser/stdafx.cpp

${OBJECTDIR}/_ext/1115299385/webrequestprocessor.o: ../../../apps/hparser/webrequestprocessor.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1115299385
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1115299385/webrequestprocessor.o ../../../apps/hparser/webrequestprocessor.cpp

${OBJECTDIR}/_ext/1115299385/webresponseprocessor.o: ../../../apps/hparser/webresponseprocessor.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1115299385
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1115299385/webresponseprocessor.o ../../../apps/hparser/webresponseprocessor.cpp

${OBJECTDIR}/_ext/932346631/membuffer.o: ../../../include/membuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/932346631
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/932346631/membuffer.o ../../../include/membuffer.cpp

${OBJECTDIR}/_ext/1027374270/listimpl.o: ../../ServerCore/listimpl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1027374270
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1027374270/listimpl.o ../../ServerCore/listimpl.cpp

${OBJECTDIR}/_ext/1027374270/resindeximpl.o: ../../ServerCore/resindeximpl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1027374270
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1027374270/resindeximpl.o ../../ServerCore/resindeximpl.cpp

${OBJECTDIR}/_ext/1027374270/serverthread.o: ../../ServerCore/serverthread.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1027374270
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1027374270/serverthread.o ../../ServerCore/serverthread.cpp

${OBJECTDIR}/_ext/1027374270/servicecontext.o: ../../ServerCore/servicecontext.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1027374270
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1027374270/servicecontext.o ../../ServerCore/servicecontext.cpp

${OBJECTDIR}/_ext/1027374270/svrimpl.o: ../../ServerCore/svrimpl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1027374270
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1027374270/svrimpl.o ../../ServerCore/svrimpl.cpp

${OBJECTDIR}/_ext/877975035/includes.o: ../../shared/includes.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/877975035
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/877975035/includes.o ../../shared/includes.cpp

${OBJECTDIR}/_ext/877975035/myopenssl.o: ../../shared/myopenssl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/877975035
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/877975035/myopenssl.o ../../shared/myopenssl.cpp

${OBJECTDIR}/_ext/877975035/ucertimpl.o: ../../shared/ucertimpl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/877975035
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/877975035/ucertimpl.o ../../shared/ucertimpl.cpp

${OBJECTDIR}/_ext/877975035/uthread.o: ../../shared/uthread.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/877975035
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/877975035/uthread.o ../../shared/uthread.cpp

${OBJECTDIR}/_ext/1472/server.o: ../server.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/server.o ../server.cpp

${OBJECTDIR}/_ext/1472/servercore.o: ../servercore.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/servercore.o ../servercore.cpp

${OBJECTDIR}/_ext/1472/session.o: ../session.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/session.o ../session.cpp

${OBJECTDIR}/_ext/1472/ucommexception.o: ../ucommexception.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -s -DNDEBUG -DUSE_SPIRIT_CLSSICAL_FOR_MULTIPART `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/ucommexception.o ../ucommexception.cpp

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
