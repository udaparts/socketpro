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
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/932346631/membuffer.o \
	${OBJECTDIR}/_ext/877975035/includes.o \
	${OBJECTDIR}/_ext/877975035/uthread.o \
	${OBJECTDIR}/_ext/1472/clientcore.o \
	${OBJECTDIR}/_ext/1472/clientglobal.o \
	${OBJECTDIR}/_ext/1472/clientsession.o \
	${OBJECTDIR}/_ext/1472/clientthread.o \
	${OBJECTDIR}/_ext/1472/csocketimpl.o \
	${OBJECTDIR}/_ext/1472/socketpool.o \
	${OBJECTDIR}/_ext/1472/stdafx.o \
	${OBJECTDIR}/_ext/1472/ucertimpl.o


# C Compiler Flags
CFLAGS=-static-libgcc -static-libstdc++

# CC Compiler Flags
CCFLAGS=-std=c++11 -static-libgcc -static-libstdc++
CXXFLAGS=-std=c++11 -static-libgcc -static-libstdc++

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lssl ../../../apps/uzip/uzip/dist/Debug/GNU-Linux-x86/libuzip.a -ldl `pkg-config --libs libcrypto` -lpthread  -lrt  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libusocket.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libusocket.${CND_DLIB_EXT}: ../../../apps/uzip/uzip/dist/Debug/GNU-Linux-x86/libuzip.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libusocket.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libusocket.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -z defs -rdynamic -shared -fPIC

${OBJECTDIR}/_ext/932346631/membuffer.o: ../../../include/membuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/932346631
	${RM} "$@.d"
	$(COMPILE.cc) -g `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/932346631/membuffer.o ../../../include/membuffer.cpp

${OBJECTDIR}/_ext/877975035/includes.o: ../../shared/includes.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/877975035
	${RM} "$@.d"
	$(COMPILE.cc) -g `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/877975035/includes.o ../../shared/includes.cpp

${OBJECTDIR}/_ext/877975035/uthread.o: ../../shared/uthread.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/877975035
	${RM} "$@.d"
	$(COMPILE.cc) -g `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/877975035/uthread.o ../../shared/uthread.cpp

${OBJECTDIR}/_ext/1472/clientcore.o: ../clientcore.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.cc) -g `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/clientcore.o ../clientcore.cpp

${OBJECTDIR}/_ext/1472/clientglobal.o: ../clientglobal.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.cc) -g `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/clientglobal.o ../clientglobal.cpp

${OBJECTDIR}/_ext/1472/clientsession.o: ../clientsession.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.cc) -g `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/clientsession.o ../clientsession.cpp

${OBJECTDIR}/_ext/1472/clientthread.o: ../clientthread.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.cc) -g `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/clientthread.o ../clientthread.cpp

${OBJECTDIR}/_ext/1472/csocketimpl.o: ../csocketimpl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.cc) -g `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/csocketimpl.o ../csocketimpl.cpp

${OBJECTDIR}/_ext/1472/socketpool.o: ../socketpool.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.cc) -g `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/socketpool.o ../socketpool.cpp

${OBJECTDIR}/_ext/1472/stdafx.o: ../stdafx.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.cc) -g `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/stdafx.o ../stdafx.cpp

${OBJECTDIR}/_ext/1472/ucertimpl.o: ../ucertimpl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.cc) -g `pkg-config --cflags libcrypto`   -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/ucertimpl.o ../ucertimpl.cpp

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
