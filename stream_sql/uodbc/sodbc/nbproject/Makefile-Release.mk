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
	${OBJECTDIR}/_ext/932346631/aserverw.o \
	${OBJECTDIR}/_ext/932346631/membuffer.o \
	${OBJECTDIR}/_ext/1865912263/odbcimpl.o \
	${OBJECTDIR}/_ext/1865912263/sodbc.o \
	${OBJECTDIR}/stdafx.o


# C Compiler Flags
CFLAGS=-std=c++11 -static-libgcc -static-libstdc++

# CC Compiler Flags
CCFLAGS=-std=c++11 -static-libgcc -static-libstdc++
CXXFLAGS=-std=c++11 -static-libgcc -static-libstdc++

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-luservercore -lodbc

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libsodbc.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libsodbc.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	g++ -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libsodbc.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -std=c++11 -static-libgcc -static-libstdc++ -shared -s -fPIC

${OBJECTDIR}/_ext/932346631/aserverw.o: ../../../include/aserverw.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/932346631
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/932346631/aserverw.o ../../../include/aserverw.cpp

${OBJECTDIR}/_ext/932346631/membuffer.o: ../../../include/membuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/932346631
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/932346631/membuffer.o ../../../include/membuffer.cpp

${OBJECTDIR}/_ext/1865912263/odbcimpl.o: ../../../include/odbc/server_impl/odbcimpl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1865912263
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1865912263/odbcimpl.o ../../../include/odbc/server_impl/odbcimpl.cpp

${OBJECTDIR}/_ext/1865912263/sodbc.o: ../../../include/odbc/server_impl/sodbc.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1865912263
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1865912263/sodbc.o ../../../include/odbc/server_impl/sodbc.cpp

${OBJECTDIR}/stdafx.o: stdafx.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/stdafx.o stdafx.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libsodbc.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
