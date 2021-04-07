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
	${OBJECTDIR}/_ext/1761269253/once.o \
	${OBJECTDIR}/_ext/1761269253/thread.o \
	${OBJECTDIR}/_ext/53904016/tss_null.o \
	${OBJECTDIR}/_ext/1719658846/includes.o \
	${OBJECTDIR}/_ext/1719658846/myopenssl.o \
	${OBJECTDIR}/_ext/1719658846/ucertimpl.o \
	${OBJECTDIR}/_ext/1719658846/uthread.o \
	${OBJECTDIR}/_ext/262068057/membuffer.o \
	${OBJECTDIR}/_ext/760551045/getsysid.o \
	${OBJECTDIR}/_ext/760551045/mqfile.o \
	${OBJECTDIR}/_ext/760551045/sha1.o \
	${OBJECTDIR}/pch.o \
	${OBJECTDIR}/rawsession.o \
	${OBJECTDIR}/rawthread.o \
	${OBJECTDIR}/urawsocket.o


# C Compiler Flags
CFLAGS=-std=c++11 -static -fPIC

# CC Compiler Flags
CCFLAGS=-std=c++11 -static -fPIC
CXXFLAGS=-std=c++11 -static -fPIC

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liburawsocket.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liburawsocket.a: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liburawsocket.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liburawsocket.a ${OBJECTFILES} 
	$(RANLIB) ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liburawsocket.a

${OBJECTDIR}/_ext/1761269253/once.o: ../../../../Downloads/boost/libs/thread/src/pthread/once.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1761269253
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -DU_RAW_SOCKET -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1761269253/once.o ../../../../Downloads/boost/libs/thread/src/pthread/once.cpp

${OBJECTDIR}/_ext/1761269253/thread.o: ../../../../Downloads/boost/libs/thread/src/pthread/thread.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1761269253
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -DU_RAW_SOCKET -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1761269253/thread.o ../../../../Downloads/boost/libs/thread/src/pthread/thread.cpp

${OBJECTDIR}/_ext/53904016/tss_null.o: ../../../../Downloads/boost/libs/thread/src/tss_null.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/53904016
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -DU_RAW_SOCKET -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/53904016/tss_null.o ../../../../Downloads/boost/libs/thread/src/tss_null.cpp

${OBJECTDIR}/_ext/1719658846/includes.o: ../core_shared/shared/includes.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1719658846
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -DU_RAW_SOCKET -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1719658846/includes.o ../core_shared/shared/includes.cpp

${OBJECTDIR}/_ext/1719658846/myopenssl.o: ../core_shared/shared/myopenssl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1719658846
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -DU_RAW_SOCKET -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1719658846/myopenssl.o ../core_shared/shared/myopenssl.cpp

${OBJECTDIR}/_ext/1719658846/ucertimpl.o: ../core_shared/shared/ucertimpl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1719658846
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -DU_RAW_SOCKET -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1719658846/ucertimpl.o ../core_shared/shared/ucertimpl.cpp

${OBJECTDIR}/_ext/1719658846/uthread.o: ../core_shared/shared/uthread.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1719658846
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -DU_RAW_SOCKET -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1719658846/uthread.o ../core_shared/shared/uthread.cpp

${OBJECTDIR}/_ext/262068057/membuffer.o: ../include/membuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/262068057
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -DU_RAW_SOCKET -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/262068057/membuffer.o ../include/membuffer.cpp

${OBJECTDIR}/_ext/760551045/getsysid.o: ../uzip/getsysid.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/760551045
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -DU_RAW_SOCKET -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/760551045/getsysid.o ../uzip/getsysid.cpp

${OBJECTDIR}/_ext/760551045/mqfile.o: ../uzip/mqfile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/760551045
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -DU_RAW_SOCKET -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/760551045/mqfile.o ../uzip/mqfile.cpp

${OBJECTDIR}/_ext/760551045/sha1.o: ../uzip/sha1.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/760551045
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -DU_RAW_SOCKET -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/760551045/sha1.o ../uzip/sha1.cpp

${OBJECTDIR}/pch.o: pch.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -DU_RAW_SOCKET -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/pch.o pch.cpp

${OBJECTDIR}/rawsession.o: rawsession.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -DU_RAW_SOCKET -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/rawsession.o rawsession.cpp

${OBJECTDIR}/rawthread.o: rawthread.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -DU_RAW_SOCKET -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/rawthread.o rawthread.cpp

${OBJECTDIR}/urawsocket.o: urawsocket.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -DU_RAW_SOCKET -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/urawsocket.o urawsocket.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liburawsocket.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
