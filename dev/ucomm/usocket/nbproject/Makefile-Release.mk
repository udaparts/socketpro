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
	${OBJECTDIR}/_ext/718243431/clientglobal.o \
	${OBJECTDIR}/_ext/718243431/clientthread.o \
	${OBJECTDIR}/_ext/718243431/csocketimpl.o \
	${OBJECTDIR}/_ext/718243431/socketpool.o \
	${OBJECTDIR}/_ext/1719658846/includes.o \
	${OBJECTDIR}/_ext/1719658846/ucertimpl.o \
	${OBJECTDIR}/_ext/1719658846/uthread.o \
	${OBJECTDIR}/_ext/262068057/membuffer.o \
	${OBJECTDIR}/clientsession.o


# C Compiler Flags
CFLAGS=-static-libgcc -static-libstdc++ -std=c++11

# CC Compiler Flags
CCFLAGS=-static-libgcc -static-libstdc++ -std=c++11
CXXFLAGS=-static-libgcc -static-libstdc++ -std=c++11

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lssl ../uzip/dist/Release/GNU-Linux-x86/libuzip.a -ldl -lpthread `pkg-config --libs libcrypto`  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libusocket.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libusocket.${CND_DLIB_EXT}: ../uzip/dist/Release/GNU-Linux-x86/libuzip.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libusocket.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	g++ -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libusocket.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -z defs -rdynamic -shared -s -fPIC

${OBJECTDIR}/_ext/718243431/clientglobal.o: ../clientcore/clientglobal.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/718243431
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/718243431/clientglobal.o ../clientcore/clientglobal.cpp

${OBJECTDIR}/_ext/718243431/clientthread.o: ../clientcore/clientthread.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/718243431
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/718243431/clientthread.o ../clientcore/clientthread.cpp

${OBJECTDIR}/_ext/718243431/csocketimpl.o: ../clientcore/csocketimpl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/718243431
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/718243431/csocketimpl.o ../clientcore/csocketimpl.cpp

${OBJECTDIR}/_ext/718243431/socketpool.o: ../clientcore/socketpool.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/718243431
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/718243431/socketpool.o ../clientcore/socketpool.cpp

${OBJECTDIR}/_ext/1719658846/includes.o: ../core_shared/shared/includes.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1719658846
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1719658846/includes.o ../core_shared/shared/includes.cpp

${OBJECTDIR}/_ext/1719658846/ucertimpl.o: ../core_shared/shared/ucertimpl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1719658846
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1719658846/ucertimpl.o ../core_shared/shared/ucertimpl.cpp

${OBJECTDIR}/_ext/1719658846/uthread.o: ../core_shared/shared/uthread.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1719658846
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1719658846/uthread.o ../core_shared/shared/uthread.cpp

${OBJECTDIR}/_ext/262068057/membuffer.o: ../include/membuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/262068057
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/262068057/membuffer.o ../include/membuffer.cpp

${OBJECTDIR}/clientsession.o: clientsession.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/clientsession.o clientsession.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libusocket.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
