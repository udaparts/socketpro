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
	${OBJECTDIR}/_ext/932346631/aclientw.o \
	${OBJECTDIR}/_ext/932346631/membuffer.o \
	${OBJECTDIR}/_ext/932346631/tablecache.o \
	${OBJECTDIR}/aqueue.o \
	${OBJECTDIR}/dllmain.o \
	${OBJECTDIR}/njadapter.o \
	${OBJECTDIR}/njasyncqueue.o \
	${OBJECTDIR}/njcache.o \
	${OBJECTDIR}/njcert.o \
	${OBJECTDIR}/njclientqueue.o \
	${OBJECTDIR}/njfile.o \
	${OBJECTDIR}/njhandler.o \
	${OBJECTDIR}/njhandlerroot.o \
	${OBJECTDIR}/njobjects.o \
	${OBJECTDIR}/njpush.o \
	${OBJECTDIR}/njqueue.o \
	${OBJECTDIR}/njsocket.o \
	${OBJECTDIR}/njsqlite.o \
	${OBJECTDIR}/njtable.o \
	${OBJECTDIR}/sfile.o \
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
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/njadapter.node

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/njadapter.node: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/njadapter.node ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -s -fPIC

${OBJECTDIR}/_ext/932346631/aclientw.o: ../../../include/aclientw.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/932346631
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/932346631/aclientw.o ../../../include/aclientw.cpp

${OBJECTDIR}/_ext/932346631/membuffer.o: ../../../include/membuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/932346631
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/932346631/membuffer.o ../../../include/membuffer.cpp

${OBJECTDIR}/_ext/932346631/tablecache.o: ../../../include/tablecache.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/932346631
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/932346631/tablecache.o ../../../include/tablecache.cpp

${OBJECTDIR}/aqueue.o: aqueue.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/aqueue.o aqueue.cpp

${OBJECTDIR}/dllmain.o: dllmain.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/dllmain.o dllmain.cpp

${OBJECTDIR}/njadapter.o: njadapter.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/njadapter.o njadapter.cpp

${OBJECTDIR}/njasyncqueue.o: njasyncqueue.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/njasyncqueue.o njasyncqueue.cpp

${OBJECTDIR}/njcache.o: njcache.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/njcache.o njcache.cpp

${OBJECTDIR}/njcert.o: njcert.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/njcert.o njcert.cpp

${OBJECTDIR}/njclientqueue.o: njclientqueue.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/njclientqueue.o njclientqueue.cpp

${OBJECTDIR}/njfile.o: njfile.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/njfile.o njfile.cpp

${OBJECTDIR}/njhandler.o: njhandler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/njhandler.o njhandler.cpp

${OBJECTDIR}/njhandlerroot.o: njhandlerroot.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/njhandlerroot.o njhandlerroot.cpp

${OBJECTDIR}/njobjects.o: njobjects.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/njobjects.o njobjects.cpp

${OBJECTDIR}/njpush.o: njpush.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/njpush.o njpush.cpp

${OBJECTDIR}/njqueue.o: njqueue.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/njqueue.o njqueue.cpp

${OBJECTDIR}/njsocket.o: njsocket.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/njsocket.o njsocket.cpp

${OBJECTDIR}/njsqlite.o: njsqlite.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/njsqlite.o njsqlite.cpp

${OBJECTDIR}/njtable.o: njtable.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/njtable.o njtable.cpp

${OBJECTDIR}/sfile.o: sfile.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/sfile.o sfile.cpp

${OBJECTDIR}/stdafx.o: stdafx.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -DNODE_JS_ADAPTER_PROJECT -Inode -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/stdafx.o stdafx.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/njadapter.node

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
