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
	${OBJECTDIR}/_ext/932346631/aclientw.o \
	${OBJECTDIR}/_ext/932346631/membuffer.o \
	${OBJECTDIR}/_ext/932346631/tablecache.o \
	${OBJECTDIR}/basehandler.o \
	${OBJECTDIR}/dllmain.o \
	${OBJECTDIR}/phpadapter.o \
	${OBJECTDIR}/phpbuffer.o \
	${OBJECTDIR}/phpcert.o \
	${OBJECTDIR}/phpclientqueue.o \
	${OBJECTDIR}/phpconncontext.o \
	${OBJECTDIR}/phpdataset.o \
	${OBJECTDIR}/phpdb.o \
	${OBJECTDIR}/phpdbcolumninfo.o \
	${OBJECTDIR}/phpdbparaminfo.o \
	${OBJECTDIR}/phpfile.o \
	${OBJECTDIR}/phpmanager.o \
	${OBJECTDIR}/phppush.o \
	${OBJECTDIR}/phpqueue.o \
	${OBJECTDIR}/phpsocket.o \
	${OBJECTDIR}/phpsocketpool.o \
	${OBJECTDIR}/phptable.o \
	${OBJECTDIR}/poolstartcontext.o \
	${OBJECTDIR}/roothandler.o \
	${OBJECTDIR}/spa_consts.o \
	${OBJECTDIR}/spa_cs_consts.o \
	${OBJECTDIR}/stdafx.o


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
LDLIBSOPTIONS=-lpthread -ldl

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libphpadapter.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libphpadapter.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libphpadapter.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -std=c++11 -shared -fPIC

${OBJECTDIR}/_ext/932346631/aclientw.o: ../../../include/aclientw.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/932346631
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/932346631/aclientw.o ../../../include/aclientw.cpp

${OBJECTDIR}/_ext/932346631/membuffer.o: ../../../include/membuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/932346631
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/932346631/membuffer.o ../../../include/membuffer.cpp

${OBJECTDIR}/_ext/932346631/tablecache.o: ../../../include/tablecache.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/932346631
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/932346631/tablecache.o ../../../include/tablecache.cpp

${OBJECTDIR}/basehandler.o: basehandler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/basehandler.o basehandler.cpp

${OBJECTDIR}/dllmain.o: dllmain.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/dllmain.o dllmain.cpp

${OBJECTDIR}/phpadapter.o: phpadapter.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/phpadapter.o phpadapter.cpp

${OBJECTDIR}/phpbuffer.o: phpbuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/phpbuffer.o phpbuffer.cpp

${OBJECTDIR}/phpcert.o: phpcert.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/phpcert.o phpcert.cpp

${OBJECTDIR}/phpclientqueue.o: phpclientqueue.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/phpclientqueue.o phpclientqueue.cpp

${OBJECTDIR}/phpconncontext.o: phpconncontext.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/phpconncontext.o phpconncontext.cpp

${OBJECTDIR}/phpdataset.o: phpdataset.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/phpdataset.o phpdataset.cpp

${OBJECTDIR}/phpdb.o: phpdb.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/phpdb.o phpdb.cpp

${OBJECTDIR}/phpdbcolumninfo.o: phpdbcolumninfo.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/phpdbcolumninfo.o phpdbcolumninfo.cpp

${OBJECTDIR}/phpdbparaminfo.o: phpdbparaminfo.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/phpdbparaminfo.o phpdbparaminfo.cpp

${OBJECTDIR}/phpfile.o: phpfile.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/phpfile.o phpfile.cpp

${OBJECTDIR}/phpmanager.o: phpmanager.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/phpmanager.o phpmanager.cpp

${OBJECTDIR}/phppush.o: phppush.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/phppush.o phppush.cpp

${OBJECTDIR}/phpqueue.o: phpqueue.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/phpqueue.o phpqueue.cpp

${OBJECTDIR}/phpsocket.o: phpsocket.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/phpsocket.o phpsocket.cpp

${OBJECTDIR}/phpsocketpool.o: phpsocketpool.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/phpsocketpool.o phpsocketpool.cpp

${OBJECTDIR}/phptable.o: phptable.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/phptable.o phptable.cpp

${OBJECTDIR}/poolstartcontext.o: poolstartcontext.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/poolstartcontext.o poolstartcontext.cpp

${OBJECTDIR}/roothandler.o: roothandler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/roothandler.o roothandler.cpp

${OBJECTDIR}/spa_consts.o: spa_consts.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/spa_consts.o spa_consts.cpp

${OBJECTDIR}/spa_cs_consts.o: spa_cs_consts.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/spa_cs_consts.o spa_cs_consts.cpp

${OBJECTDIR}/stdafx.o: stdafx.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/stdafx.o stdafx.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libphpadapter.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
