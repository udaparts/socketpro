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
	${OBJECTDIR}/_ext/932346631/aserverw.o \
	${OBJECTDIR}/_ext/932346631/membuffer.o \
	${OBJECTDIR}/_ext/1472/sproserver.o \
	${OBJECTDIR}/_ext/1472/stdafx.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-std=c++11
CXXFLAGS=-std=c++11

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-ldl -lpthread

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/userver

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/userver: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/userver ${OBJECTFILES} ${LDLIBSOPTIONS} -s

${OBJECTDIR}/_ext/1441729126/error_code.o: ../../../../../Downloads/boost/libs/system/src/error_code.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1441729126
	$(COMPILE.cc) -O2 -s -DNDEBUG -o ${OBJECTDIR}/_ext/1441729126/error_code.o ../../../../../Downloads/boost/libs/system/src/error_code.cpp

${OBJECTDIR}/_ext/932346631/aserverw.o: ../../../include/aserverw.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/932346631
	$(COMPILE.cc) -O2 -s -DNDEBUG -o ${OBJECTDIR}/_ext/932346631/aserverw.o ../../../include/aserverw.cpp

${OBJECTDIR}/_ext/932346631/membuffer.o: ../../../include/membuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/932346631
	$(COMPILE.cc) -O2 -s -DNDEBUG -o ${OBJECTDIR}/_ext/932346631/membuffer.o ../../../include/membuffer.cpp

${OBJECTDIR}/_ext/1472/sproserver.o: ../sproserver.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	$(COMPILE.cc) -O2 -s -DNDEBUG -o ${OBJECTDIR}/_ext/1472/sproserver.o ../sproserver.cpp

${OBJECTDIR}/_ext/1472/stdafx.o: ../stdafx.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	$(COMPILE.cc) -O2 -s -DNDEBUG -o ${OBJECTDIR}/_ext/1472/stdafx.o ../stdafx.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/userver

# Subprojects
.clean-subprojects:
