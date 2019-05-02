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
	${OBJECTDIR}/_ext/262558690/operations.o \
	${OBJECTDIR}/_ext/262558690/path.o \
	${OBJECTDIR}/_ext/1441729126/error_code.o \
	${OBJECTDIR}/_ext/323364652/once.o \
	${OBJECTDIR}/_ext/323364652/thread.o \
	${OBJECTDIR}/_ext/386121535/tss_null.o \
	${OBJECTDIR}/_ext/932346631/membuffer.o \
	${OBJECTDIR}/_ext/32611005/base64.o \
	${OBJECTDIR}/_ext/1472/adler32.o \
	${OBJECTDIR}/_ext/1472/aes.o \
	${OBJECTDIR}/_ext/1472/blowfish.o \
	${OBJECTDIR}/_ext/1472/compress.o \
	${OBJECTDIR}/_ext/1472/crc32.o \
	${OBJECTDIR}/_ext/1472/deflate.o \
	${OBJECTDIR}/_ext/1472/fastujson.o \
	${OBJECTDIR}/_ext/1472/getsysid.o \
	${OBJECTDIR}/_ext/1472/infback.o \
	${OBJECTDIR}/_ext/1472/inffast.o \
	${OBJECTDIR}/_ext/1472/inflate.o \
	${OBJECTDIR}/_ext/1472/inftrees.o \
	${OBJECTDIR}/_ext/1472/lz4.o \
	${OBJECTDIR}/_ext/1472/mqfile.o \
	${OBJECTDIR}/_ext/1472/sha1.o \
	${OBJECTDIR}/_ext/1472/trees.o \
	${OBJECTDIR}/_ext/1472/uncompr.o \
	${OBJECTDIR}/_ext/1472/uzip.o \
	${OBJECTDIR}/_ext/1472/zutil.o


# C Compiler Flags
CFLAGS=-fPIC -static-libgcc -static-libstdc++ -static -std=c++11

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

${OBJECTDIR}/_ext/262558690/operations.o: ../../../../../Downloads/boost/libs/filesystem/src/operations.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/262558690
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/262558690/operations.o ../../../../../Downloads/boost/libs/filesystem/src/operations.cpp

${OBJECTDIR}/_ext/262558690/path.o: ../../../../../Downloads/boost/libs/filesystem/src/path.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/262558690
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/262558690/path.o ../../../../../Downloads/boost/libs/filesystem/src/path.cpp

${OBJECTDIR}/_ext/1441729126/error_code.o: ../../../../../Downloads/boost/libs/system/src/error_code.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1441729126
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1441729126/error_code.o ../../../../../Downloads/boost/libs/system/src/error_code.cpp

${OBJECTDIR}/_ext/323364652/once.o: ../../../../../Downloads/boost/libs/thread/src/pthread/once.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/323364652
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/323364652/once.o ../../../../../Downloads/boost/libs/thread/src/pthread/once.cpp

${OBJECTDIR}/_ext/323364652/thread.o: ../../../../../Downloads/boost/libs/thread/src/pthread/thread.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/323364652
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/323364652/thread.o ../../../../../Downloads/boost/libs/thread/src/pthread/thread.cpp

${OBJECTDIR}/_ext/386121535/tss_null.o: ../../../../../Downloads/boost/libs/thread/src/tss_null.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/386121535
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/386121535/tss_null.o ../../../../../Downloads/boost/libs/thread/src/tss_null.cpp

${OBJECTDIR}/_ext/932346631/membuffer.o: ../../../include/membuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/932346631
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/932346631/membuffer.o ../../../include/membuffer.cpp

${OBJECTDIR}/_ext/32611005/base64.o: ../../../pinc/base64.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/32611005
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/32611005/base64.o ../../../pinc/base64.cpp

${OBJECTDIR}/_ext/1472/adler32.o: ../adler32.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/adler32.o ../adler32.c

${OBJECTDIR}/_ext/1472/aes.o: ../aes.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/aes.o ../aes.c

${OBJECTDIR}/_ext/1472/blowfish.o: ../blowfish.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/blowfish.o ../blowfish.cpp

${OBJECTDIR}/_ext/1472/compress.o: ../compress.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/compress.o ../compress.c

${OBJECTDIR}/_ext/1472/crc32.o: ../crc32.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/crc32.o ../crc32.c

${OBJECTDIR}/_ext/1472/deflate.o: ../deflate.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/deflate.o ../deflate.c

${OBJECTDIR}/_ext/1472/fastujson.o: ../fastujson.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/fastujson.o ../fastujson.cpp

${OBJECTDIR}/_ext/1472/getsysid.o: ../getsysid.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/getsysid.o ../getsysid.cpp

${OBJECTDIR}/_ext/1472/infback.o: ../infback.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/infback.o ../infback.c

${OBJECTDIR}/_ext/1472/inffast.o: ../inffast.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/inffast.o ../inffast.c

${OBJECTDIR}/_ext/1472/inflate.o: ../inflate.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/inflate.o ../inflate.c

${OBJECTDIR}/_ext/1472/inftrees.o: ../inftrees.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/inftrees.o ../inftrees.c

${OBJECTDIR}/_ext/1472/lz4.o: ../lz4.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/lz4.o ../lz4.c

${OBJECTDIR}/_ext/1472/mqfile.o: ../mqfile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/mqfile.o ../mqfile.cpp

${OBJECTDIR}/_ext/1472/sha1.o: ../sha1.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/sha1.o ../sha1.cpp

${OBJECTDIR}/_ext/1472/trees.o: ../trees.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/trees.o ../trees.c

${OBJECTDIR}/_ext/1472/uncompr.o: ../uncompr.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/uncompr.o ../uncompr.c

${OBJECTDIR}/_ext/1472/uzip.o: ../uzip.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/uzip.o ../uzip.cpp

${OBJECTDIR}/_ext/1472/zutil.o: ../zutil.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1472
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1472/zutil.o ../zutil.c

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
