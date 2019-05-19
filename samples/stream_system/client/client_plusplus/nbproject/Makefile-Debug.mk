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
	${OBJECTDIR}/_ext/562988696/aclientw.o \
	${OBJECTDIR}/_ext/562988696/aserverw.o \
	${OBJECTDIR}/_ext/562988696/membuffer.o \
	${OBJECTDIR}/_ext/562988696/tablecache.o \
	${OBJECTDIR}/client_plusplus.o \
	${OBJECTDIR}/stdafx.o \
	${OBJECTDIR}/webasynchandler.o


# C Compiler Flags
CFLAGS=-std=c++11 -Wall

# CC Compiler Flags
CCFLAGS=-std=c++11 -Wall
CXXFLAGS=-std=c++11 -Wall

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-ldl

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/client_plusplus

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/client_plusplus: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/client_plusplus ${OBJECTFILES} ${LDLIBSOPTIONS} -std=c++11 -lstdc++ -pthread

${OBJECTDIR}/_ext/562988696/aclientw.o: ../../../../include/aclientw.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/562988696
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/562988696/aclientw.o ../../../../include/aclientw.cpp

${OBJECTDIR}/_ext/562988696/aserverw.o: ../../../../include/aserverw.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/562988696
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/562988696/aserverw.o ../../../../include/aserverw.cpp

${OBJECTDIR}/_ext/562988696/membuffer.o: ../../../../include/membuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/562988696
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/562988696/membuffer.o ../../../../include/membuffer.cpp

${OBJECTDIR}/_ext/562988696/tablecache.o: ../../../../include/tablecache.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/562988696
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/562988696/tablecache.o ../../../../include/tablecache.cpp

${OBJECTDIR}/client_plusplus.o: client_plusplus.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/client_plusplus.o client_plusplus.cpp

${OBJECTDIR}/stdafx.o: stdafx.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/stdafx.o stdafx.cpp

${OBJECTDIR}/webasynchandler.o: webasynchandler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/webasynchandler.o webasynchandler.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/client_plusplus

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
