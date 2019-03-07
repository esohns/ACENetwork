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
CC=cc
CCC=CC
CXX=CC
FC=f95
AS=as

# Macros
CND_PLATFORM=OracleSolarisStudio-Solaris-x86
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
	${OBJECTDIR}/_ext/576663459/net_server_asynchlistener.o \
	${OBJECTDIR}/_ext/576663459/net_server_common_tools.o \
	${OBJECTDIR}/_ext/576663459/net_server_listener.o \
	${OBJECTDIR}/_ext/576663459/stdafx.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblibACENetwork_Server.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblibACENetwork_Server.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblibACENetwork_Server.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -G -KPIC -norunpath -h liblibACENetwork_Server.${CND_DLIB_EXT}

${OBJECTDIR}/_ext/576663459/net_server_asynchlistener.o: ../../../src/client_server/net_server_asynchlistener.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/576663459
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/576663459/net_server_asynchlistener.o ../../../src/client_server/net_server_asynchlistener.cpp

${OBJECTDIR}/_ext/576663459/net_server_common_tools.o: ../../../src/client_server/net_server_common_tools.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/576663459
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/576663459/net_server_common_tools.o ../../../src/client_server/net_server_common_tools.cpp

${OBJECTDIR}/_ext/576663459/net_server_listener.o: ../../../src/client_server/net_server_listener.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/576663459
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/576663459/net_server_listener.o ../../../src/client_server/net_server_listener.cpp

${OBJECTDIR}/_ext/576663459/stdafx.o: ../../../src/client_server/stdafx.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/576663459
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/576663459/stdafx.o ../../../src/client_server/stdafx.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblibACENetwork_Server.${CND_DLIB_EXT}
	${CCADMIN} -clean

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
