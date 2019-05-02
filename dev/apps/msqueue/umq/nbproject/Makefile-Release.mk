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
	${OBJECTDIR}/_ext/1441729126/error_code.o \
	${OBJECTDIR}/_ext/386121535/future.o \
	${OBJECTDIR}/_ext/323364652/once.o \
	${OBJECTDIR}/_ext/323364652/thread.o \
	${OBJECTDIR}/_ext/386121535/tss_null.o \
	${OBJECTDIR}/_ext/932346631/membuffer.o \
	${OBJECTDIR}/_ext/1472/msqueue.o \
	${OBJECTDIR}/_ext/1472/stdafx.o


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
LDLIBSOPTIONS=-L../../../../../Downloads/boost_1_53_0/stage/lib ../../uzip/uzip/dist/Release/GNU-Linux-x86/libuzip.a -lpthread -lrt

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/umq

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/umq: ../../uzip/uzip/dist/Release/GNU-Linux-x86/libuzip.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/umq: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/umq ${OBJECTFILES} ${LDLIBSOPTIONS} -static-libgcc -static-libstdc++

${OBJECTDIR}/_ext/1441729126/error_code.o: ../../../../../Downloads/boost/libs/system/src/error_code.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1441729126
	${RM} $@.d
	$(COMPILE.cc) -O3 -std=c++11 -static-libgcc -static-libstdc++ -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1441729126/error_code.o ../../../../../Downloads/boost/libs/system/src/error_code.cpp

${OBJECTDIR}/_ext/386121535/future.o: ../../../../../Downloads/boost/libs/thread/src/future.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/386121535
	${RM} $@.d
	$(COMPILE.cc) -O3 -std=c++11 -static-libgcc -static-libstdc++ -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/386121535/future.o ../../../../../Downloads/boost/libs/thread/src/future.cpp

${OBJECTDIR}/_ext/323364652/once.o: ../../../../../Downloads/boost/libs/thread/src/pthread/once.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/323364652
	${RM} $@.d
	$(COMPILE.cc) -O3 -std=c++11 -static-libgcc -static-libstdc++ -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/323364652/once.o ../../../../../Downloads/boost/libs/thread/src/pthread/once.cpp

${OBJECTDIR}/_ext/323364652/thread.o: ../../../../../Downloads/boost/libs/thread/src/pthread/thread.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/323364652
	${RM} $@.d
	$(COMPILE.cc) -O3 -std=c++11 -static-libgcc -static-libstdc++ -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/323364652/thread.o ../../../../../Downloads/boost/libs/thread/src/pthread/thread.cpp

${OBJECTDIR}/_ext/386121535/tss_null.o: ../../../../../Downloads/boost/libs/thread/src/tss_null.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/386121535
	${RM} $@.d
	$(COMPILE.cc) -O3 -std=c++11 -static-libgcc -static-libstdc++ -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/386121535/tss_null.o ../../../../../Downloads/boost/libs/thread/src/tss_null.cpp

${OBJECTDIR}/_ext/932346631/membuffer.o: ../../../include/membuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/932346631
	${RM} $@.d
	$(COMPILE.cc) -O3 -std=c++11 -static-libgcc -static-libstdc++ -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/932346631/membuffer.o ../../../include/membuffer.cpp

${OBJECTDIR}/_ext/1472/msqueue.o: ../msqueue.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} $@.d
	$(COMPILE.cc) -O3 -std=c++11 -static-libgcc -static-libstdc++ -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1472/msqueue.o ../msqueue.cpp

${OBJECTDIR}/_ext/1472/stdafx.o: ../stdafx.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} $@.d
	$(COMPILE.cc) -O3 -std=c++11 -static-libgcc -static-libstdc++ -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1472/stdafx.o ../stdafx.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/umq

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
