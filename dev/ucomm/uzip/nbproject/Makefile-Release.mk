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
	${OBJECTDIR}/_ext/473765425/operations.o \
	${OBJECTDIR}/_ext/473765425/path.o \
	${OBJECTDIR}/_ext/1881754677/error_code.o \
	${OBJECTDIR}/_ext/1761269253/once.o \
	${OBJECTDIR}/_ext/1761269253/thread.o \
	${OBJECTDIR}/_ext/53904016/tss_null.o \
	${OBJECTDIR}/_ext/302208857/base64.o \
	${OBJECTDIR}/_ext/262068057/membuffer.o \
	${OBJECTDIR}/adler32.o \
	${OBJECTDIR}/aes.o \
	${OBJECTDIR}/blowfish.o \
	${OBJECTDIR}/compress.o \
	${OBJECTDIR}/crc32.o \
	${OBJECTDIR}/deflate.o \
	${OBJECTDIR}/fastujson.o \
	${OBJECTDIR}/getsysid.o \
	${OBJECTDIR}/infback.o \
	${OBJECTDIR}/inffast.o \
	${OBJECTDIR}/inflate.o \
	${OBJECTDIR}/inftrees.o \
	${OBJECTDIR}/lz4.o \
	${OBJECTDIR}/mqfile.o \
	${OBJECTDIR}/sha1.o \
	${OBJECTDIR}/trees.o \
	${OBJECTDIR}/uncompr.o \
	${OBJECTDIR}/uzip.o \
	${OBJECTDIR}/zutil.o


# C Compiler Flags
CFLAGS=-fPIC -std=c++11 -static-libgcc -static-libstdc++ -static

# CC Compiler Flags
CCFLAGS=-fPIC -std=c++11 -static-libgcc -static-libstdc++ -static
CXXFLAGS=-fPIC -std=c++11 -static-libgcc -static-libstdc++ -static

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libuzip.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libuzip.a: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libuzip.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libuzip.a ${OBJECTFILES} 
	$(RANLIB) ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libuzip.a

${OBJECTDIR}/_ext/473765425/operations.o: ../../../../Downloads/boost/libs/filesystem/src/operations.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/473765425
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/473765425/operations.o ../../../../Downloads/boost/libs/filesystem/src/operations.cpp

${OBJECTDIR}/_ext/473765425/path.o: ../../../../Downloads/boost/libs/filesystem/src/path.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/473765425
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/473765425/path.o ../../../../Downloads/boost/libs/filesystem/src/path.cpp

${OBJECTDIR}/_ext/1881754677/error_code.o: ../../../../Downloads/boost/libs/system/src/error_code.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1881754677
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1881754677/error_code.o ../../../../Downloads/boost/libs/system/src/error_code.cpp

${OBJECTDIR}/_ext/1761269253/once.o: ../../../../Downloads/boost/libs/thread/src/pthread/once.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1761269253
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1761269253/once.o ../../../../Downloads/boost/libs/thread/src/pthread/once.cpp

${OBJECTDIR}/_ext/1761269253/thread.o: ../../../../Downloads/boost/libs/thread/src/pthread/thread.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1761269253
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1761269253/thread.o ../../../../Downloads/boost/libs/thread/src/pthread/thread.cpp

${OBJECTDIR}/_ext/53904016/tss_null.o: ../../../../Downloads/boost/libs/thread/src/tss_null.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/53904016
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/53904016/tss_null.o ../../../../Downloads/boost/libs/thread/src/tss_null.cpp

${OBJECTDIR}/_ext/302208857/base64.o: ../core_shared/pinc/base64.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/302208857
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/302208857/base64.o ../core_shared/pinc/base64.cpp

${OBJECTDIR}/_ext/262068057/membuffer.o: ../include/membuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/262068057
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/262068057/membuffer.o ../include/membuffer.cpp

${OBJECTDIR}/adler32.o: adler32.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/adler32.o adler32.c

${OBJECTDIR}/aes.o: aes.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/aes.o aes.c

${OBJECTDIR}/blowfish.o: blowfish.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/blowfish.o blowfish.cpp

${OBJECTDIR}/compress.o: compress.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/compress.o compress.c

${OBJECTDIR}/crc32.o: crc32.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/crc32.o crc32.c

${OBJECTDIR}/deflate.o: deflate.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/deflate.o deflate.c

${OBJECTDIR}/fastujson.o: fastujson.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/fastujson.o fastujson.cpp

${OBJECTDIR}/getsysid.o: getsysid.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/getsysid.o getsysid.cpp

${OBJECTDIR}/infback.o: infback.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/infback.o infback.c

${OBJECTDIR}/inffast.o: inffast.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/inffast.o inffast.c

${OBJECTDIR}/inflate.o: inflate.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/inflate.o inflate.c

${OBJECTDIR}/inftrees.o: inftrees.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/inftrees.o inftrees.c

${OBJECTDIR}/lz4.o: lz4.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lz4.o lz4.c

${OBJECTDIR}/mqfile.o: mqfile.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/mqfile.o mqfile.cpp

${OBJECTDIR}/sha1.o: sha1.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/sha1.o sha1.cpp

${OBJECTDIR}/trees.o: trees.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/trees.o trees.c

${OBJECTDIR}/uncompr.o: uncompr.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/uncompr.o uncompr.c

${OBJECTDIR}/uzip.o: uzip.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/uzip.o uzip.cpp

${OBJECTDIR}/zutil.o: zutil.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -DNDEBUG -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/zutil.o zutil.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libuzip.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
