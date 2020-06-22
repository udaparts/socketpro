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
	${OBJECTDIR}/_ext/1165644825/mysqlimpl.o \
	${OBJECTDIR}/_ext/1165644825/smysql.o


# C Compiler Flags
CFLAGS=-std=c++11 -static-libstdc++ -Os

# CC Compiler Flags
CCFLAGS=-std=c++11 -static-libstdc++ -Os
CXXFLAGS=-std=c++11 -static-libstdc++ -Os

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-luservercore

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libsmysql.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libsmysql.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	g++ -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libsmysql.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -std=c++11 -shared -s -fPIC

${OBJECTDIR}/_ext/932346631/aserverw.o: ../../../include/aserverw.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/932346631
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_BOOST_LARGE_INTEGER_FOR_DECIMAL -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/932346631/aserverw.o ../../../include/aserverw.cpp

${OBJECTDIR}/_ext/932346631/membuffer.o: ../../../include/membuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/932346631
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_BOOST_LARGE_INTEGER_FOR_DECIMAL -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/932346631/membuffer.o ../../../include/membuffer.cpp

${OBJECTDIR}/_ext/1165644825/mysqlimpl.o: ../../../include/mysql/server_impl/mysqlimpl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1165644825
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_BOOST_LARGE_INTEGER_FOR_DECIMAL -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1165644825/mysqlimpl.o ../../../include/mysql/server_impl/mysqlimpl.cpp

${OBJECTDIR}/_ext/1165644825/smysql.o: ../../../include/mysql/server_impl/smysql.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1165644825
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DNDEBUG -DUSE_BOOST_LARGE_INTEGER_FOR_DECIMAL -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1165644825/smysql.o ../../../include/mysql/server_impl/smysql.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libsmysql.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
