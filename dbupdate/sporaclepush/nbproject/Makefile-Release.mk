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
	${OBJECTDIR}/_ext/1559065848/aclientw.o \
	${OBJECTDIR}/_ext/1559065848/error_code.o \
	${OBJECTDIR}/_ext/1559065848/membuffer.o \
	${OBJECTDIR}/_ext/1332925342/dbupdateimpl.o \
	${OBJECTDIR}/_ext/1332925342/udbubase.o \
	${OBJECTDIR}/sporaclepush.o


# C Compiler Flags
CFLAGS=-fPIC -static-libgcc -static-libstdc++ -std=c++11

# CC Compiler Flags
CCFLAGS=-fPIC -static-libgcc -static-libstdc++ -std=c++11
CXXFLAGS=-fPIC -static-libgcc -static-libstdc++ -std=c++11

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-ldl -lusocket

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libsporaclepush.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libsporaclepush.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libsporaclepush.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -s -fPIC

${OBJECTDIR}/_ext/1559065848/aclientw.o: ../../include/aclientw.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1559065848
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1559065848/aclientw.o ../../include/aclientw.cpp

${OBJECTDIR}/_ext/1559065848/error_code.o: ../../include/error_code.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1559065848
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1559065848/error_code.o ../../include/error_code.cpp

${OBJECTDIR}/_ext/1559065848/membuffer.o: ../../include/membuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1559065848
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -DNDEBUG -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1559065848/membuffer.o ../../include/membuffer.cpp

${OBJECTDIR}/_ext/1332925342/dbupdateimpl.o: ../udbubase/dbupdateimpl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1332925342
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1332925342/dbupdateimpl.o ../udbubase/dbupdateimpl.cpp

${OBJECTDIR}/_ext/1332925342/udbubase.o: ../udbubase/udbubase.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1332925342
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1332925342/udbubase.o ../udbubase/udbubase.cpp

${OBJECTDIR}/sporaclepush.o: sporaclepush.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DNDEBUG -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/sporaclepush.o sporaclepush.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libsporaclepush.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
