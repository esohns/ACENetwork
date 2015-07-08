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
	${OBJECTDIR}/_ext/1385395595/FILE_Stream.o \
	${OBJECTDIR}/_ext/1385395595/IOStream_alt_T.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_IRCbisect.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_IRCmessage.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_IRCparser.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_IRCparser_driver.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_IRCscanner.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_gui.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_gui_callbacks.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_gui_connection.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_gui_messagehandler.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_gui_tools.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_inputhandler.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_message.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_messageallocator.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_module_IRChandler.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_module_IRCparser.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_module_IRCsplitter.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_module_IRCstreamer.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_sessionmessage.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_signalhandler.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_statemachine_registration.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_stream.o \
	${OBJECTDIR}/_ext/1385395595/IRC_client_tools.o \
	${OBJECTDIR}/_ext/1385395595/Streambuf_alt.o \
	${OBJECTDIR}/_ext/1385395595/stdafx.o


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
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/test_i_irc_client_gui

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/test_i_irc_client_gui: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/test_i_irc_client_gui ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/1385395595/FILE_Stream.o: ../../../test_i/FILE_Stream.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/FILE_Stream.o ../../../test_i/FILE_Stream.cpp

${OBJECTDIR}/_ext/1385395595/IOStream_alt_T.o: ../../../test_i/IOStream_alt_T.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IOStream_alt_T.o ../../../test_i/IOStream_alt_T.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_IRCbisect.o: ../../../test_i/IRC_client_IRCbisect.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_IRCbisect.o ../../../test_i/IRC_client_IRCbisect.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_IRCmessage.o: ../../../test_i/IRC_client_IRCmessage.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_IRCmessage.o ../../../test_i/IRC_client_IRCmessage.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_IRCparser.o: ../../../test_i/IRC_client_IRCparser.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_IRCparser.o ../../../test_i/IRC_client_IRCparser.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_IRCparser_driver.o: ../../../test_i/IRC_client_IRCparser_driver.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_IRCparser_driver.o ../../../test_i/IRC_client_IRCparser_driver.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_IRCscanner.o: ../../../test_i/IRC_client_IRCscanner.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_IRCscanner.o ../../../test_i/IRC_client_IRCscanner.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_gui.o: ../../../test_i/IRC_client_gui.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_gui.o ../../../test_i/IRC_client_gui.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_gui_callbacks.o: ../../../test_i/IRC_client_gui_callbacks.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_gui_callbacks.o ../../../test_i/IRC_client_gui_callbacks.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_gui_connection.o: ../../../test_i/IRC_client_gui_connection.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_gui_connection.o ../../../test_i/IRC_client_gui_connection.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_gui_messagehandler.o: ../../../test_i/IRC_client_gui_messagehandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_gui_messagehandler.o ../../../test_i/IRC_client_gui_messagehandler.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_gui_tools.o: ../../../test_i/IRC_client_gui_tools.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_gui_tools.o ../../../test_i/IRC_client_gui_tools.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_inputhandler.o: ../../../test_i/IRC_client_inputhandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_inputhandler.o ../../../test_i/IRC_client_inputhandler.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_message.o: ../../../test_i/IRC_client_message.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_message.o ../../../test_i/IRC_client_message.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_messageallocator.o: ../../../test_i/IRC_client_messageallocator.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_messageallocator.o ../../../test_i/IRC_client_messageallocator.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_module_IRChandler.o: ../../../test_i/IRC_client_module_IRChandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_module_IRChandler.o ../../../test_i/IRC_client_module_IRChandler.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_module_IRCparser.o: ../../../test_i/IRC_client_module_IRCparser.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_module_IRCparser.o ../../../test_i/IRC_client_module_IRCparser.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_module_IRCsplitter.o: ../../../test_i/IRC_client_module_IRCsplitter.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_module_IRCsplitter.o ../../../test_i/IRC_client_module_IRCsplitter.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_module_IRCstreamer.o: ../../../test_i/IRC_client_module_IRCstreamer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_module_IRCstreamer.o ../../../test_i/IRC_client_module_IRCstreamer.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_sessionmessage.o: ../../../test_i/IRC_client_sessionmessage.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_sessionmessage.o ../../../test_i/IRC_client_sessionmessage.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_signalhandler.o: ../../../test_i/IRC_client_signalhandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_signalhandler.o ../../../test_i/IRC_client_signalhandler.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_statemachine_registration.o: ../../../test_i/IRC_client_statemachine_registration.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_statemachine_registration.o ../../../test_i/IRC_client_statemachine_registration.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_stream.o: ../../../test_i/IRC_client_stream.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_stream.o ../../../test_i/IRC_client_stream.cpp

${OBJECTDIR}/_ext/1385395595/IRC_client_tools.o: ../../../test_i/IRC_client_tools.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/IRC_client_tools.o ../../../test_i/IRC_client_tools.cpp

${OBJECTDIR}/_ext/1385395595/Streambuf_alt.o: ../../../test_i/Streambuf_alt.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/Streambuf_alt.o ../../../test_i/Streambuf_alt.cpp

${OBJECTDIR}/_ext/1385395595/stdafx.o: ../../../test_i/stdafx.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1385395595
	$(COMPILE.cc) -fast -g0 -o ${OBJECTDIR}/_ext/1385395595/stdafx.o ../../../test_i/stdafx.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/test_i_irc_client_gui
	${CCADMIN} -clean

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
