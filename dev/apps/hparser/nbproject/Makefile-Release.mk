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
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/jsloader.o \
	${OBJECTDIR}/asyncsession.o \
	${OBJECTDIR}/appsettings.o \
	${OBJECTDIR}/SHA1.o \
	${OBJECTDIR}/techunkedgrammar.o \
	${OBJECTDIR}/hparser.o \
	${OBJECTDIR}/httpgrammar.o \
	${OBJECTDIR}/_ext/1559065848/membuffer.o \
	${OBJECTDIR}/base64.o \
	${OBJECTDIR}/stdafx.o \
	${OBJECTDIR}/chat.o \
	${OBJECTDIR}/asyncserver.o \
	${OBJECTDIR}/multipartgrammar.o \
	${OBJECTDIR}/httpcontext.o


# C Compiler Flags
CFLAGS=-std=gnu++0x

# CC Compiler Flags
CCFLAGS=-std=c++0x
CXXFLAGS=-std=c++0x

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lpthread -lboost_system -lssl

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/hparser

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/hparser: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/hparser ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/jsloader.o: jsloader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/jsloader.o jsloader.cpp

${OBJECTDIR}/asyncsession.o: asyncsession.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/asyncsession.o asyncsession.cpp

${OBJECTDIR}/appsettings.o: appsettings.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/appsettings.o appsettings.cpp

${OBJECTDIR}/SHA1.o: SHA1.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/SHA1.o SHA1.cpp

${OBJECTDIR}/techunkedgrammar.o: techunkedgrammar.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/techunkedgrammar.o techunkedgrammar.cpp

${OBJECTDIR}/hparser.o: hparser.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/hparser.o hparser.cpp

${OBJECTDIR}/httpgrammar.o: httpgrammar.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/httpgrammar.o httpgrammar.cpp

${OBJECTDIR}/_ext/1559065848/membuffer.o: ../../include/membuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1559065848
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1559065848/membuffer.o ../../include/membuffer.cpp

${OBJECTDIR}/base64.o: base64.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/base64.o base64.cpp

${OBJECTDIR}/stdafx.o: stdafx.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/stdafx.o stdafx.cpp

${OBJECTDIR}/chat.o: chat.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/chat.o chat.cpp

${OBJECTDIR}/asyncserver.o: asyncserver.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/asyncserver.o asyncserver.cpp

${OBJECTDIR}/multipartgrammar.o: multipartgrammar.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/multipartgrammar.o multipartgrammar.cpp

${OBJECTDIR}/httpcontext.o: httpcontext.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/httpcontext.o httpcontext.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/hparser

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
