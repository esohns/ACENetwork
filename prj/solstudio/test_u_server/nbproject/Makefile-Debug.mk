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
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/1385395607/net_eventhandler.o \
	${OBJECTDIR}/_ext/1385395607/net_module_eventhandler.o \
	${OBJECTDIR}/_ext/1385395607/net_server.o \
	${OBJECTDIR}/_ext/1385395607/net_server_signalhandler.o \
	${OBJECTDIR}/_ext/1385395607/stdafx.o


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
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/test_u_server

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/test_u_server: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/test_u_server ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/1385395607/net_eventhandler.o: ../../../test_u/net_eventhandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395607
	$(COMPILE.cc) -g -o ${OBJECTDIR}/_ext/1385395607/net_eventhandler.o ../../../test_u/net_eventhandler.cpp

${OBJECTDIR}/_ext/1385395607/net_module_eventhandler.o: ../../../test_u/net_module_eventhandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395607
	$(COMPILE.cc) -g -o ${OBJECTDIR}/_ext/1385395607/net_module_eventhandler.o ../../../test_u/net_module_eventhandler.cpp

${OBJECTDIR}/_ext/1385395607/net_server.o: ../../../test_u/net_server.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395607
	$(COMPILE.cc) -g -o ${OBJECTDIR}/_ext/1385395607/net_server.o ../../../test_u/net_server.cpp

${OBJECTDIR}/_ext/1385395607/net_server_signalhandler.o: ../../../test_u/net_server_signalhandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395607
	$(COMPILE.cc) -g -o ${OBJECTDIR}/_ext/1385395607/net_server_signalhandler.o ../../../test_u/net_server_signalhandler.cpp

${OBJECTDIR}/_ext/1385395607/stdafx.o: ../../../test_u/stdafx.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395607
	$(COMPILE.cc) -g -o ${OBJECTDIR}/_ext/1385395607/stdafx.o ../../../test_u/stdafx.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/test_u_server
	${CCADMIN} -clean

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
